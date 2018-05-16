#ifndef PP_LAYERMORPH_HPP_INCLUDED
#define PP_LAYERMORPH_HPP_INCLUDED

/**
 @file      pp_layermorph.hpp
 @copyright François Becker
 @date      2017-2018
 */

#include "pp_layer.hpp"
#include "pp_utils.hpp"

namespace PP
{

//==============================================================================
class LayerMorph
: public Layer
{
public:
    LayerMorph(float pThreshold)
    : Layer()
    , mThreshold(pThreshold)
    {
    }
    
    ~LayerMorph()
    {
    }
    
    float getThreshold() const
    {
        return mThreshold;
    }
    
    void setThreshold(float pThreshold)
    {
        mThreshold = pThreshold;
        // TODO: trigger updates
    }
    
    /**
     */
    void blendPreview(const QImage& pSrc, QImage& pBlendedImage, const Tool& pTool, float pWidthMM) const override
    {
        assert(pBlendedImage.size() == pSrc.size());
        
#if 1
        QColor lToolColour = pTool.getColour();
        float lToolColourRedFactor = lToolColour.redF();
        float lToolColourGreenFactor = lToolColour.greenF();
        float lToolColourBlueFactor = lToolColour.blueF();
        
        //QImage lThresholded = pSrc.copy();
        for (int y = 0 ; y != pSrc.height() ; ++y)
        {
            for (int x = 0 ; x != pSrc.width() ; ++x)
            {
                bool lComputed = (pSrc.pixelColor(x, y).lightnessF() < getThreshold());
                if (lComputed)
                {
                    QColor lColour = pBlendedImage.pixelColor(x, y);
                    
                    QColor lNewColour = QColor(lColour.red() * lToolColourRedFactor,
                                               lColour.green() * lToolColourGreenFactor,
                                               lColour.blue() * lToolColourBlueFactor);
                    pBlendedImage.setPixelColor(x, y, lNewColour);
                }
            }
        }
#else
        pBlendedImage = essentialize(pSrc, pWidthMM, pTool).toImage();
#endif
    }
    
    /**
     Extract the path trace of the pencil as a binary image.
     */
    BinaryImage essentialize(QImage pImage, float pWidthMM, const Tool& pTool) const
    {
        // width of the tool in pixels
        const int cStepPixels = std::max(1, (int)std::floor(pTool.getWidthMM() * pImage.width() / pWidthMM));
        
        // Median
#if 0
        QImage lMedian = MorphOps::median(pImage);
#else
        QImage lMedian = pImage;
#endif
        
        // Threshold
        BinaryImage lBinaryImage(lMedian, getThreshold()); // TODO: static BinaryImage::thresholded(...)
        
        // Matrices of the paths
        lBinaryImage.invert();
        BinaryImage lBorders(lBinaryImage.getWidth(), lBinaryImage.getHeight());
        
#if 1
        for (int u = 0 ; u < cStepPixels / 2 ; ++u)
        {
            MorphOps::thin(lBinaryImage);
        }
        lBorders.add(MorphOps::removeBorder(lBinaryImage));
#if 0
        for (int u = 0 ; u < cStepPixels - 1 - (cStepPixels / 2) ; ++u)
#else
        for (int u = 0 ; u < std::max(cStepPixels / 4, 1) ; ++u)
#endif
        {
            MorphOps::erode(lBinaryImage);
        }
        
        //lBorders.add(lBinaryImage);
        static bool sDirection = true;
        sDirection = !sDirection;
        lBorders.add(MorphOps::diagonal(lBinaryImage, cStepPixels, sDirection));
#elif 1
        const int cSubStepPixels = std::max(2, (3 * cStepPixels) / 4);
        while (! lBinaryImage.isEmpty())
        {
            for (int u = 0 ; u < cSubStepPixels / 2 ; ++u)
            {
                MorphOps::thin(lBinaryImage);
            }
            lBorders.add(MorphOps::removeBorder(lBinaryImage));
            for (int u = 0 ; u < cSubStepPixels - 1 - (cSubStepPixels / 2) ; ++u)
            {
                MorphOps::erode(lBinaryImage);
            }
        }
#else
        while (! lBinaryImage.isEmpty())
        {
            for (int u = 0 ; u < cStepPixels / 2 ; ++u)
            {
                MorphOps::thin(lBinaryImage);
            }
            lBorders.add(MorphOps::removeBorder(lBinaryImage));
            for (int u = 0 ; u < cStepPixels - 1 - (cStepPixels / 2) ; ++u)
            {
                MorphOps::thin(lBinaryImage);
            }
        }
#endif
        
        MorphOps::thin(lBorders);
        
        MorphOps::median(lBorders);
        
        return lBorders;
    }
    
    /**
     */
    void compile(QImage pImage, float pZoneSizeMMX, float pZoneSizeMMY, float pWidthMM, const Tool& pTool, std::ofstream& pOut) const override
    {
        BinaryImage lBorders = essentialize(pImage, pWidthMM, pTool);
        
        // Build the paths from the matrices
        std::vector<PointPixel> lPointPixels;
        for (int y = 0 ; y != lBorders.getHeight() ; ++y)
        {
            for (int x = 0 ; x != lBorders.getWidth() ; ++x)
            {
                if (lBorders.getPixel(x, y))
                {
                    lPointPixels.push_back({x,y});
                }
            }
        }
        // Combine paths
        std::vector<CombinedPathsPixels> lCombinedPathPixels;
        std::vector<PointPixel>::iterator lPointIt = lPointPixels.begin();
        while (lPointIt != lPointPixels.end())
        {
            CombinedPathsPixels lCombinedPath({*lPointIt});
            // find a path that begins or ends with one end touching the begin or end of this one.
            auto lIt = lPointIt;
            ++lIt;
            while (lIt != lPointPixels.end())
            {
                if (PointPixel::areNeighbour8(lCombinedPath.mPoints.back(), *lIt))
                {
                    lCombinedPath.mPoints.push_back(*lIt);
                    lIt = lPointPixels.erase(lIt);
                }
                else if (PointPixel::areNeighbour8(lCombinedPath.mPoints.front(), *lIt))
                {
                    lCombinedPath.mPoints.push_front(*lIt);
                    lIt = lPointPixels.erase(lIt);
                }
                else
                {
                    ++lIt;
                }
            }
            lCombinedPathPixels.push_back(lCombinedPath);
            ++lPointIt;
        }
        
        // simplify paths by removing points in colinear moves
        for (auto& lPath : lCombinedPathPixels)
        {
            auto& lPoints = lPath.mPoints;
            for(auto lIt = lPoints.begin() ; lIt != lPoints.end() ;)
            {
                if (std::distance(lIt, lPoints.end()) >= 3)
                {
                    auto lNext1 = std::next(lIt);
                    auto lNext2 = std::next(lNext1);
                    if (cross(*lNext1 - *lIt, *lNext2 - *lNext1) == 0)
                    {
                        // colinear, suppress *lNext1
                        lPoints.erase(lNext1);
                    }
                    else
                    {
                        ++lIt;
                    }
                }
                else
                {
                    ++lIt;
                }
            }
        }
        
        // convert to physical coordinates
        std::vector<CombinedPathMM> lCombinedPathMM;
        float lMMperPixel = pWidthMM / pImage.width();
        const float cXOffset = pZoneSizeMMX / 2.f + pWidthMM / 2.f;
        const float cYOffset = pZoneSizeMMY / 2.f - pImage.height() * lMMperPixel / 2.f;
        for (CombinedPathsPixels lCPP : lCombinedPathPixels)
        {
            CombinedPathMM lCPMM;
            for (PointPixel lPP : lCPP.mPoints)
            {
                float lXMM = cXOffset - lPP.mX * lMMperPixel;
                float lYMM = cYOffset + lPP.mY * lMMperPixel;
                lCPMM.mPoints.push_back({lXMM, lYMM});
            }
            lCombinedPathMM.push_back(lCPMM);
        }
        
        // re-combine
        float lLimitDist = pTool.getWidthMM() * 2.f;
        for (auto lIt = lCombinedPathMM.begin() ; lIt != lCombinedPathMM.end() ; ++lIt)
        {
            for (auto lOtherIt = std::next(lIt) ; lOtherIt != lCombinedPathMM.end() ; )
            {
                if (pTool.getNeedsRefill() && lIt->length() > pTool.getLengthBeforeRefillMM())
                {
                    break;
                }
                else if (PointMM::length(lIt->mPoints.back(), lOtherIt->mPoints.front()) < lLimitDist)
                {
                    for (auto lPointMM : lOtherIt->mPoints)
                    {
                        lIt->mPoints.push_back(lPointMM);
                    }
                    lOtherIt = lCombinedPathMM.erase(lOtherIt);
                }
                else if (PointMM::length(lIt->mPoints.back(), lOtherIt->mPoints.back()) < lLimitDist)
                {
                    for (auto lPointMMIt = lOtherIt->mPoints.rbegin() ; lPointMMIt != lOtherIt->mPoints.rend() ; ++lPointMMIt)
                    {
                        lIt->mPoints.push_back(*lPointMMIt);
                    }
                    lOtherIt = lCombinedPathMM.erase(lOtherIt);
                }
                else if (PointMM::length(lIt->mPoints.front(), lOtherIt->mPoints.front()) < lLimitDist)
                {
                    for (auto lPointMM : lOtherIt->mPoints)
                    {
                        lIt->mPoints.push_front(lPointMM);
                    }
                    lOtherIt = lCombinedPathMM.erase(lOtherIt);
                }
                else if (PointMM::length(lIt->mPoints.front(), lOtherIt->mPoints.back()) < lLimitDist)
                {
                    for (auto lPointMMIt = lOtherIt->mPoints.rbegin() ; lPointMMIt != lOtherIt->mPoints.rend() ; ++lPointMMIt)
                    {
                        lIt->mPoints.push_front(*lPointMMIt);
                    }
                    lOtherIt = lCombinedPathMM.erase(lOtherIt);
                }
                else
                {
                    ++lOtherIt;
                }
            }
        }
        
#if 0
        // Convert to gcode
        float lLength = 0.f;
        pOut << pTool.getReloadCommand();
        for (auto& lPath : lCombinedPathMM)
        {
            lPath.fixDragError(pTool.getDragErrorMM());
            lPath.compile(pOut);
            lLength += lPath.length();
            lLength += 0.5f; // each one spills some ink
            if (pTool.getNeedsReload() && lLength > pTool.getLengthBeforeReloadMM())
            {
                pOut << pTool.getReloadCommand();
                lLength = 0.f;
            }
        }
#else
        // Convert to gcode while sorting pathes by length so that no ink drip occurs on short pathes
        float lLength = 0.f;
        pOut << pTool.getRefillCommand();
        std::vector<CombinedPathMM> lPathes;
        for (auto& lPath : lCombinedPathMM)
        {
            if (pTool.getNeedsRefill())
            {
                lLength += lPath.length() + 2.f; // as each one spills ink
                lPathes.push_back(lPath);
                if (lLength > pTool.getLengthBeforeRefillMM())
                {
                    std::sort(lPathes.begin(), lPathes.end(),
                              [](CombinedPathMM& p1, CombinedPathMM& p2) {
                                  return p1.length() > p2.length();
                              });
                    for (auto& p : lPathes)
                    {
                        p.fixDragError(pTool.getDragErrorMM());
                        p.compile(pOut);
                    }
                    pOut << pTool.getRefillCommand();
                    lLength = 0.f;
                    lPathes.clear();
                }
            }
            else
            {
                lPath.fixDragError(pTool.getDragErrorMM());
                lPath.compile(pOut);
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
