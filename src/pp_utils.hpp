#ifndef PP_UTILS_HPP_INCLUDED
#define PP_UTILS_HPP_INCLUDED

/**
 @file      pp_utils.hpp
 @copyright François Becker
 @date      2017-2018
 */

#include "pp_utils.hpp"

#include <deque>
#include <list>

#include <iostream>
#include <fstream>
#include <cmath>

namespace PP
{

struct PointPixel
{
    int mX;
    int mY;
    
    static bool areNeighbour8(PointPixel p1, PointPixel p2)
    {
        return std::abs(p2.mX - p1.mX) <= 1 && std::abs(p2.mY - p1.mY) <= 1;
    }
};

struct VectorPixel
: public PointPixel
{
    VectorPixel(int pX, int pY)
    : PointPixel({pX, pY})
    {
    }
};

inline VectorPixel operator -(PointPixel a, PointPixel b)
{
    return VectorPixel(b.mX - a.mX, b.mY - a.mY);
}

inline float cross(VectorPixel a, VectorPixel b)
{
    return a.mX * b.mY - a.mY * b.mX;
}

struct LinePathPixel
{
    int mFromX;
    int mFromY;
    int mToX;
    int mToY;
};

struct CombinedPathsPixels
{
    std::list<PointPixel> mPoints;
    CombinedPathsPixels(std::initializer_list<PointPixel> pPoints)
    : mPoints(pPoints)
    {
    }
};

struct PointMM
{
    float mX;
    float mY;
    static float length(PointMM p1, PointMM p2)
    {
        return std::hypot(p2.mX - p1.mX, p2.mY - p1.mY);
    }
};

inline bool operator !=(PointMM a, PointMM b)
{
    return ((a.mX != b.mX) || (a.mY != b.mY));
}

struct VectorMM
: public PointMM
{
    VectorMM(float pX, float pY)
    : PointMM({pX, pY})
    {
    }
    float length() const
    {
        return std::hypot(mX, mY);
    }
    VectorMM normalized() const
    {
        float lLength = length();
        assert(lLength != 0.f);
        float lInvLength = 1.f / lLength;
        return VectorMM(mX * lInvLength, mY * lInvLength);
    }
};

inline VectorMM operator -(PointMM a, PointMM b)
{
    return VectorMM(a.mX - b.mX, a.mY - b.mY);
}

inline VectorMM operator *(float s, VectorMM v)
{
    return VectorMM(s * v.mX, s * v.mY);
}

inline PointMM operator +(PointMM p, VectorMM v)
{
    return PointMM{p.mX + v.mX, p.mY + v.mY};
}

struct SegmentMM
{
    PointMM mFrom;
    PointMM mTo;
    float length() const
    {
        return PointMM::length(mFrom, mTo);
    }
};

struct CombinedPathMM
{
    std::deque<PointMM> mPoints;
    CombinedPathMM()
    {
    }
    CombinedPathMM(std::initializer_list<PointMM> pPoints)
    : mPoints(pPoints)
    {
    }
    float length() const
    {
        float lLength = 0.f;
        for (int i = 0 ; i < mPoints.size() - 1 ; ++i)
        {
            lLength += PointMM::length(mPoints[i], mPoints[i+1]);
        }
        return lLength;
    }
    void fixDragError(float pToolDragErrorMM)
    {
        if (mPoints.size() < 2 || pToolDragErrorMM == 0.f)
        {
            return;
        }
        std::deque<PointMM> lNewPoints;
        PointMM lCurrent = mPoints.front();
        PointMM lCurrentHead = lCurrent;
        lNewPoints.push_front(lCurrent);
        
        for (auto lIt = std::next(mPoints.begin()) ; lIt != mPoints.end() ; ++lIt)
        {
            PointMM lNew = (*lIt);
            
            assert(lNew != lCurrent);
            
            VectorMM lDrag = pToolDragErrorMM * ((lNew - lCurrent).normalized());
            PointMM lP1 = lCurrent + lDrag;
            PointMM lP2 = lNew + lDrag;
            
            lNewPoints.push_back(lP1);
            lNewPoints.push_back(lP2);
            
            lCurrent = lNew;
            lCurrentHead = lP2;
        }
        if (lCurrent != lCurrentHead)
        {
            lNewPoints.push_back(lCurrent);
        }
        
        mPoints = lNewPoints;
    }
    void compile(std::ofstream& pOut) const
    {
        pOut << "G0 Z2.5" << std::endl;
        pOut << "G0 X" << mPoints.front().mX << " Y" << mPoints.front().mY << std::endl;
        pOut << "G0 Z0" << std::endl;
        for (PointMM lPoint : mPoints)
        {
            pOut << "G1 X" << lPoint.mX << " Y" << lPoint.mY << std::endl;
        }
        pOut << "G0 Z2.5" << std::endl;
    }
};

class BinaryImage
{
public:
    BinaryImage(size_t pWidth, size_t pHeight, bool pClear = true)
    : mWidth(pWidth)
    , mHeight(pHeight)
    {
        alloc();
        if (pClear)
        {
            clear();
        }
    }
    
    BinaryImage(const BinaryImage& pImage)
    : mWidth(pImage.getWidth())
    , mHeight(pImage.getHeight())
    {
        alloc();
        
        for (size_t u = 0 ; u != mWidth * mHeight ; ++u)
        {
            mData[u] = pImage.mData[u];
        }
    }
    
    BinaryImage(const QImage& pImage, float pThreshold = 0.5f)
    : mWidth(pImage.width())
    , mHeight(pImage.height())
    {
        alloc();
        for (unsigned int y = 0 ; y != mHeight ; ++y)
        {
            for (unsigned int x = 0 ; x != mWidth ; ++x)
            {
                getPixel(x, y) = (pImage.pixelColor(x, y).lightnessF() >= pThreshold);
            }
        }
    }
    
    ~BinaryImage()
    {
        delete []mData;
    }
    
    size_t getWidth() const
    {
        return mWidth;
    }
    
    size_t getHeight() const
    {
        return mHeight;
    }
    
    bool& getPixel(int x, int y)
    {
        assert(x < mWidth);
        assert(y < mHeight);
        size_t lIndex = y * mWidth + x;
        return mData[lIndex];
    }
    
    const bool& getPixel(int x, int y) const
    {
        assert(x < mWidth);
        assert(y < mHeight);
        size_t lIndex = y * mWidth + x;
        return mData[lIndex];
    }
    
    void invert()
    {
        for (size_t u = 0 ; u != mWidth * mHeight ; ++u)
        {
            bool& lPixel = mData[u];
            lPixel = !lPixel;
        }
    }
    
    bool isEmpty() const
    {
        size_t u = 0;
        while (u < mWidth * mHeight && ! mData[u])
        {
            ++u;
        }
        return u == mWidth * mHeight;
    }
    
    void add(const BinaryImage& pOther)
    {
        assert(mWidth == pOther.mWidth);
        assert(mHeight == pOther.mHeight);
        for (size_t u = 0 ; u != mWidth * mHeight ; ++u)
        {
            mData[u] |= pOther.mData[u];
        }
    }
    
    void clear()
    {
        for (size_t u = 0 ; u != mWidth * mHeight ; ++u)
        {
            mData[u] = false;
        }
    }
    
    QImage toImage() const
    {
        QImage lReturn((int)mWidth, (int)mHeight, QImage::Format_ARGB32);
        for (unsigned int y = 0 ; y != mHeight ; ++y)
        {
            for (unsigned int x = 0 ; x != mWidth ; ++x)
            {
                lReturn.setPixelColor(x, y, getPixel(x, y) ? Qt::white : Qt::black);
            }
        }
        return lReturn;
    }
    
private:
    void alloc()
    {
        mData = new bool[mWidth * mHeight];
    }
    
    size_t mWidth = 0;
    size_t mHeight = 0;
    bool* mData = nullptr;
};

namespace MorphOps
{
    static void thin(BinaryImage& pImage)
    {
        // http://agcggs680.pbworks.com/f/Zhan-Suen_algorithm.pdf
        
        // add a border
        BinaryImage lOriginal(pImage.getWidth() + 2, pImage.getHeight() + 2);
        for (unsigned int y = 0 ; y != pImage.getHeight() ; ++y)
        {
            for (unsigned int x = 0 ; x != pImage.getWidth() ; ++x)
            {
                lOriginal.getPixel(x + 1, y + 1) = pImage.getPixel(x, y);
            }
        }
        
        BinaryImage lThinned1(lOriginal);
        
        // Pass 1
        for (unsigned int y = 1 ; y != lThinned1.getHeight() - 1 ; ++y)
        {
            for (unsigned int x = 1 ; x != lThinned1.getWidth() - 1 ; ++x)
            {
                if (lOriginal.getPixel(x, y))
                {
                    bool p2 = lOriginal.getPixel(x - 1, y    );
                    bool p3 = lOriginal.getPixel(x - 1, y + 1);
                    bool p4 = lOriginal.getPixel(x    , y + 1);
                    bool p5 = lOriginal.getPixel(x + 1, y + 1);
                    bool p6 = lOriginal.getPixel(x + 1, y    );
                    bool p7 = lOriginal.getPixel(x + 1, y - 1);
                    bool p8 = lOriginal.getPixel(x    , y - 1);
                    bool p9 = lOriginal.getPixel(x - 1, y - 1);
                    
                    int p2i = p2 ? 1 : 0;
                    int p3i = p3 ? 1 : 0;
                    int p4i = p4 ? 1 : 0;
                    int p5i = p5 ? 1 : 0;
                    int p6i = p6 ? 1 : 0;
                    int p7i = p7 ? 1 : 0;
                    int p8i = p8 ? 1 : 0;
                    int p9i = p9 ? 1 : 0;
                    
                    int lConditionASum = p2i + p3i + p4i + p5i + p6i + p7i + p8i + p9i;
                    bool lConditionA = (lConditionASum >= 2 && lConditionASum <= 6);
                    
                    int lConditionBSum = ((!p2 && p3) ? 1 : 0)
                                       + ((!p3 && p4) ? 1 : 0)
                                       + ((!p4 && p5) ? 1 : 0)
                                       + ((!p5 && p6) ? 1 : 0)
                                       + ((!p6 && p7) ? 1 : 0)
                                       + ((!p7 && p8) ? 1 : 0)
                                       + ((!p8 && p9) ? 1 : 0)
                                       + ((!p9 && p2) ? 1 : 0);
                    bool lConditionB = (lConditionBSum == 1);
                    
                    bool lConditionC = (p2i * p4i * p6i == 0);
                    
                    bool lConditionD = (p4i * p6i * p8i == 0);
                    
                    if (lConditionA && lConditionB && lConditionC && lConditionD)
                    {
                        lThinned1.getPixel(x, y) = false;
                    }
                }
            }
        }
        
        BinaryImage lThinned2(lThinned1);
        
        // Pass 2
        for (unsigned int y = 1 ; y != lThinned2.getHeight() - 1 ; ++y)
        {
            for (unsigned int x = 1 ; x != lThinned2.getWidth() - 1 ; ++x)
            {
                if (lThinned1.getPixel(x, y))
                {
                    bool p2 = lThinned1.getPixel(x - 1, y    );
                    bool p3 = lThinned1.getPixel(x - 1, y + 1);
                    bool p4 = lThinned1.getPixel(x    , y + 1);
                    bool p5 = lThinned1.getPixel(x + 1, y + 1);
                    bool p6 = lThinned1.getPixel(x + 1, y    );
                    bool p7 = lThinned1.getPixel(x + 1, y - 1);
                    bool p8 = lThinned1.getPixel(x    , y - 1);
                    bool p9 = lThinned1.getPixel(x - 1, y - 1);
                    
                    int p2i = p2 ? 1 : 0;
                    int p3i = p3 ? 1 : 0;
                    int p4i = p4 ? 1 : 0;
                    int p5i = p5 ? 1 : 0;
                    int p6i = p6 ? 1 : 0;
                    int p7i = p7 ? 1 : 0;
                    int p8i = p8 ? 1 : 0;
                    int p9i = p9 ? 1 : 0;
                    
                    int lConditionASum = p2i + p3i + p4i + p5i + p6i + p7i + p8i + p9i;
                    bool lConditionA = (lConditionASum >= 2 && lConditionASum <= 6);
                    
                    int lConditionBSum = ((!p2 && p3) ? 1 : 0)
                                       + ((!p3 && p4) ? 1 : 0)
                                       + ((!p4 && p5) ? 1 : 0)
                                       + ((!p5 && p6) ? 1 : 0)
                                       + ((!p6 && p7) ? 1 : 0)
                                       + ((!p7 && p8) ? 1 : 0)
                                       + ((!p8 && p9) ? 1 : 0)
                                       + ((!p9 && p2) ? 1 : 0);
                    bool lConditionB = (lConditionBSum == 1);
                    
                    bool lConditionC = (p2i * p4i * p8i == 0);
                    
                    bool lConditionD = (p2i * p6i * p8i == 0);
                    
                    if (lConditionA && lConditionB && lConditionC && lConditionD)
                    {
                        lThinned2.getPixel(x, y) = false;
                    }
                }
            }
        }
        
        for (unsigned int y = 0 ; y != pImage.getHeight() ; ++y)
        {
            for (unsigned int x = 0 ; x != pImage.getWidth() ; ++x)
            {
                pImage.getPixel(x, y) = lThinned2.getPixel(x + 1, y + 1);
            }
        }
    }
    
    static void erode(BinaryImage& pImage)
    {
        // add a border
        BinaryImage lOriginal(pImage.getWidth() + 2, pImage.getHeight() + 2);
        for (unsigned int y = 0 ; y != pImage.getHeight() ; ++y)
        {
            for (unsigned int x = 0 ; x != pImage.getWidth() ; ++x)
            {
                lOriginal.getPixel(x + 1, y + 1) = pImage.getPixel(x, y);
            }
        }
        
        BinaryImage lEroded(lOriginal);
        
        for (unsigned int y = 1 ; y != lEroded.getHeight() - 1 ; ++y)
        {
            for (unsigned int x = 1 ; x != lEroded.getWidth() - 1 ; ++x)
            {
                if (lOriginal.getPixel(x, y))
                {
                    bool p2 = lOriginal.getPixel(x - 1, y    );
                    bool p3 = lOriginal.getPixel(x - 1, y + 1);
                    bool p4 = lOriginal.getPixel(x    , y + 1);
                    bool p5 = lOriginal.getPixel(x + 1, y + 1);
                    bool p6 = lOriginal.getPixel(x + 1, y    );
                    bool p7 = lOriginal.getPixel(x + 1, y - 1);
                    bool p8 = lOriginal.getPixel(x    , y - 1);
                    bool p9 = lOriginal.getPixel(x - 1, y - 1);
                    lEroded.getPixel(x,y) = (p2 && p3 && p4 && p5 && p6 && p7 && p8 && p9);
                }
            }
        }
        
        for (unsigned int y = 0 ; y != pImage.getHeight() ; ++y)
        {
            for (unsigned int x = 0 ; x != pImage.getWidth() ; ++x)
            {
                pImage.getPixel(x, y) = lEroded.getPixel(x + 1, y + 1);
            }
        }
    }
    
#if 0
    /**
     Border of a binary image
     */
    static BinaryImage border(const BinaryImage& pImage)
    {
        BinaryImage lReturn(pImage);
        const int cWidth = (int)lReturn.getWidth();
        const int cHeight = (int)lReturn.getHeight();
        for (unsigned int y = 1 ; y < cHeight - 1 ; ++y)
        {
            for (unsigned int x = 1 ; x < cWidth - 1 ; ++x)
            {
                bool& lPixel = lReturn.getPixel(x, y);
                lPixel = lPixel &&
                    (! lReturn.getPixel(x - 1, y - 1) ||
                     ! lReturn.getPixel(x - 1, y    ) ||
                     ! lReturn.getPixel(x - 1, y + 1) ||
                     ! lReturn.getPixel(x    , y - 1) ||
                     ! lReturn.getPixel(x    , y + 1) ||
                     ! lReturn.getPixel(x + 1, y - 1) ||
                     ! lReturn.getPixel(x + 1, y    ) ||
                     ! lReturn.getPixel(x + 1, y + 1));
            }
        }
        // everything that is at the border of the image is a border
        return lReturn;
    }
#endif
    
    /**
     Subtract the border of a binary image and return this border
     */
    static BinaryImage removeBorder(BinaryImage& pImage)
    {
        BinaryImage lReturn(pImage);
        const int cWidth = (int)lReturn.getWidth();
        const int cHeight = (int)lReturn.getHeight();
        for (unsigned int y = 1 ; y < cHeight - 1 ; ++y)
        {
            for (unsigned int x = 1 ; x < cWidth - 1 ; ++x)
            {
                bool& lPixel = lReturn.getPixel(x, y);
                lPixel = lPixel &&
                (//   ! pImage.getPixel(x - 1, y - 1)
                      ! pImage.getPixel(x - 1, y    )
                 //|| ! pImage.getPixel(x - 1, y + 1)
                   || ! pImage.getPixel(x    , y - 1)
                   || ! pImage.getPixel(x    , y + 1)
                 //|| ! pImage.getPixel(x + 1, y - 1)
                   || ! pImage.getPixel(x + 1, y    )
                 //|| ! pImage.getPixel(x + 1, y + 1)
                 );
            }
        }
        for (unsigned int y = 1 ; y < cHeight - 1 ; ++y)
        {
            for (unsigned int x = 1 ; x < cWidth - 1 ; ++x)
            {
                bool& lPixel = lReturn.getPixel(x, y);
                if (lPixel)
                {
                    pImage.getPixel(x, y) = false;
                }
            }
        }
        
        // everything that is at the border of the image is a border
        for (int x = 0 ; x != cWidth ; ++x)
        {
            pImage.getPixel(x, 0) = false;
            pImage.getPixel(x, cHeight - 1) = false;
        }
        for (int y = 0 ; y != cHeight ; ++y)
        {
            pImage.getPixel(0, y) = false;
            pImage.getPixel(cWidth - 1, y) = false;
        }
        return lReturn;
    }
    
    template< typename T >
    typename std::vector<T>::iterator
    insert_sorted( std::vector<T> & vec, T const& item )
    {
        return vec.insert
        (
         std::upper_bound( vec.begin(), vec.end(), item ),
         item
         );
    }


    template<>
    std::vector<uint8_t>::iterator
    insert_sorted(std::vector<uint8_t>&, uint8_t const&);

#if 0
    static void accumulateMedian(const QImage& pSrc, int pX, int pY, std::vector<uint8_t>& pRed, std::vector<uint8_t>& pGreen, std::vector<uint8_t>& pBlue)
    {
        QColor lColour = pSrc.pixelColor(pX, pY);
        insert_sorted(pRed, (uint8_t)lColour.red());
        insert_sorted(pGreen, (uint8_t)lColour.green());
        insert_sorted(pBlue, (uint8_t)lColour.blue());
    }
    
    static QImage median(const QImage& pSrc)
    {
        std::vector<uint8_t> lRed;
        std::vector<uint8_t> lGreen;
        std::vector<uint8_t> lBlue;
        QImage lMedian = pSrc.copy();
        for (int y = 1 ; y < pSrc.height() - 1 ; ++y)
        {
            for (int x = 1 ; x < pSrc.width() - 1 ; ++x)
            {
                lRed.clear();
                lGreen.clear();
                lBlue.clear();
                accumulateMedian(pSrc, x-1, y-1, lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x-1, y  , lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x-1, y+1, lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x  , y-1, lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x  , y  , lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x  , y+1, lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x+1, y-1, lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x+1, y  , lRed, lGreen, lBlue);
                accumulateMedian(pSrc, x+1, y+1, lRed, lGreen, lBlue);
                QColor lNewColour(lRed[4], lGreen[4], lBlue[4]);
                lMedian.setPixelColor(x, y, lNewColour);
            }
        }
        return lMedian;
    }
    
    static QImage threshold(const QImage& pSrc, float pThreshold)
    {
        QImage lThresholded = pSrc.copy();
        for (int y = 0 ; y != pSrc.height() ; ++y)
        {
            for (int x = 0 ; x != pSrc.width() ; ++x)
            {
                bool lComputed = (pSrc.pixelColor(x, y).lightnessF() < pThreshold);
                lThresholded.setPixelColor(x, y, lComputed ? Qt::black : Qt::white);
            }
        }
        return lThresholded;
    }
#endif
    
    /**
     TODO: misnamed?
     */
    static void median(BinaryImage& pSrcDst)
    {
        for (int y = 1 ; y < pSrcDst.getHeight() - 1 ; ++y)
        {
            for (int x = 1 ; x < pSrcDst.getWidth() - 1 ; ++x)
            {
                if (pSrcDst.getPixel(x-1, y-1) &&
                    pSrcDst.getPixel(x-1, y  ) &&
                    pSrcDst.getPixel(x-1, y+1) &&
                    pSrcDst.getPixel(x  , y-1) &&
                    !pSrcDst.getPixel(x  , y  ) &&
                    pSrcDst.getPixel(x  , y+1) &&
                    pSrcDst.getPixel(x+1, y-1) &&
                    pSrcDst.getPixel(x+1, y  ) &&
                    pSrcDst.getPixel(x+1, y+1))
                {
                    pSrcDst.getPixel(x, y) = true;
                }
                else if (!pSrcDst.getPixel(x-1, y-1) &&
                         !pSrcDst.getPixel(x-1, y  ) &&
                         !pSrcDst.getPixel(x-1, y+1) &&
                         !pSrcDst.getPixel(x  , y-1) &&
                         pSrcDst.getPixel(x  , y  ) &&
                         !pSrcDst.getPixel(x  , y+1) &&
                         !pSrcDst.getPixel(x+1, y-1) &&
                         !pSrcDst.getPixel(x+1, y  ) &&
                         !pSrcDst.getPixel(x+1, y+1))
                {
                    pSrcDst.getPixel(x, y) = false;
                }
            }
        }
    }
    
    static BinaryImage diagonalDown(const BinaryImage& pSrc, int pStepPixels)
    {
        BinaryImage lDst(pSrc.getWidth(), pSrc.getHeight());
        
        int lStepPixelsDiagonal = (int)((float)pStepPixels);// * (float)M_SQRT2);
        
        for (int i = 0 ; i < lDst.getWidth() ; i += lStepPixelsDiagonal)
        {
            int x = i;
            int y = 0;
            while (x < lDst.getWidth() && y < lDst.getHeight())
            {
                lDst.getPixel(x, y) = pSrc.getPixel(x, y);
                ++x;
                ++y;
            }
        }
        
        for (int i = lStepPixelsDiagonal ; i < lDst.getHeight() ; i += lStepPixelsDiagonal)
        {
            int x = 0;
            int y = i;
            while (x < lDst.getWidth() && y < lDst.getHeight())
            {
                lDst.getPixel(x, y) = pSrc.getPixel(x, y);
                ++x;
                ++y;
            }
        }
        
        return lDst;
    }
    
    static BinaryImage diagonalUp(const BinaryImage& pSrc, int pStepPixels)
    {
        BinaryImage lDst(pSrc.getWidth(), pSrc.getHeight());
        
        int lStepPixelsDiagonal = (int)((float)pStepPixels);// * (float)M_SQRT2);
        
        for (int i = (int)lDst.getHeight() - 1 ; i >= 0 ; i -= lStepPixelsDiagonal)
        {
            int x = 0;
            int y = i;
            while (x < lDst.getWidth() && y >= 0)
            {
                lDst.getPixel(x, y) = pSrc.getPixel(x, y);
                ++x;
                --y;
            }
        }
        
        for (int i = 0 ; i < lDst.getWidth() ; i += lStepPixelsDiagonal)
        {
            int x = i;
            int y = (int)lDst.getHeight() - 1;
            while (x < lDst.getWidth() && y >= 0)
            {
                lDst.getPixel(x, y) = pSrc.getPixel(x, y);
                ++x;
                --y;
            }
        }
        
        return lDst;
    }

    static BinaryImage diagonal(const BinaryImage& pSrc, int pStepPixels, bool pDirection)
    {
        if (pDirection)
        {
            return diagonalDown(pSrc, pStepPixels);
        }
        else
        {
            return diagonalUp(pSrc, pStepPixels);
        }
    }
}

}

#endif
