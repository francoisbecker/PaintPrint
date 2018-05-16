#ifndef PP_LAYER_HPP_INCLUDED
#define PP_LAYER_HPP_INCLUDED

/**
 @file      pp_layer.hpp
 @copyright François Becker
 @date      2017-2018
 */

#include "pp_tool.hpp"

#include <QImage>

namespace PP
{
    class Layer
    {
    public:
        Layer() {}

        virtual ~Layer() {}

        virtual void blendPreview(const QImage& pSrc, QImage& pBlendedImage, const Tool& pTool, float pWidthMM) const = 0;

        virtual void compile(QImage pImage, float pZoneSizeMMX, float pZoneSizeMMY, float pWidthMM, const Tool& pTool, std::ofstream& pOut) const = 0;
    };
}

#endif
