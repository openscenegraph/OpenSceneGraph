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
    // Check that the given dimensions are 2^x, 2^y
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

//
// Structure of a DXT-1 compressed texture block
// see page "Opaque and 1-Bit Alpha Textures (Direct3D 9)" on http://msdn.microsoft.com
// url at time of writing http://msdn.microsoft.com/en-us/library/bb147243(v=VS.85).aspx
//
struct DXT1TexelsBlock
{
    unsigned short color_0;     // colors at their
    unsigned short color_1;     // extreme
    unsigned int   texels4x4;   // interpolated colors (2 bits per texel)
};
struct DXT3TexelsBlock
{
    unsigned short alpha4[4];   // alpha values (4 bits per texel) - 64 bits
    unsigned short color_0;     // colors at their
    unsigned short color_1;     // extreme
    unsigned int   texels4x4;   // interpolated colors (2 bits per texel)
};

struct DXT5TexelsBlock
{
    unsigned char  alpha_0;     // alpha at their
    unsigned char  alpha_1;     // extreme
    unsigned char  alpha3[6];   // alpha index values (3 bits per texel)
    unsigned short color_0;     // colors at their
    unsigned short color_1;     // extreme
    unsigned int   texels4x4;   // interpolated colors (2 bits per texel)
};

bool isCompressedImageTranslucent(size_t width, size_t height, GLenum format, void * imageData)
{
    // OSG_NOTICE<<"isCompressedImageTranslucent("<<width<<", "<<height<<", "<<format<<", "<<imageData<<")"<<std::endl;
    int blockCount = ((width + 3) >> 2) * ((height + 3) >> 2);
    switch(format)
    {
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
            return false;

        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        {
            const DXT1TexelsBlock *texelsBlock = reinterpret_cast<const DXT1TexelsBlock*>(imageData);

            // Only do the check on the first mipmap level, and stop when we
            // see the first alpha texel
            int i = blockCount;
            while (i>0)
            {
                // See if this block might contain transparent texels
                if (texelsBlock->color_0<=texelsBlock->color_1)
                {
                    // Scan the texels block for the '11' bit pattern that
                    // indicates a transparent texel
                    int j = 0;
                    while (j < 32)
                    {
                        // Check for the '11' bit pattern on this texel
                        if ( ((texelsBlock->texels4x4 >> j) & 0x03) == 0x03)
                        {
                            // Texture is using the 1-bit alpha encoding, so we
                            return true;
                        }

                        // Next texel
                        j += 2;
                    }
                }

                // Next block
                --i;
                ++texelsBlock;
            }
            return false;
        }

        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        {
            const DXT3TexelsBlock *texelsBlock = reinterpret_cast<const DXT3TexelsBlock*>(imageData);
            // Only do the check on the first mipmap level, and stop when we see the first alpha texel
            int i = blockCount;
            while (i>0)
            {
                for (int j =0; j < 4; ++j)
                    if ( texelsBlock->alpha4[j] != 0xFFFF) //4 pixels at once
                            return true; //not fully opaque
                // Next block
                --i;
                ++texelsBlock;
            }
            return false;
        }
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
        {
            const DXT5TexelsBlock *texelsBlock = reinterpret_cast<const DXT5TexelsBlock*>(imageData);
            // Only do the check on the first mipmap level, and stop when we see the first alpha texel
            int i = blockCount;
            unsigned char alphaBlock[8];
            while (i>0)
            {
                bool eightStep = texelsBlock->alpha_0 > texelsBlock->alpha_1;
                alphaBlock[0] = texelsBlock->alpha_0;
                alphaBlock[1] = texelsBlock->alpha_1;
                if (eightStep) {
                    if (texelsBlock->alpha_0 < 255) return true; //not fully opaque
                    alphaBlock[2] = (6 * alphaBlock[0] + 1 * alphaBlock[1] + 3) / 7;    // bit code 010
                    alphaBlock[3] = (5 * alphaBlock[0] + 2 * alphaBlock[1] + 3) / 7;    // bit code 011
                    alphaBlock[4] = (4 * alphaBlock[0] + 3 * alphaBlock[1] + 3) / 7;    // bit code 100
                    alphaBlock[5] = (3 * alphaBlock[0] + 4 * alphaBlock[1] + 3) / 7;    // bit code 101
                    alphaBlock[6] = (2 * alphaBlock[0] + 5 * alphaBlock[1] + 3) / 7;    // bit code 110
                    alphaBlock[7] = (1 * alphaBlock[0] + 6 * alphaBlock[1] + 3) / 7;    // bit code 111
                } else {
                    alphaBlock[2] = (4 * alphaBlock[0] + 1 * alphaBlock[1] + 2) / 5;    // bit code 010
                    alphaBlock[3] = (3 * alphaBlock[0] + 2 * alphaBlock[1] + 2) / 5;    // bit code 011
                    alphaBlock[4] = (2 * alphaBlock[0] + 3 * alphaBlock[1] + 2) / 5;    // bit code 100
                    alphaBlock[5] = (1 * alphaBlock[0] + 4 * alphaBlock[1] + 2) / 5;    // bit code 101
                    alphaBlock[6] = 0;    // bit code 110
                    alphaBlock[7] = 255;    // bit code 111
                }

                int last_added_byte = 1;
                unsigned short running_a_index = texelsBlock->alpha3[0] + (((unsigned short)texelsBlock->alpha3[last_added_byte]) << 8);
                for (int j = 0; j < 16; ++j) {
                    unsigned char alphaIndex = running_a_index & 0x7;
                    if (alphaBlock[alphaIndex] < 255) return true; //not fully opaque
                    running_a_index >>= 3;
                    if ((3 * j / 8) == last_added_byte) {
                        ++last_added_byte;
                        //(&texelsBlock->alpha3[0]) to avoid gcc warning: array subscript is above array bounds [-Warray-bounds]
                        running_a_index += (((unsigned short)(&(texelsBlock->alpha3[0]))[last_added_byte]) << (8 - (3 * j & 0x7)));
                    }
                }
                // Next block
                --i;
                ++texelsBlock;
            }
            return false;
        }
        default:
            break;
    }

    return false;
}

unsigned short interpolateColors21(unsigned short color1, unsigned short color2) {
    unsigned short result = (((color1 >> 11) * 2 + (color2 >> 11)  + 1) / 3) << 11;
    result += (((color1 >> 5 & 0x3F) * 2 + (color2 >> 5 & 0x3F)  + 1) / 3) << 5;
    result += (((color1 & 0x1F) * 2 + (color2 & 0x1F)  + 1) / 3);
    return result;
}
unsigned short interpolateColors11(unsigned short color1, unsigned short color2) {
    unsigned short result = (((color1 >> 11)  + (color2 >> 11) ) / 2) << 11;
    result += (((color1 >> 5 & 0x3F)  + (color2 >> 5 & 0x3F)) / 2) << 5;
    result += (((color1 & 0x1F)  + (color2 & 0x1F) ) / 2);
    return result;
}

bool CompressedImageGetColor(unsigned char color[4], unsigned int s, unsigned int t, unsigned int r, int width, int height, int depth, GLenum format, unsigned char *imageData)
{
    unsigned short color16 = 0;//RGB 5:6:5 format


    unsigned int slab4Count = (depth & ~0x3); //4*floor(d/4)
    unsigned int col = (s >> 2);//(floor(x/4)
    unsigned int row = (t >> 2);//(floor(y/4)
    unsigned int blockWidth = (width + 3) >> 2;//ceil(w/4)
    unsigned int blockHeight = (height + 3) >> 2;//ceil(h/4)
    int blockNumber = col + blockWidth * row ; // block to jump to

    if (depth > 1) {
// https://www.opengl.org/registry/specs/NV/texture_compression_vtc.txt
//    if (z >= 4*floor(d/4)) {
//        blockIndex = blocksize * (ceil(w/4) * ceil(h/4) * 4*floor(d/4) + floor(x/4) + ceil(w/4) * (floor(y/4) + ceil(h/4) * (z-4*floor(d/4)) ));
//    } else {
//        blockIndex = blocksize * 4 * (floor(x/4) + ceil(w/4) * (floor(y/4) + ceil(h/4) * floor(z/4)));
//    }
// note floor(a/4) = (a >> 2)
// note 4*floor(a/4) = a & ~0x3
// note ceil(a/4) = ((a + 3) >> 2)
//
//  rewrite: this describes the final blocks as consecutive 4x4x1 blocks - and thats not in the wording of the specs
//    if (r >= slab4Count) {
//        blockNumber = (blockWidth * blockHeight * slab4Count  + col + blockWidth * (row + blockHeight * (r-slab4Count) ));
//    } else {
//      blockNumber = 4 * (col + blockWidth * (row + blockHeight * (r >> 2)) );
//    }

// or in the version of the openGL specs:
//    if (z >= 4*floor(d/4)) {
//        blockIndex = blocksize * (ceil(w/4) * ceil(h/4) * 4*floor(d/4) + (z - 4*floor(d/4)) * ( (floor(x/4) + ceil(w/4) * (floor(y/4) );
//    } else {
//        blockIndex = blocksize * 4 * (floor(x/4) + ceil(w/4) * (floor(y/4) + ceil(h/4) * floor(z/4)));
//    }

    unsigned int sub_r = r & 0x3;//(r-slab4Count)
    if (r >= slab4Count) { //slice number beyond  4x4x4 slabs
        unsigned int blockDepth = depth & 0x3;// equals: depth - slab4Count;//depth of this final block: 1/2/3 in case of 4x4x1; 4x4x2 or 4x4x3 bricks
        blockNumber = (blockWidth * blockHeight * slab4Count //jump full 4x4x4 slabs
        + blockDepth * ( col + blockWidth * row )
        + sub_r);
    } else {
        blockNumber = 4 * (col + blockWidth * (row + blockHeight * (r >> 2)) ) + sub_r;
    }
    }

    int sub_s = s & 0x3;
    int sub_t = t & 0x3;
    switch (format)
    {
    case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT) :
    case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) :
    {
        const DXT1TexelsBlock *texelsBlock = reinterpret_cast<const DXT1TexelsBlock*>(imageData);
        texelsBlock += blockNumber; //jump to block
        char index = (texelsBlock->texels4x4 >> (2 * sub_s + 8 * sub_t)) & 0x3; //two bit "index value"
        color[3] = 255;
        switch (index) {
        case 0:
            color16 = texelsBlock->color_0;
            break;
        case 1:
            color16 = texelsBlock->color_1;
            break;
        case 2:
            if (texelsBlock->color_0 > texelsBlock->color_1) {
                color16 = interpolateColors21(texelsBlock->color_0, texelsBlock->color_1);
            }
            else {
                color16 = interpolateColors11(texelsBlock->color_0, texelsBlock->color_1);
            }
            break;
        case 3:
            if (texelsBlock->color_0 > texelsBlock->color_1) {
                color16 = interpolateColors21(texelsBlock->color_1, texelsBlock->color_0);
            }
            else {
                color16 = 0;//black
                if (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) color[3] = 0;//transparent
            }
            break;
        }
        break;
    }
    case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) :
    {
        const DXT3TexelsBlock *texelsBlock = reinterpret_cast<const DXT3TexelsBlock*>(imageData);
        texelsBlock += blockNumber; //jump to block
        color[3] = 17 * (texelsBlock->alpha4[sub_t] >> 4 * sub_s & 0xF);
        char index = (texelsBlock->texels4x4 >> (2 * sub_s + 8 * sub_t)) & 0x3; //two bit "index value"
        switch (index) {
        case 0:
            color16 = texelsBlock->color_0;
            break;
        case 1:
            color16 = texelsBlock->color_1;
            break;
        case 2:
            color16 = interpolateColors21(texelsBlock->color_0, texelsBlock->color_1);
            break;
        case 3:
            color16 = interpolateColors21(texelsBlock->color_1, texelsBlock->color_0);
            break;
        }
        break;
    }
    case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) :
    {
        const DXT5TexelsBlock *texelsBlock = reinterpret_cast<const DXT5TexelsBlock*>(imageData);
        texelsBlock += blockNumber; //jump to block
        char index = (texelsBlock->texels4x4 >> (2 * sub_s + 8 * sub_t)) & 0x3; //two bit "index value"
        switch (index) {
        case 0:
            color16 = texelsBlock->color_0;
            break;
        case 1:
            color16 = texelsBlock->color_1;
            break;
        case 2:
            color16 = interpolateColors21(texelsBlock->color_0, texelsBlock->color_1);
            break;
        case 3:
            color16 = interpolateColors21(texelsBlock->color_1, texelsBlock->color_0);
            break;
        }
        char pixel = sub_s + 4 * sub_t;//pixel number in block: 0 - 15
        char firstBit = 3 * pixel;//least significant bit: range 0 - 45
        unsigned char alpha_index;
        if ((firstBit & 0x7) < 6) {
            alpha_index = texelsBlock->alpha3[firstBit >> 3] >> (firstBit & 0x7) & 0x7;//grab byte containing least significant bit; shift and get 3 bits
        } else {
            alpha_index = texelsBlock->alpha3[firstBit >> 3] >> (firstBit & 0x7);
            alpha_index |= texelsBlock->alpha3[1 + (firstBit >> 3)] << (8 - (firstBit & 0x7));
            alpha_index &= 0x7;
        }
        if (alpha_index == 0) {
            color[3] = texelsBlock->alpha_0;
        } else {
            if (alpha_index == 1) {
                color[3] = texelsBlock->alpha_1;
            } else {
                if (texelsBlock->alpha_0 > texelsBlock->alpha_1) {
                    color[3] = ((unsigned short)texelsBlock->alpha_0  * (8 - alpha_index) + (unsigned short)texelsBlock->alpha_1 * (alpha_index - 1) + 3) / 7;
                } else {
                    if (alpha_index < 6) {
                        color[3] = ((unsigned short)texelsBlock->alpha_0  * (6 - alpha_index) + (unsigned short)texelsBlock->alpha_1 * (alpha_index - 1) + 3) / 5;
                    } else {
                        if (alpha_index == 6) {
                            color[3] = 0;
                        } else {
                            color[3] = 255;
                        }
                    }
                }
            }
        }
        break;
    }
    default:
        return false;
    }
    unsigned short colorChannel = color16 >> 11;//red - 5 bits
    color[0] = colorChannel << 3 | colorChannel >> 2 ;
    colorChannel = color16 >> 5 & 0x3F;//green - 6 bits
    color[1] = colorChannel << 2 | colorChannel >> 3;
    colorChannel = color16 & 0x1F;//blue - 5 bits
    color[2] = colorChannel << 3 | colorChannel >> 2;
    return true;
}
void compressedBlockOrientationConversion(const GLenum format, const unsigned char *src_block, unsigned char *dst_block, const osg::Vec3i& srcOrigin, const osg::Vec3i& rowDelta, const osg::Vec3i& columnDelta)
{
    unsigned int src_texels4x4;
    unsigned int *dst_texels4x4 = NULL;
    switch (format)
    {
    case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT) :
    case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) :
    {
        const DXT1TexelsBlock *src_texelsBlock = reinterpret_cast<const DXT1TexelsBlock*>(src_block);
        //make a copy as source might be equal to destination
        src_texels4x4 = src_texelsBlock->texels4x4;   // interpolated colors (2 bits per texel)
        DXT1TexelsBlock *dst_texelsBlock = reinterpret_cast<DXT1TexelsBlock*>(dst_block);
        dst_texels4x4 = &dst_texelsBlock->texels4x4;

        break;
    }
    case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) :
    {
        const DXT3TexelsBlock *src_texelsBlock = reinterpret_cast<const DXT3TexelsBlock*>(src_block);
        //make a copy as source might be equal to destination
        src_texels4x4 = src_texelsBlock->texels4x4;   // interpolated colors (2 bits per texel)
        DXT3TexelsBlock *dst_texelsBlock = reinterpret_cast<DXT3TexelsBlock*>(dst_block);
        dst_texels4x4 = &dst_texelsBlock->texels4x4;
        unsigned short src_alpha4[4];   // alpha values (4 bits per texel) - 64 bits

        memcpy(src_alpha4, src_texelsBlock->alpha4, 4 * sizeof(unsigned short));//make a copy as source might be equal to destination

        memset(dst_texelsBlock->alpha4, 0, 4 * sizeof(unsigned short)); //clear
        osg::Vec3i source_pixel(srcOrigin);
        for (int r = 0; r<4; r++)//rows
        {
            for (int c = 0; c<4; c++)//columns
            {
                int sub_s = source_pixel.x() & 0x3;
                int sub_t = source_pixel.y() & 0x3;
                int shiftBits = 4 * sub_s;
                unsigned int alpha_value = src_alpha4[sub_t] >> shiftBits & 0xf; //four bit alpha values

                shiftBits = 4 * c;//destination
                alpha_value <<= shiftBits;
                dst_texelsBlock->alpha4[r] |= alpha_value;

                source_pixel = source_pixel + rowDelta;
            }
            source_pixel = source_pixel + columnDelta;
        }
        break;
    }
    case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) :
    {
        const DXT5TexelsBlock *src_texelsBlock = reinterpret_cast<const DXT5TexelsBlock*>(src_block);
        //make a copy as source might be equal to destination
        src_texels4x4 = src_texelsBlock->texels4x4;   // interpolated colors (2 bits per texel)
        DXT5TexelsBlock *dst_texelsBlock = reinterpret_cast<DXT5TexelsBlock*>(dst_block);
        dst_texels4x4 = &dst_texelsBlock->texels4x4;

        unsigned char  src_alpha3[6];   // alpha index values (3 bits per texel)

        memcpy(src_alpha3, src_texelsBlock->alpha3, 6 * sizeof(unsigned char));//make a copy as source might be equal to destination

        memset(dst_texelsBlock->alpha3, 0, 6 * sizeof(unsigned char)); //clear
        osg::Vec3i source_pixel(srcOrigin);
        unsigned int last_added_byte = 1;
        unsigned short running_a_index = src_texelsBlock->alpha3[0] + (((unsigned short)src_texelsBlock->alpha3[last_added_byte]) << 8);
        unsigned int j = 0;
        for (int r = 0; r<4; r++)//rows
        {
            for (int c = 0; c<4; c++)//columns
            {
                int sub_s = source_pixel.x() & 0x3;
                int sub_t = source_pixel.y() & 0x3;

                unsigned char alphaIndex = running_a_index & 0x7;
                //store alphaIndex in output position:
                int shiftBits = 3 * sub_s + 12 * sub_t;//LSB
                dst_texelsBlock->alpha3[shiftBits >> 3] |= alphaIndex << (shiftBits & 0x7);
                if ((shiftBits & 0x7) > 5) {
                    dst_texelsBlock->alpha3[1 + (shiftBits >> 3)] |= alphaIndex >> (8 - (shiftBits & 0x7));
                }

                running_a_index >>= 3;
                if ((3 * ++j / 8) == last_added_byte) {
                    ++last_added_byte;
                    //(&texelsBlock->alpha3[0]) to avoid gcc warning: array subscript is above array bounds [-Warray-bounds]
                    running_a_index += (((unsigned short)(&(src_texelsBlock->alpha3[0]))[last_added_byte]) << (8 - (3 * j & 0x7)));
                }
                source_pixel = source_pixel + rowDelta;
            }
            source_pixel = source_pixel + columnDelta;
        }
        break;
    }
    default:
        return;
    }//switch

    //all formats: rearrange the colors
    *dst_texels4x4 = 0;//clear
    osg::Vec3i source_pixel(srcOrigin);
    for (int r = 0; r<4; r++)//rows
    {
        for (int c = 0; c<4; c++)//columns
        {
            int sub_s = source_pixel.x() & 0x3;
            int sub_t = source_pixel.y() & 0x3;
            int shiftBits = 2 * sub_s + 8 * sub_t;
            unsigned int index = (src_texels4x4 >> (shiftBits)) & 0x3; //two bit "index value"

            shiftBits = 2 * c + 8 * r;//destination
            index <<= shiftBits;
            *dst_texels4x4 |= index;

            source_pixel = source_pixel + rowDelta;
        }
        source_pixel = source_pixel + columnDelta;
    }
}

void compressedBlockStripAlhpa(const GLenum format, const unsigned char *src_block, unsigned char *dst_block) {
    unsigned int src_texels4x4;
    char reshuffle[4] = { 1, 0, 3, 2 };
    switch (format)
    {
    default:
    case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT) :
    case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) :
    {
        const DXT1TexelsBlock *src_texelsBlock = reinterpret_cast<const DXT1TexelsBlock*>(src_block);
        //make a copy as source might be equal to destination
        src_texels4x4 = src_texelsBlock->texels4x4;   // interpolated colors (2 bits per texel)
        DXT1TexelsBlock *dst_texelsBlock = reinterpret_cast<DXT1TexelsBlock*>(dst_block);
        if (src_texelsBlock->color_0 > src_texelsBlock->color_1) {
            // Four-color block
            memcpy(dst_texelsBlock, src_texelsBlock, sizeof(DXT1TexelsBlock));
        } else {
            dst_texelsBlock->color_0 = src_texelsBlock->color_1;
            dst_texelsBlock->color_1 = src_texelsBlock->color_0;
            dst_texelsBlock->texels4x4 = 0;
            for (unsigned int shiftBits = 0; shiftBits < 32; shiftBits += 2) {
                unsigned char index = src_texels4x4 >> shiftBits & 0x3; //two bit "index value"
                dst_texelsBlock->texels4x4 |= reshuffle[index] << shiftBits;
            }

        }
        break;
    }
    case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) :
    case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) :
    {
        const DXT3TexelsBlock *src_texelsBlock = reinterpret_cast<const DXT3TexelsBlock*>(src_block);
        //make a copy as source might be equal to destination
        src_texels4x4 = src_texelsBlock->texels4x4;   // interpolated colors (2 bits per texel)
        DXT1TexelsBlock *dst_texelsBlock = reinterpret_cast<DXT1TexelsBlock*>(dst_block);
        if (src_texelsBlock->color_0 > src_texelsBlock->color_1) {
            // Four-color block
            memcpy(dst_texelsBlock, src_texelsBlock, sizeof(DXT3TexelsBlock));
        }
        else {
            dst_texelsBlock->color_0 = src_texelsBlock->color_1;
            dst_texelsBlock->color_1 = src_texelsBlock->color_0;
            dst_texelsBlock->texels4x4 = 0;
            for (unsigned int shiftBits = 0; shiftBits < 32; shiftBits += 2) {
                unsigned char index = src_texels4x4 >> shiftBits & 0x3; //two bit "index value"
                dst_texelsBlock->texels4x4 |= reshuffle[index] << shiftBits;

            }

        }
        break;
    }
    }
}
} // namespace dxtc_tool
