#ifndef PP_PROJECT_HPP_INCLUDED
#define PP_PROJECT_HPP_INCLUDED

/**
 @file      pp_project.hpp
 @copyright François Becker
 @date      2017-2018
 */

#include "pp_tool.hpp"
#include "pp_layermorph.hpp"

#include <iostream>
#include <fstream>

namespace PP
{

    static QColor gLightSepia = QColor(0x70,0x42,0x14).lighter();

class Project
{
public:
    Project()
    : mPreview(100, 100, QImage::Format_ARGB32)
    , mTool(Tool::noRefillTool("SepiaPen", 1.f, gLightSepia, 1.f, 10))
    {
    }
    
    void setImagePath(std::string pImagePath)
    {
        mImageFilePath = pImagePath;
        loadImage(mImageFilePath);
    }
    
    void loadImage(std::string pPath)
    {
        QImage lImage;
        lImage.load(pPath.c_str());
        setImage(lImage);
    }
    
    void setImage(QImage pImage)
    {
        mImage = pImage;
        mPreview = mImage.copy();
    }
    
    void setSaveRoot(std::string pPath)
    {
        mSaveRootPath = pPath;
    }

    std::string getSaveRoot() const
    {
        return mSaveRootPath;
    }

    void setTool(Tool pTool)
    {
        mTool = pTool;
    }

    void addLayer(float pThreshold)
    {
        mLayers.push_back(LayerMorph(pThreshold));
    }

    void compileProject()
    {
        std::string lPath = mSaveRootPath + ".gcode";
        std::ofstream lGCodeFile;
        lGCodeFile.open (lPath);
        lGCodeFile << "(GCode file generated by PaintPrint)" << std::endl;
        // TODO: add date
        
#if 1
        int i = 0;
        for (const auto& lLayer : mLayers)
        {
            ++i;
            lGCodeFile << "(LAYER " << i << ")" << std::endl;
            lLayer.compile(mImage, mPrintAreaXMM, mPrintAreaYMM, mWidthMM, mTool, lGCodeFile);
        }
#else
        // reverse
        int i = 0;
        for (auto lIt = mLayers.rbegin() ; lIt != mLayers.rend() ; ++lIt)
        {
            ++i;
            lGCodeFile << "(LAYER " << i << ")" << std::endl;
            lIt->compile(mImage, mPrintAreaXMM, mPrintAreaYMM, mWidthMM, mTool, lGCodeFile);
        }
#endif
        
        lGCodeFile.close();
    }
    
    float getWidthMM() const
    {
        return mWidthMM;
    }
    
    void setWidthMM(float pWidthMM)
    {
        mWidthMM = pWidthMM;
    }

    void setPrintArea(float pPrintAreaXMM, float pPrintAreaYMM)
    {
        mPrintAreaXMM = pPrintAreaXMM;
        mPrintAreaYMM = pPrintAreaYMM;
    }
    
    // Update the preview images of all layers, plus blends an image for the selected level
    void updatePreview(float pLevel = 1.f) const
    {
        mPreview.fill(Qt::white);
        
        float lNumLayers = mLayers.size();
        float lLimit = lNumLayers * pLevel;
        for (int i = 0 ; i < (int)std::min(lNumLayers, lLimit) ; ++i)
        {
            mLayers[i].blendPreview(mImage, mPreview, mTool, mWidthMM);
        }
    }

    int getNumLayers() const
    {
        return mLayers.size();
    }

    BinaryImage getLayerEssential(int pIndex) const
    {
        assert(pIndex >= 0);
        assert(pIndex < mLayers.size());
        return  mLayers[pIndex].essentialize(mImage, mWidthMM, mTool);
    }
    
    QImage& getPreview()
    {
        return mPreview;
    }
    
private:
    std::string mSaveRootPath;

    std::string mProjectFilePath;
    
    std::string mImageFilePath;
    
    QImage mImage;
    std::vector<LayerMorph> mLayers;
    float mPrintAreaXMM = 200.f;
    float mPrintAreaYMM = 200.f;
    float mWidthMM = 80.f;
    
    mutable QImage mPreview;
    
    Tool mTool;
};

}

#endif
