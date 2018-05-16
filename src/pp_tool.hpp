#ifndef PP_TOOL_HPP_INCLUDED
#define PP_TOOL_HPP_INCLUDED

/**
 @file      pp_tool.hpp
 @copyright François Becker
 @date      2017-2018
 */

#include <QColor>

#include <string>

namespace PP
{
    class Tool
    {
    public:
        static Tool noRefillTool(std::string pName, float pWidthMM, QColor pColour, float pDragErrorMM, int pDryTimeSeconds)
        {
            return Tool(pName, pWidthMM, pColour, pDragErrorMM, false, std::numeric_limits<float>::max(), "\n", pDryTimeSeconds);
        }

        static Tool refillingTool(std::string pName, float pWidthMM, QColor pColour, float pDragErrorMM, float pLengthBeforeRefillMM, std::string pRefillCommand, int pDryTimeSeconds)
        {
            return Tool(pName, pWidthMM, pColour, pDragErrorMM, true, pLengthBeforeRefillMM, pRefillCommand, pDryTimeSeconds);
        }

        float getWidthMM() const
        {
            return mWidthMM;
        }
        
        QColor getColour() const
        {
            return mColour;
        }

        std::string getRefillCommand() const
        {
            return mRefillCommand;
        }

        float getDragErrorMM() const
        {
            return mDragErrorMM;
        }

        bool getNeedsRefill() const
        {
            return mNeedsRefill;
        }

        float getLengthBeforeRefillMM() const
        {
            return mLengthBeforeRefillMM;
        }

        int getDryTimeSeconds() const
        {
            return mDryTimeSeconds;
        }

    protected:
        Tool(std::string pName, float pWidthMM, QColor pColour, float pDragErrorMM, bool pNeedsRefill, float pLengthBeforeRefillMM, std::string pRefillCommand, int pDryTimeSeconds)
        : mName(pName)
        , mWidthMM(pWidthMM)
        , mColour(pColour)
        , mDragErrorMM(pDragErrorMM)
        , mNeedsRefill(pNeedsRefill)
        , mLengthBeforeRefillMM(pLengthBeforeRefillMM)
        , mRefillCommand(pRefillCommand)
        , mDryTimeSeconds(pDryTimeSeconds)
        {
        }

        std::string mName;
        float  mWidthMM;
        QColor mColour;
        float  mDragErrorMM;
        bool   mNeedsRefill;
        float  mLengthBeforeRefillMM;
        std::string mRefillCommand;
        int    mDryTimeSeconds;
    };
}

#endif
