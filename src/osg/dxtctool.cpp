// dxtctool.cpp: implementation of DXTC Tools functions.
// 
// Copyright (C) 2002 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in dxtctool.h
//
//////////////////////////////////////////////////////////////////////

#include "dxtctool.h"


namespace dxtc_tool {

    const size_t dxtc_pixels::BSIZE_DXT1 = 8;
    const size_t dxtc_pixels::BSIZE_DXT3 = 16;
    const size_t dxtc_pixels::BSIZE_DXT5 = 16;
    const size_t dxtc_pixels::BSIZE_ALPHA_DXT3 = 8;
    const size_t dxtc_pixels::BSIZE_ALPHA_DXT5 = 8;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
// Members Functions
//////////////////////////////////////////////////////////////////////

bool dxtc_pixels::OpenGLSize() const
{
    size_t Width = m_Width;
    size_t Height = m_Height;
    // size_t TotalTrueBits = 0;

    if ((Width == 0) || (Height == 0))
        return false;

    for (; (Width % 2) == 0; Width /= 2);
    for (; (Height % 2) == 0; Height /= 2);

    if ((Width != 1) || (Height != 1))
        return false;
    else
        return true;
}



bool dxtc_pixels::VFlip() const
{
    // Check that the given dimentions are 2^x, 2^y
    if (! OpenGLSize())
        return false;

    // Check that the given format are supported
    if (! SupportedFormat())
        return false;

    // Nothing to do if Height == 1
    if (m_Height == 1)
        return true;
        
    if (DXT1())
        VFlip_DXT1();
    else if (DXT3())
        VFlip_DXT3();
    else if (DXT5())
        VFlip_DXT5();
    else
        return false; // We should never get there

    return true;
}



void dxtc_pixels::VFlip_DXT1() const
{
    // const size_t Size = ((m_Width + 3) / 4) * ((m_Height + 3) / 4) * BSIZE_DXT1;
    dxtc_int8 * pPixels = (dxtc_int8 * ) m_pPixels;

    if (m_Height == 2)
        for (size_t j = 0; j < (m_Width + 3) / 4; ++j)
            BVF_Color_H2(pPixels + j * BSIZE_DXT1);

    if (m_Height == 4)
        for (size_t j = 0; j < (m_Width + 3) / 4; ++j)
            BVF_Color_H4(pPixels + j * BSIZE_DXT1);

    if (m_Height > 4)
        for (size_t i = 0; i < ((m_Height + 7) / 8); ++i)
            for (size_t j = 0; j < (m_Width + 3) / 4; ++j) {
                const size_t TargetRow = ((m_Height + 3) / 4) - (i + 1);
                BVF_Color(GetBlock(i, j, BSIZE_DXT1), GetBlock(TargetRow, j, BSIZE_DXT1));
            }
}



void dxtc_pixels::VFlip_DXT3() const
{
    // const size_t Size = ((m_Width + 3) / 4) * ((m_Height + 3) / 4) * BSIZE_DXT3;
    // dxtc_int8 * const pPixels = (dxtc_int8 * const) m_pPixels;

    if (m_Height == 2)
        for (size_t j = 0; j < (m_Width + 3) / 4; ++j) {
            BVF_Alpha_DXT3_H2(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT3));
            BVF_Color_H2(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT3 + BSIZE_ALPHA_DXT3));
        }

    if (m_Height == 4)
        for (size_t j = 0; j < (m_Width + 3) / 4; ++j) {
            BVF_Alpha_DXT3_H4(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT3));
            BVF_Color_H4(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT3 + BSIZE_ALPHA_DXT3));
        }

    if (m_Height > 4)
        for (size_t i = 0; i < ((m_Height + 7) / 8); ++i)
            for (size_t j = 0; j < (m_Width + 3) / 4; ++j) {
                const size_t TargetRow = ((m_Height + 3) / 4) - (i + 1);
                BVF_Alpha_DXT3(GetBlock(i, j, BSIZE_DXT3), GetBlock(TargetRow, j, BSIZE_DXT3));
                BVF_Color(((dxtc_int8 * ) GetBlock(i, j, BSIZE_DXT3)) + BSIZE_ALPHA_DXT3, 
                          ((dxtc_int8 * ) GetBlock(TargetRow, j, BSIZE_DXT3)) + BSIZE_ALPHA_DXT3);
            }
}



void dxtc_pixels::VFlip_DXT5() const
{
    // const size_t Size = ((m_Width + 3) / 4) * ((m_Height + 3) / 4) * BSIZE_DXT5;
    // dxtc_int8 * const pPixels = (dxtc_int8 * const) m_pPixels;

    if (m_Height == 2)
        for (size_t j = 0; j < (m_Width + 3) / 4; ++j) {
            BVF_Alpha_DXT5_H2(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT5));
            BVF_Color_H2(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT5 + BSIZE_ALPHA_DXT5));
        }

    if (m_Height == 4)
        for (size_t j = 0; j < (m_Width + 3) / 4; ++j) {
            BVF_Alpha_DXT5_H4(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT5));
            BVF_Color_H4(((dxtc_int8 * ) m_pPixels) + (j * BSIZE_DXT5 + BSIZE_ALPHA_DXT5));
        }

    if (m_Height > 4)
        for (size_t i = 0; i < ((m_Height + 7) / 8); ++i)
            for (size_t j = 0; j < (m_Width + 3) / 4; ++j) {
                const size_t TargetRow = ((m_Height + 3) / 4) - (i + 1);
                BVF_Alpha_DXT5(GetBlock(i, j, BSIZE_DXT5), GetBlock(TargetRow, j, BSIZE_DXT5));
                BVF_Color(((dxtc_int8 * ) GetBlock(i, j, BSIZE_DXT5)) + BSIZE_ALPHA_DXT5, 
                          ((dxtc_int8 * ) GetBlock(TargetRow, j, BSIZE_DXT5)) + BSIZE_ALPHA_DXT5);
            }
}




}; // namespace dxtc_tool
