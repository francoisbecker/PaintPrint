#ifndef PP_LAYERDIAGONAL_HPP_INCLUDED
#define PP_LAYERDIAGONAL_HPP_INCLUDED

/**
 @file      pp_layerdiagonal.hpp
 @copyright François Becker
 @date      2017-2018
 */

#include "pp_layer.hpp"
#include "pp_utils.hpp"

namespace PP
{

class LayerDiagonal
: public Layer
{
public:
    LayerDiagonal(float pThreshold)
    : Layer()
    , mThreshold(pThreshold)
    {
    }
    
    ~LayerDiagonal()
    {
    }
    
    float getThreshold() const
    {
        return mThreshold;
    }
    
    void setThreshold(float pThreshold)
    {
        mThreshold = pThreshold;
    }
    
    void blendPreview(const QImage& pSrc, QImage& pBlendedImage, const Tool& pTool, float pWidthMM) const override
    {
        jassert(pBlendedImage.getBounds() == pSrc.getBounds());
        
        //QImage lThresholded = pSrc.createCopy();
        for (int x = 0 ; x != pSrc.getWidth() ; ++x)
        {
            for (int y = 0 ; y != pSrc.getHeight() ; ++y)
            {
                bool lComputed = (pSrc.getPixelAt(x, y).getBrightness() < getThreshold());
                if (lComputed)
                {
                    pBlendedImage.setPixelAt(x, y, pBlendedImage.getPixelAt(x, y).darker(0.1f));
                }
            }
        }
    }
    
    void compile(QImage pImage, float pZoneSizeMMX, float pZoneSizeMMY, float pWidthMM, const Tool& pTool, std::ofstream& pOut) const override
    {
        // width of the tool in pixels
        const int cStepPixels = std::max(1, (int)std::floor(pTool.getWidthMM() * pImage.getWidth() / pWidthMM));
        
        Image lThresholded = pImage.createCopy();
        for (int x = 0 ; x != pImage.getWidth() ; ++x)
        {
            for (int y = 0 ; y != pImage.getHeight() ; ++y)
            {
                bool lComputed = (pImage.getPixelAt(x, y).getBrightness() < getThreshold());
                lThresholded.setPixelAt(x, y, lComputed ? Colours::black : Colours::white);
            }
        }
        
        //Image lMorphed = lThresholded.createCopy(); // TODO later
        
        // Diagonal paths
        std::vector<LinePathPixel> lLinesPixels;
        for (int c = 0 ; c < lThresholded.getWidth() ; c += cStepPixels)
        {
            int x = c;
            int y = 0;
            bool lWasDown = false;
            while (x < lThresholded.getWidth() && y < lThresholded.getHeight())
            {
                bool lIsDown = (lThresholded.getPixelAt(x, y) == Colours::black);
                if (lIsDown)
                {
                    if (lWasDown)
                    {
                        lLinesPixels.back().mToX = x;
                        lLinesPixels.back().mToY = y;
                    }
                    else
                    {
                        lLinesPixels.push_back({x,y,x,y});
                    }
                }
                lWasDown = lIsDown;
                ++x;
                ++y;
            }
        }
        std::reverse(lLinesPixels.begin(), lLinesPixels.end());
        for (int l = cStepPixels ; l < lThresholded.getHeight() ; l += cStepPixels)
        {
            int x = 0;
            int y = l;
            bool lWasDown = false;
            while (x < lThresholded.getWidth() && y < lThresholded.getHeight())
            {
                bool lIsDown = (lThresholded.getPixelAt(x, y) == Colours::black);
                if (lIsDown)
                {
                    if (lWasDown)
                    {
                        lLinesPixels.back().mToX = x;
                        lLinesPixels.back().mToY = y;
                    }
                    else
                    {
                        lLinesPixels.push_back({x,y,x,y});
                    }
                }
                lWasDown = lIsDown;
                ++x;
                ++y;
            }
        }
        
        // convert lines to physical coordinates
        struct SegmentMMCompiler
        {
            static void compile (std::ofstream& pOut, const SegmentMM& pLinePathMM)
            {
                //pOut << "G0 Z3" << std::endl;
                pOut << "G0 X" << pLinePathMM.mFrom.mX << " Y" << pLinePathMM.mFrom.mY << " Z2.5" << std::endl;
                pOut << "G0 Z0" << std::endl;
                pOut << "G1 X" << pLinePathMM.mTo.mX << " Y" << pLinePathMM.mTo.mY << std::endl;
                pOut << "G0 Z2.5" << std::endl;
            }
        };
        std::vector<SegmentMM> lSegmentsMM;
        float lMMperPixel = pWidthMM / pImage.getWidth();
        for (auto lLine : lLinesPixels)
        {
            float lXOffset = pZoneSizeMMX / 2.f - pImage.getHeight() * lMMperPixel / 2.f;
            float lYOffset = pZoneSizeMMY / 2.f - pWidthMM / 2.f;
            float lFromXMM = lXOffset + lLine.mFromY * lMMperPixel;
            float lFromYMM = lYOffset + lLine.mFromX * lMMperPixel;
            float lToXMM = lXOffset + lLine.mToY * lMMperPixel;
            float lToYMM = lYOffset + lLine.mToX * lMMperPixel;
            lSegmentsMM.push_back({lFromXMM, lFromYMM, lToXMM, lToYMM});
        }
        
#if 1
        // combine paths
        std::vector<CombinedPathMM> lCombinedPathMM;
        auto lSegmentIt = std::begin(lSegmentsMM);
        while (lSegmentIt != std::end(lSegmentsMM))
        {
            //const float cSpacingTolerance = 1.45f * pTool.getWidthMM();
            const float cSpacingTolerance = 2.15f * pTool.getWidthMM();
            float lLength = lSegmentIt->length();
            CombinedPathMM lCombinedPath({lSegmentIt->mFrom, lSegmentIt->mTo});
            // find a path that begins or ends with one end close to the begin or end of this one.
            auto lIt = lSegmentIt;
            ++lIt;
            while (lIt != std::end(lSegmentsMM) && lLength < pTool.getLengthBeforeRefillMM())
            {
                if (PointMM::length(lCombinedPath.mPoints.back(), lIt->mFrom) < cSpacingTolerance)
                {
                    lCombinedPath.mPoints.push_back(lIt->mFrom);
                    lCombinedPath.mPoints.push_back(lIt->mTo);
                    lLength += lIt->length() + cSpacingTolerance;
                    lIt = lSegmentsMM.erase(lIt);
                }
                else if (PointMM::length(lCombinedPath.mPoints.back(), lIt->mTo) < cSpacingTolerance)
                {
                    lCombinedPath.mPoints.push_back(lIt->mTo);
                    lCombinedPath.mPoints.push_back(lIt->mFrom);
                    lLength += lIt->length() + cSpacingTolerance;
                    lIt = lSegmentsMM.erase(lIt);
                }
                else if (PointMM::length(lCombinedPath.mPoints.front(), lIt->mFrom) < cSpacingTolerance)
                {
                    lCombinedPath.mPoints.push_front(lIt->mFrom);
                    lCombinedPath.mPoints.push_front(lIt->mTo);
                    lLength += lIt->length() + cSpacingTolerance;
                    lIt = lSegmentsMM.erase(lIt);
                }
                else if (PointMM::length(lCombinedPath.mPoints.front(), lIt->mTo) < cSpacingTolerance)
                {
                    lCombinedPath.mPoints.push_front(lIt->mTo);
                    lCombinedPath.mPoints.push_front(lIt->mFrom);
                    lLength += lIt->length() + cSpacingTolerance;
                    lIt = lSegmentsMM.erase(lIt);
                }
                else
                {
                    ++lIt;
                }
            }
            lCombinedPathMM.push_back(lCombinedPath);
            ++lSegmentIt;
        }
        
        // Convert to gcode
        float lLength = 0.f;
        pOut << pTool.getReloadCommand();
        for (auto lPath : lCombinedPathMM)
        {
            lPath.compile(pOut);
            lLength += lPath.length();
            lLength += 10.f; // each one spills some ink
            if (lLength > pTool.getLengthBeforeRefillMM())
            {
                pOut << pTool.getReloadCommand();
                lLength = 0.f;
            }
        }
#else
        // Convert to gcode
        float lLength = 0.f;
        pOut << pReloadCommand;
        for (auto lSegment : lSegmentsMM)
        {
            SegmentMMCompiler::compile(pOut, lSegment);
            lLength += lSegment.length();
            lLength += 10.f; // each one spills some ink
            if (lLength > pTool.getLengthBeforeRefillMM())
            {
                pOut << pReloadCommand;
                lLength = 0.f;
            }
        }
#endif
        
        // Wait to dry
        pOut << "G4 P" << pTool.getDryTimeSeconds() << "000" << std::endl;
    }
    
private:
    float mThreshold;
};

}

#endif
