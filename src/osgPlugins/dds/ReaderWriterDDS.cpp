/**********************************************************************
*
*    FILE:            ReaderWriterdds.cpp
*
*    DESCRIPTION:    Class for reading a DDS file into an osg::Image.
*
*                    Example on reading a DDS file code can be found at:
*                    http://developer.nvidia.com/docs/IO/1327/ATT/
*                    ARB_texture_compression.pdf
*                    Author: Sebastien Domine, NVIDIA Corporation
*
*    CREATED BY:     Rune Schmidt Jensen, rsj@uni-dk
*
*    HISTORY:        Created   31.03.2003
*             Modified  13.05.2004
*                by George Tarantilis, gtaranti@nps.navy.mil
*             Modified  22.05.2009
*                Wojtek Lewandowski, lewandowski@ai.com.pl
*
*    WARNING:
*          Bit Masks in the WrtiteDDS are set for 8 bit components
*          write with 4 or 16 bit components will
*          probably produce corrupted file
*          Wojtek Lewandowski 2009-05-22
*
**********************************************************************/
#include <osg/Texture>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <iomanip>
#include <stdio.h>
#include <string.h>

// Macro similar to what's in FLT/TRP plugins (except it uses wide char under Windows if OSG_USE_UTF8_FILENAME)
#if defined(_WIN32)
    #include <windows.h>
    #include <osg/Config>
    #include <osgDB/ConvertUTF>
    #ifdef OSG_USE_UTF8_FILENAME
        #define DELETEFILE(file) DeleteFileW(osgDB::convertUTF8toUTF16((file)).c_str())
    #else
        #define DELETEFILE(file) DeleteFileA((file))
    #endif

#else   // Unix

    #include <stdio.h>
    #define DELETEFILE(file) remove((file))

#endif

#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    #define GL_RED                  0x1903
    #define GL_LUMINANCE4_ALPHA4    0x8043
#endif

#if defined(OSG_GL3_AVAILABLE)
    #define GL_LUMINANCE4_ALPHA4    0x8043
#endif

// NOTICE ON WIN32:
// typedef DWORD unsigned long;
// sizeof(DWORD) = 4

typedef unsigned int UI32;
typedef int I32;

struct  DDCOLORKEY
{
    DDCOLORKEY():
        dwColorSpaceLowValue(0),
        dwColorSpaceHighValue(0) {}

    UI32    dwColorSpaceLowValue;
    UI32    dwColorSpaceHighValue;
};

struct DDPIXELFORMAT
{

    DDPIXELFORMAT():
        dwSize(0),
        dwFlags(0),
        dwFourCC(0),
        dwRGBBitCount(0),
        dwRBitMask(0),
        dwGBitMask(0),
        dwBBitMask(0),
        dwRGBAlphaBitMask(0) {}


    UI32    dwSize;
    UI32    dwFlags;
    UI32    dwFourCC;
    union
    {
        UI32    dwRGBBitCount;
        UI32    dwYUVBitCount;
        UI32    dwZBufferBitDepth;
        UI32    dwAlphaBitDepth;
        UI32    dwLuminanceBitDepth;
    };
    union
    {
        UI32    dwRBitMask;
        UI32    dwYBitMask;
    };
    union
    {
        UI32    dwGBitMask;
        UI32    dwUBitMask;
    };
    union
    {
        UI32    dwBBitMask;
        UI32    dwVBitMask;
    };
    union
    {
        UI32    dwRGBAlphaBitMask;
        UI32    dwYUVAlphaBitMask;
        UI32    dwRGBZBitMask;
        UI32    dwYUVZBitMask;
    };
};

struct  DDSCAPS2
{
     DDSCAPS2():
        dwCaps(0),
        dwCaps2(0),
        dwCaps3(0),
        dwCaps4(0) {}

    UI32       dwCaps;
    UI32       dwCaps2;
    UI32       dwCaps3;
    union
    {
        UI32       dwCaps4;
        UI32       dwVolumeDepth;
    };
};

struct DDSURFACEDESC2
{
    DDSURFACEDESC2():
        dwSize(0),
        dwFlags(0),
        dwHeight(0),
        dwWidth(0),
        lPitch(0),
        dwBackBufferCount(0),
        dwMipMapCount(0),
        dwAlphaBitDepth(0),
        dwReserved(0),
        lpSurface(0),
        dwTextureStage(0) {}


    UI32         dwSize;
    UI32         dwFlags;
    UI32         dwHeight;
    UI32         dwWidth;
    union
    {
        I32              lPitch;
        UI32     dwLinearSize;
    };
    union
    {
        UI32      dwBackBufferCount;
        UI32      dwDepth;
    };
    union
    {
        UI32     dwMipMapCount;
        UI32     dwRefreshRate;
    };
    UI32         dwAlphaBitDepth;
    UI32         dwReserved;
    UI32        lpSurface;         //Fred Marmond: removed from pointer type to UI32 for 64bits compatibility. it is unused data
    DDCOLORKEY    ddckCKDestOverlay;
    DDCOLORKEY    ddckCKDestBlt;
    DDCOLORKEY    ddckCKSrcOverlay;
    DDCOLORKEY    ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2      ddsCaps;
    UI32 dwTextureStage;
};

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

//
// DDSURFACEDESC2 flags that mark the validity of the struct data
//
#define DDSD_CAPS               0x00000001l     // default
#define DDSD_HEIGHT             0x00000002l        // default
#define DDSD_WIDTH              0x00000004l        // default
#define DDSD_PIXELFORMAT        0x00001000l        // default
#define DDSD_PITCH              0x00000008l     // For uncompressed formats
#define DDSD_MIPMAPCOUNT        0x00020000l
#define DDSD_LINEARSIZE         0x00080000l     // For compressed formats
#define DDSD_DEPTH              0x00800000l        // Volume Textures

//
// DDPIXELFORMAT flags
//
#define DDPF_ALPHAPIXELS        0x00000001l
#define DDPF_FOURCC             0x00000004l        // Compressed formats
#define DDPF_PALETTEINDEXED8    0x00000020l
#define DDPF_RGB                0x00000040l        // Uncompressed formats
#define DDPF_ALPHA              0x00000002l
#define DDPF_COMPRESSED         0x00000080l
#define DDPF_LUMINANCE          0x00020000l
#define DDPF_BUMPLUMINANCE      0x00040000l        // L,U,V
#define DDPF_BUMPDUDV           0x00080000l        // U,V

//
// DDSCAPS flags
//
#define DDSCAPS_TEXTURE         0x00001000l     // default
#define DDSCAPS_COMPLEX         0x00000008l
#define DDSCAPS_MIPMAP          0x00400000l
#define DDSCAPS2_VOLUME         0x00200000l


#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
    ((UI32)(char)(ch0) | ((UI32)(char)(ch1) << 8) |   \
    ((UI32)(char)(ch2) << 16) | ((UI32)(char)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

/*
* FOURCC codes for DX compressed-texture pixel formats
*/
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

/*
* FOURCC codes for 3dc compressed-texture pixel formats
*/
#define FOURCC_ATI1  (MAKEFOURCC('A','T','I','1'))
#define FOURCC_ATI2  (MAKEFOURCC('A','T','I','2'))

/*
* FOURCC codes for DX10 files
*/
#define FOURCC_DX10  (MAKEFOURCC('D','X','1','0'))

typedef enum OSG_DXGI_FORMAT {
  OSG_DXGI_FORMAT_UNKNOWN                     = 0,
  OSG_DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
  OSG_DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
  OSG_DXGI_FORMAT_R32G32B32A32_UINT           = 3,
  OSG_DXGI_FORMAT_R32G32B32A32_SINT           = 4,
  OSG_DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
  OSG_DXGI_FORMAT_R32G32B32_FLOAT             = 6,
  OSG_DXGI_FORMAT_R32G32B32_UINT              = 7,
  OSG_DXGI_FORMAT_R32G32B32_SINT              = 8,
  OSG_DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
  OSG_DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
  OSG_DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
  OSG_DXGI_FORMAT_R16G16B16A16_UINT           = 12,
  OSG_DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
  OSG_DXGI_FORMAT_R16G16B16A16_SINT           = 14,
  OSG_DXGI_FORMAT_R32G32_TYPELESS             = 15,
  OSG_DXGI_FORMAT_R32G32_FLOAT                = 16,
  OSG_DXGI_FORMAT_R32G32_UINT                 = 17,
  OSG_DXGI_FORMAT_R32G32_SINT                 = 18,
  OSG_DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
  OSG_DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
  OSG_DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
  OSG_DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
  OSG_DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
  OSG_DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
  OSG_DXGI_FORMAT_R10G10B10A2_UINT            = 25,
  OSG_DXGI_FORMAT_R11G11B10_FLOAT             = 26,
  OSG_DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
  OSG_DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
  OSG_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
  OSG_DXGI_FORMAT_R8G8B8A8_UINT               = 30,
  OSG_DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
  OSG_DXGI_FORMAT_R8G8B8A8_SINT               = 32,
  OSG_DXGI_FORMAT_R16G16_TYPELESS             = 33,
  OSG_DXGI_FORMAT_R16G16_FLOAT                = 34,
  OSG_DXGI_FORMAT_R16G16_UNORM                = 35,
  OSG_DXGI_FORMAT_R16G16_UINT                 = 36,
  OSG_DXGI_FORMAT_R16G16_SNORM                = 37,
  OSG_DXGI_FORMAT_R16G16_SINT                 = 38,
  OSG_DXGI_FORMAT_R32_TYPELESS                = 39,
  OSG_DXGI_FORMAT_D32_FLOAT                   = 40,
  OSG_DXGI_FORMAT_R32_FLOAT                   = 41,
  OSG_DXGI_FORMAT_R32_UINT                    = 42,
  OSG_DXGI_FORMAT_R32_SINT                    = 43,
  OSG_DXGI_FORMAT_R24G8_TYPELESS              = 44,
  OSG_DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
  OSG_DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
  OSG_DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
  OSG_DXGI_FORMAT_R8G8_TYPELESS               = 48,
  OSG_DXGI_FORMAT_R8G8_UNORM                  = 49,
  OSG_DXGI_FORMAT_R8G8_UINT                   = 50,
  OSG_DXGI_FORMAT_R8G8_SNORM                  = 51,
  OSG_DXGI_FORMAT_R8G8_SINT                   = 52,
  OSG_DXGI_FORMAT_R16_TYPELESS                = 53,
  OSG_DXGI_FORMAT_R16_FLOAT                   = 54,
  OSG_DXGI_FORMAT_D16_UNORM                   = 55,
  OSG_DXGI_FORMAT_R16_UNORM                   = 56,
  OSG_DXGI_FORMAT_R16_UINT                    = 57,
  OSG_DXGI_FORMAT_R16_SNORM                   = 58,
  OSG_DXGI_FORMAT_R16_SINT                    = 59,
  OSG_DXGI_FORMAT_R8_TYPELESS                 = 60,
  OSG_DXGI_FORMAT_R8_UNORM                    = 61,
  OSG_DXGI_FORMAT_R8_UINT                     = 62,
  OSG_DXGI_FORMAT_R8_SNORM                    = 63,
  OSG_DXGI_FORMAT_R8_SINT                     = 64,
  OSG_DXGI_FORMAT_A8_UNORM                    = 65,
  OSG_DXGI_FORMAT_R1_UNORM                    = 66,
  OSG_DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
  OSG_DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
  OSG_DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
  OSG_DXGI_FORMAT_BC1_TYPELESS                = 70,
  OSG_DXGI_FORMAT_BC1_UNORM                   = 71,
  OSG_DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
  OSG_DXGI_FORMAT_BC2_TYPELESS                = 73,
  OSG_DXGI_FORMAT_BC2_UNORM                   = 74,
  OSG_DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
  OSG_DXGI_FORMAT_BC3_TYPELESS                = 76,
  OSG_DXGI_FORMAT_BC3_UNORM                   = 77,
  OSG_DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
  OSG_DXGI_FORMAT_BC4_TYPELESS                = 79,
  OSG_DXGI_FORMAT_BC4_UNORM                   = 80,
  OSG_DXGI_FORMAT_BC4_SNORM                   = 81,
  OSG_DXGI_FORMAT_BC5_TYPELESS                = 82,
  OSG_DXGI_FORMAT_BC5_UNORM                   = 83,
  OSG_DXGI_FORMAT_BC5_SNORM                   = 84,
  OSG_DXGI_FORMAT_B5G6R5_UNORM                = 85,
  OSG_DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
  OSG_DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
  OSG_DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
  OSG_DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
  OSG_DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
  OSG_DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
  OSG_DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
  OSG_DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
  OSG_DXGI_FORMAT_BC6H_TYPELESS               = 94,
  OSG_DXGI_FORMAT_BC6H_UF16                   = 95,
  OSG_DXGI_FORMAT_BC6H_SF16                   = 96,
  OSG_DXGI_FORMAT_BC7_TYPELESS                = 97,
  OSG_DXGI_FORMAT_BC7_UNORM                   = 98,
  OSG_DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
  OSG_DXGI_FORMAT_AYUV                        = 100,
  OSG_DXGI_FORMAT_Y410                        = 101,
  OSG_DXGI_FORMAT_Y416                        = 102,
  OSG_DXGI_FORMAT_NV12                        = 103,
  OSG_DXGI_FORMAT_P010                        = 104,
  OSG_DXGI_FORMAT_P016                        = 105,
  OSG_DXGI_FORMAT_420_OPAQUE                  = 106,
  OSG_DXGI_FORMAT_YUY2                        = 107,
  OSG_DXGI_FORMAT_Y210                        = 108,
  OSG_DXGI_FORMAT_Y216                        = 109,
  OSG_DXGI_FORMAT_NV11                        = 110,
  OSG_DXGI_FORMAT_AI44                        = 111,
  OSG_DXGI_FORMAT_IA44                        = 112,
  OSG_DXGI_FORMAT_P8                          = 113,
  OSG_DXGI_FORMAT_A8P8                        = 114,
  OSG_DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
  OSG_DXGI_FORMAT_FORCE_UINT                  = 0xffffffffUL
} OSG_DXGI_FORMAT;

typedef enum OSG_D3D10_RESOURCE_DIMENSION {
  OSG_D3D10_RESOURCE_DIMENSION_UNKNOWN    = 0,
  OSG_D3D10_RESOURCE_DIMENSION_BUFFER     = 1,
  OSG_D3D10_RESOURCE_DIMENSION_TEXTURE1D  = 2,
  OSG_D3D10_RESOURCE_DIMENSION_TEXTURE2D  = 3,
  OSG_D3D10_RESOURCE_DIMENSION_TEXTURE3D  = 4
} OSG_D3D10_RESOURCE_DIMENSION;

typedef struct {
  OSG_DXGI_FORMAT              dxgiFormat;
  OSG_D3D10_RESOURCE_DIMENSION resourceDimension;
  UI32                     miscFlag;
  UI32                     arraySize;
  UI32                     reserved;
} OSG_DDS_HEADER_DXT10;

static unsigned int ComputeImageSizeInBytes( int width, int height, int depth,
                                             unsigned int pixelFormat, unsigned int pixelType,
                                             int packing = 1, int slice_packing = 1, int image_packing = 1 )
{
    if( width < 1 )  width = 1;
    if( height < 1 ) height = 1;
    if( depth < 1 )  depth = 1;

    return osg::Image::computeImageSizeInBytes(width, height, depth, pixelFormat, pixelType, packing, slice_packing, image_packing);
}

osg::Image* ReadDDSFile(std::istream& _istream, bool flipDDSRead)
{
    DDSURFACEDESC2 ddsd;

    char filecode[4];

    _istream.read(filecode, 4);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        return NULL;
    }
    // Get the surface desc.
    _istream.read((char*)(&ddsd), sizeof(ddsd));

    osg::ref_ptr<osg::Image> osgImage = new osg::Image();

    //Check valid structure sizes
    if(ddsd.dwSize != 124 && ddsd.ddpfPixelFormat.dwSize != 32)
    {
        return NULL;
    }

    int depth = 1;

    // Check for volume image
    if( ddsd.dwDepth > 0 && (ddsd.dwFlags & DDSD_DEPTH))
    {
        depth = ddsd.dwDepth;
    }

    // Retrieve image properties.
    int s = ddsd.dwWidth;
    int t = ddsd.dwHeight;
    int r = depth;
    unsigned int dataType = GL_UNSIGNED_BYTE;
    unsigned int pixelFormat = 0;
    unsigned int internalFormat = 0;

    // Handle some esoteric formats
    if(ddsd.ddpfPixelFormat.dwFlags & DDPF_BUMPDUDV)
    {
        OSG_WARN << "ReadDDSFile warning: DDPF_BUMPDUDV format is not supported" << std::endl;
        return NULL;
//         ddsd.ddpfPixelFormat.dwFlags =
//             DDPF_LUMINANCE + DDPF_ALPHAPIXELS;
//         // handle V8U8 as A8L8
//         // handle V16U16 as A16L16
//         // but Q8W8U8L8 as RGB?
//         // A2W10U10V10 as RGBA (dwFlags == DDPF_BUMPDUDV + DDPF_ALPHAPIXELS)
    }
    if(ddsd.ddpfPixelFormat.dwFlags & DDPF_BUMPLUMINANCE)
    {
        OSG_WARN << "ReadDDSFile warning: DDPF_BUMPLUMINANCE format is not supported" << std::endl;
        return NULL;
//         ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
//         // handle as RGB
//         // L6V5U5 -- 655 is not supported data type in GL
//         // X8L8V8U8 -- just as RGB
    }

    // Uncompressed formats will usually use DDPF_RGB to indicate an RGB format,
    // while compressed formats will use DDPF_FOURCC with a four-character code.

    bool usingAlpha = ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS;
    int packing(1);
    bool isDXTC(false);

    // Compressed formats
    if(ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
    {
        switch(ddsd.ddpfPixelFormat.dwFourCC)
        {
        case FOURCC_DXT1:
            OSG_INFO << "ReadDDSFile info : format = DXT1, usingAlpha=" <<usingAlpha<< std::endl;
            if (usingAlpha)
            {
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            }
            else
            {
                internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                pixelFormat    = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            }
            packing = 2;        // 4 bits/pixel. 4 px = 2 bytes
            isDXTC = true;
            break;
        case FOURCC_DXT3:
            OSG_INFO << "ReadDDSFile info : format = DXT3" << std::endl;
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            packing = 4;        // 8 bits/pixel. 4 px = 4 bytes
            isDXTC = true;
            break;
        case FOURCC_DXT5:
            OSG_INFO << "ReadDDSFile info : format = DXT5" << std::endl;
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            packing = 4;        // 8 bits/pixel. 4 px = 4 bytes
            isDXTC = true;
            break;
        case FOURCC_ATI1:
            OSG_INFO << "ReadDDSFile info : format = ATI1" << std::endl;
            internalFormat = GL_COMPRESSED_RED_RGTC1_EXT;
            pixelFormat    = GL_COMPRESSED_RED_RGTC1_EXT;
            break;
        case FOURCC_ATI2:
            OSG_INFO << "ReadDDSFile info : format = ATI2" << std::endl;
            internalFormat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
            pixelFormat    = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
            break;
        case 0x00000024: // A16B16G16R16
            OSG_INFO << "ReadDDSFile info : format = A16B16G16R16" << std::endl;
            internalFormat = GL_RGBA;
            pixelFormat    = GL_RGBA;
            dataType       = GL_UNSIGNED_SHORT;
            break;
        case 0x00000071: // A16B16G16R16F
            OSG_INFO << "ReadDDSFile info : format = A16B16G16R16F" << std::endl;
            internalFormat = GL_RGBA16F_ARB; // why no transparency?
            pixelFormat    = GL_RGBA;
            dataType       = GL_HALF_FLOAT;
            break;
        case 0x0000006E: // Q16W16V16U16
            OSG_INFO << "ReadDDSFile info : format = Q16W16V16U16" << std::endl;
            internalFormat = GL_RGBA;
            pixelFormat    = GL_RGBA;
            dataType       = GL_UNSIGNED_SHORT;
            break;
        case 0x00000070: // G16R16F
            OSG_INFO << "ReadDDSFile info : G16R16F format is not supported"
                                   << std::endl;
            return NULL;
//             internalFormat = GL_RGB;
//             pixelFormat    = must be GL_RED and GL_GREEN
//             dataType       = GL_HALF_FLOAT;
            break;
        case 0x00000073: // G32R32F
            OSG_INFO << "ReadDDSFile info : G32R32F format is not supported"
                                   << std::endl;
            return NULL;
//             internalFormat = GL_RGB;
//             pixelFormat    = must be GL_RED and GL_GREEN
//             dataType       = GL_FLOAT;
            break;
        case 0x00000072: // R32F
            OSG_INFO << "ReadDDSFile info : format = R32F" << std::endl;
            internalFormat = GL_R32F;
            pixelFormat    = GL_RED;
            dataType       = GL_FLOAT;
            break;
        case 0x0000006F: // R16F
            OSG_INFO << "ReadDDSFile info : format = R16F" << std::endl;
            internalFormat = GL_R16F;
            pixelFormat    = GL_RED;
            dataType       = GL_HALF_FLOAT;
            break;
        case 0x00000074: // A32B32G32R32F
            OSG_INFO << "ReadDDSFile info : format = A32B32G32R32F" << std::endl;
            internalFormat = GL_RGBA32F_ARB;
            pixelFormat    = GL_RGBA;
            dataType       = GL_FLOAT;
            break;
        case 0x00000075: // CxV8U8
            OSG_INFO << "ReadDDSFile info : CxV8U8 format is not supported" << std::endl;
            return NULL;

        case FOURCC_DX10:
            OSG_INFO << "ReadDDSFile info : format = DX10 file" << std::endl;
            {
                OSG_DDS_HEADER_DXT10 header10;
                _istream.read((char*)(&header10), sizeof(header10));
                switch (header10.dxgiFormat) {
                case OSG_DXGI_FORMAT_R32G32B32A32_FLOAT:
                    internalFormat = GL_RGBA32F_ARB;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R32G32B32A32_UINT:
                    internalFormat = GL_RGBA32UI_EXT;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_UNSIGNED_INT;
                    break;

                case OSG_DXGI_FORMAT_R32G32B32A32_SINT:
                    internalFormat = GL_RGBA32I_EXT;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_INT;
                    break;

                case OSG_DXGI_FORMAT_R32G32B32_FLOAT:
                    internalFormat = GL_RGB32F_ARB;
                    pixelFormat    = GL_RGB;
                    dataType       = GL_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R32G32B32_UINT:
                    internalFormat = GL_RGB32UI_EXT;
                    pixelFormat    = GL_RGB;
                    dataType       = GL_UNSIGNED_INT;
                    break;

                case OSG_DXGI_FORMAT_R32G32B32_SINT:
                    internalFormat = GL_RGB32I_EXT;
                    pixelFormat    = GL_RGB;
                    dataType       = GL_INT;
                    break;

                case OSG_DXGI_FORMAT_R16G16B16A16_FLOAT:
                    internalFormat = GL_RGBA16F_ARB;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_HALF_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R16G16B16A16_UNORM:
                    internalFormat = GL_RGBA16;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_UNSIGNED_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16G16B16A16_UINT:
                    internalFormat = GL_RGBA16UI_EXT;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_UNSIGNED_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16G16B16A16_SNORM:
                    internalFormat = GL_RGBA16_SNORM;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16G16B16A16_SINT:
                    internalFormat = GL_RGBA16I_EXT;
                    pixelFormat    = GL_RGBA;
                    dataType       = GL_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R32G32_FLOAT:
                    internalFormat = GL_RG32F;
                    pixelFormat    = GL_RG;
                    dataType       = GL_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R32G32_UINT:
                    internalFormat = GL_RG32UI;
                    pixelFormat    = GL_RG;
                    dataType       = GL_UNSIGNED_INT;
                    break;

                case OSG_DXGI_FORMAT_R32G32_SINT:
                    internalFormat = GL_RG32I;
                    pixelFormat    = GL_RG;
                    dataType       = GL_INT;
                    break;

                case OSG_DXGI_FORMAT_R16G16_FLOAT:
                    internalFormat = GL_RG16F;
                    pixelFormat    = GL_RG;
                    dataType       = GL_HALF_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R16G16_UNORM:
                    internalFormat = GL_RG16;
                    pixelFormat    = GL_RG;
                    dataType       = GL_UNSIGNED_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16G16_UINT:
                    internalFormat = GL_RG16UI;
                    pixelFormat    = GL_RG;
                    dataType       = GL_UNSIGNED_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16G16_SNORM:
                    internalFormat = GL_RG16_SNORM;
                    pixelFormat    = GL_RG;
                    dataType       = GL_UNSIGNED_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16G16_SINT:
                    internalFormat = GL_RG16I;
                    pixelFormat    = GL_RG;
                    dataType       = GL_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R32_FLOAT:
                    internalFormat = GL_R32F;
                    pixelFormat    = GL_RED;
                    dataType       = GL_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R32_UINT:
                    internalFormat = GL_R32UI;
                    pixelFormat    = GL_RED;
                    dataType       = GL_UNSIGNED_INT;
                    break;

                case OSG_DXGI_FORMAT_R32_SINT:
                    internalFormat = GL_R32I;
                    pixelFormat    = GL_RED;
                    dataType       = GL_INT;
                    break;

                case OSG_DXGI_FORMAT_R8G8_UNORM:
                    internalFormat = GL_RG;
                    pixelFormat    = GL_RG;
                    dataType       = GL_UNSIGNED_BYTE;
                    break;

                case OSG_DXGI_FORMAT_R8G8_UINT:
                    internalFormat = GL_RG8UI;
                    pixelFormat    = GL_RG;
                    dataType       = GL_UNSIGNED_BYTE;
                    break;

                case OSG_DXGI_FORMAT_R8G8_SNORM:
                    internalFormat = GL_RG_SNORM;
                    pixelFormat    = GL_RG;
                    dataType       = GL_BYTE;
                    break;

                case OSG_DXGI_FORMAT_R8G8_SINT:
                    internalFormat = GL_RG8I;
                    pixelFormat    = GL_RG;
                    dataType       = GL_BYTE;
                    break;

                case OSG_DXGI_FORMAT_R16_FLOAT:
                    internalFormat = GL_R16F;
                    pixelFormat    = GL_RED;
                    dataType       = GL_HALF_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R16_UNORM:
                    internalFormat = GL_RED;
                    pixelFormat    = GL_RED;
                    dataType       = GL_HALF_FLOAT;
                    break;

                case OSG_DXGI_FORMAT_R16_UINT:
                    internalFormat = GL_R16UI;
                    pixelFormat    = GL_RED;
                    dataType       = GL_UNSIGNED_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16_SNORM:
                    internalFormat = GL_RED_SNORM;
                    pixelFormat    = GL_RED;
                    dataType       = GL_SHORT;
                    break;

                case OSG_DXGI_FORMAT_R16_SINT:
                    internalFormat = GL_R16I;
                    pixelFormat    = GL_RED;
                    dataType       = GL_SHORT;
                    break;

                default:
                    OSG_WARN << "ReadDDSFile warning: unhandled DX10 pixel format 0x"
                             << std::hex << std::setw(8) << std::setfill('0')
                             << header10.dxgiFormat << std::dec
                             << " in dds file, image not loaded." << std::endl;
                    return NULL;
                }
            }
            break;

        case MAKEFOURCC( 'U', 'Y', 'V', 'Y' ): // not supported in OSG
        case MAKEFOURCC( 'U', 'Y', 'V', '2' ): // not supported in OSG
        case MAKEFOURCC( 'R', 'G', 'B', 'G' ): // R8G8_B8G8 -- what is it?
        case MAKEFOURCC( 'G', 'R', 'G', 'B' ): // G8R8_G8B8 -- what is it?
            //break;

        default:
            OSG_WARN << "ReadDDSFile warning: unhandled FOURCC pixel format ("
                                   << (char)((ddsd.ddpfPixelFormat.dwFourCC & 0x000000ff))
                                   << (char)((ddsd.ddpfPixelFormat.dwFourCC & 0x0000ff00) >> 8)
                                   << (char)((ddsd.ddpfPixelFormat.dwFourCC & 0x00ff0000) >> 16)
                                   << (char)((ddsd.ddpfPixelFormat.dwFourCC & 0xff000000) >> 24)
                                   << " = 0x" << std::hex << std::setw(8) << std::setfill('0')
                                   << ddsd.ddpfPixelFormat.dwFourCC << std::dec
                                   << ") in dds file, image not loaded." << std::endl;
            return NULL;
        }
    }
    // Uncompressed formats.
    else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
    {
        struct RGBFormat
        {
                const char*  name;
                UI32         bitCount;
                UI32         rBitMask;
                UI32         gBitMask;
                UI32         bBitMask;
                UI32         aBitMask;
                unsigned int internalFormat;
                unsigned int pixelFormat;
                unsigned int dataType;
        };

        const unsigned int UNSUPPORTED = 0;

        static const RGBFormat rgbFormats[] =
        {
            { "R3G3B2"     ,  8,       0xe0,       0x1c,       0x03,       0x00,
              GL_RGB , GL_RGB , GL_UNSIGNED_BYTE_3_3_2 },

            { "R5G6B5"     , 16,     0xf800,     0x07e0,     0x001f,     0x0000,
              GL_RGB , GL_RGB , GL_UNSIGNED_SHORT_5_6_5 },
            { "A1R5G5B5"   , 16,     0x7c00,     0x03e0,     0x001f,     0x8000,
              GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV },
            { "X1R5G5B5"   , 16,     0x7c00,     0x03e0,     0x001f,     0x0000,
              GL_RGB , GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV },
            { "A4R4G4B4"   , 16,     0x0f00,     0x00f0,     0x000f,     0xf000,
              GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV },
            { "X4R4G4B4"   , 16,     0x0f00,     0x00f0,     0x000f,     0x0000,
              GL_RGB , GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV },
            { "A8R3G3B2"   , 16,     0x00e0,     0x001c,     0x0003,     0xff00,
              GL_RGBA, GL_BGRA, UNSUPPORTED },

            { "R8G8B8",      24,   0xff0000,   0x00ff00,   0x0000ff,   0x000000,
              GL_RGB , GL_BGR , GL_UNSIGNED_BYTE },

            { "B8G8R8",      24,   0x0000ff,   0x00ff00,   0xff0000,   0x000000,
              GL_RGB , GL_RGB , GL_UNSIGNED_BYTE },

            { "A8R8G8B8",    32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000,
              GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE },
            { "X8R8G8B8",    32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000,
              GL_RGB , GL_BGRA, GL_UNSIGNED_BYTE },
            { "A8B8G8R8",    32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000,
              GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE },
            { "X8B8G8R8",    32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000,
              GL_RGB , GL_RGBA, GL_UNSIGNED_BYTE },
            { "A2R10G10B10", 32, 0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000,
              GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_2_10_10_10_REV },
            { "A2B10G10R10", 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000,
              GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV },
            { "G16R16",      32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000,
              GL_RGB, UNSUPPORTED, GL_UNSIGNED_SHORT },
            { "B16G16R16",   48,   0x0000ff,   0x00ff00,   0xff0000,   0x000000,
              GL_RGB16F_ARB , GL_RGB , GL_HALF_FLOAT },
            { "B32G32R32",   96,   0x0000ff,   0x00ff00,   0xff0000,   0x000000,
              GL_RGB32F_ARB , GL_RGB , GL_FLOAT },
        };

        bool found = false;

        for ( unsigned int i = 0; i < sizeof ( rgbFormats ) / sizeof ( RGBFormat ); i++ )
        {
            const RGBFormat& f = rgbFormats[ i ];
            if ( ddsd.ddpfPixelFormat.dwRGBBitCount     == f.bitCount &&
                 ddsd.ddpfPixelFormat.dwRBitMask        == f.rBitMask &&
                 ddsd.ddpfPixelFormat.dwGBitMask        == f.gBitMask &&
                 ddsd.ddpfPixelFormat.dwBBitMask        == f.bBitMask &&
                 ddsd.ddpfPixelFormat.dwRGBAlphaBitMask == f.aBitMask )
            {
                if ( f.internalFormat != UNSUPPORTED &&
                     f.pixelFormat    != UNSUPPORTED &&
                     f.dataType       != UNSUPPORTED )
                {
                    OSG_INFO << "ReadDDSFile info : format = " << f.name << std::endl;
                    internalFormat = f.internalFormat;
                    pixelFormat    = f.pixelFormat;
                    dataType       = f.dataType;
                    found = true;
                    break;
                }
                else
                {
                    OSG_INFO << "ReadDDSFile info : " << f.name
                                           << " format is not supported" << std::endl;
                    return NULL;
                }
            }
        }

        if ( !found )
        {
            OSG_WARN << "ReadDDSFile warning: unhandled RGB pixel format in dds file, image not loaded" << std::endl;
            OSG_INFO << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwRGBBitCount     = "
                                   << ddsd.ddpfPixelFormat.dwRGBBitCount << std::endl;
            OSG_INFO << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwRBitMask        = 0x"
                                   << std::hex << std::setw(8) << std::setfill('0')
                                   << ddsd.ddpfPixelFormat.dwRBitMask << std::endl;
            OSG_INFO << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwGBitMask        = 0x"
                                   << std::hex << std::setw(8) << std::setfill('0')
                                   << ddsd.ddpfPixelFormat.dwGBitMask << std::endl;
            OSG_INFO << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwBBitMask        = 0x"
                                   << std::hex << std::setw(8) << std::setfill('0')
                                   << ddsd.ddpfPixelFormat.dwBBitMask << std::endl;
            OSG_INFO << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x"
                                   << std::hex << std::setw(8) << std::setfill('0')
                                   << ddsd.ddpfPixelFormat.dwRGBAlphaBitMask << std::dec << std::endl;
            return NULL;
        }
    }
    else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
    {
            internalFormat = usingAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;
            pixelFormat    = usingAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;
            if ( usingAlpha && ddsd.ddpfPixelFormat.dwLuminanceBitDepth == 8 )
            {
                OSG_INFO << "ReadDDSFile info : format = L4A4" << std::endl;
                pixelFormat = GL_LUMINANCE4_ALPHA4; // invalid enumerant?
            }
            else if ( usingAlpha && ddsd.ddpfPixelFormat.dwLuminanceBitDepth == 32 )
            {
                OSG_INFO << "ReadDDSFile info : format = L16A16" << std::endl;
                dataType = GL_UNSIGNED_SHORT;
            }
            else if ( !usingAlpha && ddsd.ddpfPixelFormat.dwLuminanceBitDepth == 16 )
            {
                OSG_INFO << "ReadDDSFile info : format = L16" << std::endl;
                dataType = GL_UNSIGNED_SHORT;
            }
            else if ( usingAlpha )
            {
                OSG_INFO << "ReadDDSFile info : format = L8A8" << std::endl;
            }
            else
            {
                OSG_INFO << "ReadDDSFile info : format = L8" << std::endl;
            }
//             else if ( ddsd.ddpfPixelFormat.dwLuminanceBitDepth == (usingAlpha ? 64 : 32) )
//             {
//                 dataType = GL_UNSIGNED_INT;
//             }
    }
    else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHA)
    {
            OSG_INFO << "ReadDDSFile info : format = ALPHA" << std::endl;
            internalFormat = GL_ALPHA;
            pixelFormat    = GL_ALPHA;
    }
    else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
    {
            OSG_INFO << "ReadDDSFile info : format = PALETTEINDEXED8" << std::endl;
            // The indexed data needs to first be loaded as a single-component image.
            pixelFormat = GL_RED;
    }
    else
    {
        OSG_WARN << "ReadDDSFile warning: unhandled pixel format (ddsd.ddpfPixelFormat.dwFlags"
                               << " = 0x" << std::hex << std::setw(8) << std::setfill('0')
                               << ddsd.ddpfPixelFormat.dwFlags << std::dec
                               << ") in dds file, image not loaded."<<std::endl;
        return NULL;
    }

    unsigned int size = ComputeImageSizeInBytes( s, t, r, pixelFormat, dataType, packing );

    // Take care of mipmaps if any.
    unsigned int sizeWithMipmaps = size;
    osg::Image::MipmapDataType mipmap_offsets;
    if ( ddsd.dwMipMapCount>1 )
    {
        unsigned numMipmaps = osg::Image::computeNumberOfMipmapLevels( s, t, r );
        if( numMipmaps > ddsd.dwMipMapCount ) numMipmaps = ddsd.dwMipMapCount;
        // array starts at 1 level offset, 0 level skipped
        mipmap_offsets.resize( numMipmaps - 1 );

        int mip_width = s;
        int mip_height = t;
        int mip_depth = r;

        for( unsigned int k = 0; k < mipmap_offsets.size(); ++k  )
        {
           mipmap_offsets[k] = sizeWithMipmaps;

           mip_width = osg::maximum( mip_width >> 1, 1 );
           mip_height = osg::maximum( mip_height >> 1, 1 );
           mip_depth = osg::maximum( mip_depth >> 1, 1 );

           sizeWithMipmaps +=
                ComputeImageSizeInBytes( mip_width, mip_height, mip_depth, pixelFormat, dataType, packing );
        }
    }

    OSG_INFO<<"ReadDDS, dataType = 0x"<<std::hex<<dataType<<std::endl;

    unsigned char palette [1024];

    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
    {
        if (!_istream.read((char*)palette, 1024))
        {
            OSG_WARN << "ReadDDSFile warning: couldn't read palette" << std::endl;
            return NULL;
        }
    }

    unsigned char* imageData = new unsigned char [sizeWithMipmaps];
    if(!imageData)
    {
        OSG_WARN << "ReadDDSFile warning: imageData == NULL" << std::endl;
        return NULL;
    }

    // Read pixels in two chunks. First main image, next mipmaps.
    if ( !_istream.read( (char*)imageData, size ) )
    {
        delete [] imageData;
        OSG_WARN << "ReadDDSFile warning: couldn't read imageData" << std::endl;
        return NULL;
    }

    // If loading mipmaps in second chunk fails we may still use main image
    if ( size < sizeWithMipmaps && !_istream.read( (char*)imageData + size, sizeWithMipmaps - size ) )
    {
        sizeWithMipmaps = size;
        mipmap_offsets.resize( 0 );
        OSG_WARN << "ReadDDSFile warning: couldn't read mipmapData" << std::endl;

        // if mipmaps read failed we leave some not used overhead memory allocated past main image
        // this memory will not be used but it will not cause leak in worst meaning of this word.
    }

    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
    {
        // Now we need to substitute the indexed image data with full RGBA image data.
        unsigned char * convertedData = new unsigned char [sizeWithMipmaps * 4];
        unsigned char * pconvertedData = convertedData;
        for (unsigned int i = 0; i < sizeWithMipmaps; i++)
        {
            memcpy(pconvertedData, &palette[ imageData[i] * 4], sizeof(unsigned char) * 4 );
            pconvertedData += 4;
        }
        delete [] imageData;
        for (unsigned int i = 0; i < mipmap_offsets.size(); i++)
            mipmap_offsets[i] *= 4;
        internalFormat = GL_RGBA;
        pixelFormat = GL_RGBA;
        osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, convertedData, osg::Image::USE_NEW_DELETE, packing);
    }
    else
    {
        osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, imageData, osg::Image::USE_NEW_DELETE, packing);
    }

    if (mipmap_offsets.size()>0) osgImage->setMipmapLevels(mipmap_offsets);

    if (flipDDSRead) {
        osgImage->setOrigin(osg::Image::BOTTOM_LEFT);
        if (!isDXTC || ((s>4 && s%4==0 && t>4 && t%4==0) || s<=4)) // Flip may crash (access violation) or fail for non %4 dimensions (except for s<4). Tested with revision trunk 2013-02-22.
        {
            OSG_INFO<<"Flipping dds on load"<<std::endl;
            osgImage->flipVertical();
        }
        else
        {
            OSG_WARN << "ReadDDSFile warning: Vertical flip was skipped. Image dimensions have to be multiple of 4." << std::endl;
        }
    }

    // Return Image.
    return osgImage.release();
}

bool WriteDDSFile(const osg::Image *img, std::ostream& fout, bool autoFlipDDSWrite)
{
    bool isDXTC(false);

    // Initialize ddsd structure and its members
    DDSURFACEDESC2 ddsd;
    memset( &ddsd, 0, sizeof( ddsd ) );
    DDPIXELFORMAT  ddpf;
    memset( &ddpf, 0, sizeof( ddpf ) );
    //DDCOLORKEY     ddckCKDestOverlay;
    //DDCOLORKEY     ddckCKDestBlt;
    //DDCOLORKEY     ddckCKSrcOverlay;
    //DDCOLORKEY     ddckCKSrcBlt;
    DDSCAPS2       ddsCaps;
    memset( &ddsCaps, 0, sizeof( ddsCaps ) );

    ddsd.dwSize = sizeof(ddsd);
    ddpf.dwSize = sizeof(ddpf);

    // Default values and initialization of structures' flags
    unsigned int SD_flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    unsigned int CAPS_flags  = DDSCAPS_TEXTURE;
    unsigned int PF_flags = 0;
    unsigned int CAPS2_flags = 0;

    // Get image properties
    unsigned int dataType       = img->getDataType();
    unsigned int pixelFormat    = img->getPixelFormat();
    //unsigned int internalFormat = img->getInternalTextureFormat();
    //unsigned int components     = osg::Image::computeNumComponents(pixelFormat);
    unsigned int pixelSize      = osg::Image::computePixelSizeInBits(pixelFormat, dataType);
    unsigned int imageSize      = img->getTotalSizeInBytes();

   OSG_INFO<<"WriteDDS, dataType = 0x"<<std::hex<<dataType<<std::endl;

   // Check that theorical image size (computation taking into account DXTC blocks) is not bigger than actual image size.
    // This may happen, for instance, if some operation tuncated the data buffer non block-aligned. Example:
    //  - Read DXT1 image, size = 8x7. Actually, image data is 8x8 because it stores 4x4 blocks.
    //  - Some hypothetical operation wrongly assumes the data buffer is 8x7 and truncates the buffer. This may even lead to access violations.
    //  - Then we write the DXT1 image: last block(s) is (are) corrupt.
    // Actually what could be very nice is to handle some "lines packing" (?) in DDS reading, indicating that the image buffer has "additional lines to reach a multiple of 4".
    // Please note this can also produce false positives (ie. when data buffer is large enough, but getImageSizeInBytes() returns a smaller value). There is no way to detect this, until we fix getImageSizeInBytes() with "line packing".
    unsigned int imageSizeTheorical = ComputeImageSizeInBytes( img->s(), img->t(), img->r(), pixelFormat, dataType, img->getPacking() );
    if (imageSize < imageSizeTheorical) {
        OSG_FATAL << "Image cannot be written as DDS (Maybe a corrupt S3TC-DXTC image, with non %4 dimensions)." << std::endl;
        return false;
    }

    ddsd.dwWidth  = img->s();
    ddsd.dwHeight = img->t();
    int r = img->r();

    if(r > 1)  /* check for 3d image */
    {
        ddsd.dwDepth = r;
        SD_flags    |= DDSD_DEPTH;
        CAPS_flags  |= DDSCAPS_COMPLEX;
        CAPS2_flags |= DDSCAPS2_VOLUME;
    }

    // Determine format - set flags and ddsd, ddpf properties
    switch (pixelFormat)
    {
        //Uncompressed
    case GL_RGBA:
        {
            ddpf.dwRBitMask        = 0x000000ff;
            ddpf.dwGBitMask        = 0x0000ff00;
            ddpf.dwBBitMask        = 0x00ff0000;
            ddpf.dwRGBAlphaBitMask = 0xff000000;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_RGB);
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_BGRA:
        {
            ddpf.dwBBitMask        = 0x000000ff;
            ddpf.dwGBitMask        = 0x0000ff00;
            ddpf.dwRBitMask        = 0x00ff0000;
            ddpf.dwRGBAlphaBitMask = 0xff000000;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_RGB);
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_LUMINANCE_ALPHA:
        {
            ddpf.dwRBitMask         = 0x000000ff;
            ddpf.dwRGBAlphaBitMask  = 0x0000ff00;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_LUMINANCE);
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_RGB:
        {
            ddpf.dwRBitMask        = 0x000000ff;
            ddpf.dwGBitMask        = 0x0000ff00;
            ddpf.dwBBitMask        = 0x00ff0000;
            PF_flags |= DDPF_RGB;
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_BGR:
        {
            ddpf.dwBBitMask        = 0x000000ff;
            ddpf.dwGBitMask        = 0x0000ff00;
            ddpf.dwRBitMask        = 0x00ff0000;
            PF_flags |= DDPF_RGB;
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_LUMINANCE:
    case GL_DEPTH_COMPONENT:
        {
            ddpf.dwRBitMask         = 0x000000ff;
            PF_flags |= DDPF_LUMINANCE;
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_ALPHA:
        {
            ddpf.dwRGBAlphaBitMask  = 0x000000ff;
            PF_flags |= DDPF_ALPHA;
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;

        //Compressed
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        {
            isDXTC = true;
            ddpf.dwFourCC = FOURCC_DXT1;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_FOURCC);
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        {
            isDXTC = true;
            ddpf.dwFourCC = FOURCC_DXT3;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_FOURCC);
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        {
            isDXTC = true;
            ddpf.dwFourCC = FOURCC_DXT5;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_FOURCC);
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        {
            isDXTC = true;
            ddpf.dwFourCC = FOURCC_DXT1;
            PF_flags |= DDPF_FOURCC;  /* No alpha here */
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        {
            ddpf.dwFourCC = FOURCC_ATI1;
            PF_flags |= DDPF_FOURCC;  /* No alpha here */
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RED_RGTC1_EXT:
        {
            ddpf.dwFourCC = FOURCC_ATI1;
            PF_flags |= DDPF_FOURCC;  /* No alpha here */
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
        {
            ddpf.dwFourCC = FOURCC_ATI2;
            PF_flags |= DDPF_FOURCC;  /* No alpha here */
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        {
            ddpf.dwFourCC = FOURCC_ATI2;
            PF_flags |= DDPF_FOURCC;  /* No alpha here */
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    default:
        OSG_WARN<<"Warning:: unhandled pixel format in image, file cannot be written."<<std::endl;
        return false;
    }

    // set even more flags
    if( !img->isMipmap() ) {

       OSG_INFO<<"no mipmaps to write out."<<std::endl;

    //} else if( img->getPacking() > 1 ) {

    //   OSG_WARN<<"Warning: mipmaps not written. DDS requires packing == 1."<<std::endl;

    } else { // image contains mipmaps and has 1 byte alignment

        SD_flags   |= DDSD_MIPMAPCOUNT;
        CAPS_flags |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;

        ddsd.dwMipMapCount = img->getNumMipmapLevels();

        OSG_INFO<<"writing out with mipmaps ddsd.dwMipMapCount"<<ddsd.dwMipMapCount<<std::endl;
    }

    // Assign flags and structure members of ddsd
    ddsd.dwFlags    = SD_flags;
    ddpf.dwFlags    = PF_flags;
    ddsCaps.dwCaps  = CAPS_flags;
    ddsCaps.dwCaps2 = CAPS2_flags;

    ddsd.ddpfPixelFormat = ddpf;
    ddsd.ddsCaps = ddsCaps;

    osg::ref_ptr<const osg::Image> source;
    if (autoFlipDDSWrite && img->getOrigin() == osg::Image::BOTTOM_LEFT)
    {
        OSG_INFO<<"Flipping dds image on write"<<std::endl;

        osg::ref_ptr<osg::Image> copy( new osg::Image(*img,osg::CopyOp::DEEP_COPY_ALL) );
        const int s(copy->s());
        const int t(copy->t());
        if (!isDXTC || ((s>4 && s%4==0 && t>4 && t%4==0) || s<=4)) // Flip may crash (access violation) or fail for non %4 dimensions (except for s<4). Tested with revision trunk 2013-02-22.
        {
            copy->flipVertical();
        }
        else
        {
            OSG_WARN << "WriteDDSFile warning: Vertical flip was skipped. Image dimensions have to be multiple of 4." << std::endl;
        }
        source = copy;
    }
    else
    {
        source = img;
    }

    // Write DDS file
    fout.write("DDS ", 4); /* write FOURCC */
    fout.write(reinterpret_cast<char*>(&ddsd), sizeof(ddsd)); /* write file header */

    for(osg::Image::DataIterator itr(source.get()); itr.valid(); ++itr)
    {
        fout.write(reinterpret_cast<const char*>(itr.data()), itr.size() );
    }

    // Check for correct saving
    if ( fout.fail() )
        return false;

    // If we get that far the file was saved properly //
    return true;
}


class ReaderWriterDDS : public osgDB::ReaderWriter
{
public:

    ReaderWriterDDS()
    {
        supportsExtension("dds","DDS image format");
        supportsOption("dds_dxt1_rgb","Set the pixel format of DXT1 encoded images to be RGB variant of DXT1");
        supportsOption("dds_dxt1_rgba","Set the pixel format of DXT1 encoded images to be RGBA variant of DXT1");
        supportsOption("dds_dxt1_detect_rgba","For DXT1 encode images set the pixel format according to presence of transparent pixels");
        supportsOption("dds_flip","Flip the image about the horizontal axis");
        supportsOption("ddsNoAutoFlipWrite", "(Write option) Avoid automatically flipping the image vertically when writing, depending on the origin (Image::getOrigin()).");
    }

    virtual const char* className() const
    {
        return "DDS Image Reader/Writer";
    }

    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        return readImage(file,options);
    }

    virtual ReadResult readObject(std::istream& fin, const Options* options) const
    {
        return readImage(fin,options);
    }

    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );

        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        osgDB::ifstream stream(fileName.c_str(), std::ios::in | std::ios::binary);
        if(!stream) return ReadResult::FILE_NOT_HANDLED;
        ReadResult rr = readImage(stream, options);
        if(rr.validImage()) rr.getImage()->setFileName(file);
        return rr;
    }

    virtual ReadResult readImage(std::istream& fin, const Options* options) const
    {
        bool dds_flip(false);
        bool dds_dxt1_rgba(false);
        bool dds_dxt1_rgb(false);
        bool dds_dxt1_detect_rgba(false);
        if (options)
        {
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                if (opt == "dds_flip") dds_flip = true;
                if (opt == "dds_dxt1_rgba") dds_dxt1_rgba = true;
                if (opt == "dds_dxt1_rgb") dds_dxt1_rgb = true;
                if (opt == "dds_dxt1_detect_rgba") dds_dxt1_detect_rgba = true;
            }
        }
        osg::Image* osgImage = ReadDDSFile(fin, dds_flip);
        if (osgImage==NULL) return ReadResult::FILE_NOT_HANDLED;

        if (osgImage->getPixelFormat()==GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
            osgImage->getPixelFormat()==GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
        {
            if (dds_dxt1_rgba)
            {
                osgImage->setPixelFormat(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
                osgImage->setInternalTextureFormat(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
            }
            else if (dds_dxt1_rgb)
            {
                osgImage->setPixelFormat(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
                osgImage->setInternalTextureFormat(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
            }
            else if (dds_dxt1_detect_rgba)
            {
                // check to see if DXT1c (RGB_S3TC_DXT1) format image might actually be
                // a DXT1a format image

                // temporarily set pixel format to GL_COMPRESSED_RGBA_S3TC_DXT1_EXT so
                // that the isImageTranslucent() method assumes that RGBA is present and then
                // checks the alpha values to see if they are all 1.0.
                osgImage->setPixelFormat(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
                osgImage->setInternalTextureFormat(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
                if (!osgImage->isImageTranslucent())
                {
                    // image contains alpha's that are 1.0, so treat is as RGB
                    OSG_INFO<<"Image with PixelFormat==GL_COMPRESSED_RGB_S3TC_DXT1_EXT is opaque."<<std::endl;
                    osgImage->setPixelFormat(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
                    osgImage->setInternalTextureFormat(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
                }
                else
                {
                    // image contains alpha's that are non 1.0, so treat is as RGBA
                    OSG_INFO<<"Image with PixelFormat==GL_COMPRESSED_RGB_S3TC_DXT1_EXT has transparency, setting format to GL_COMPRESSED_RGBA_S3TC_DXT1_EXT."<<std::endl;
                }
            }
        }

        return osgImage;
    }

    virtual WriteResult writeObject(const osg::Object& object,const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
        if (!image) return WriteResult::FILE_NOT_HANDLED;

        return writeImage(*image,file,options);
    }

    virtual WriteResult writeObject(const osg::Object& object,std::ostream& fout,const Options* options) const
    {
        const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
        if (!image) return WriteResult::FILE_NOT_HANDLED;

        return writeImage(*image,fout,options);
    }


    virtual WriteResult writeImage(const osg::Image &image,const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        osgDB::ofstream fout(file.c_str(), std::ios::out | std::ios::binary);
        if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

        WriteResult res( writeImage(image,fout,options) );
        if (!res.success()) {
            // Remove file on failure
            fout.close();
            DELETEFILE(file.c_str());
        }
        return res;
    }

    virtual WriteResult writeImage(const osg::Image& image,std::ostream& fout,const Options* options) const
    {
        bool noAutoFlipDDSWrite = options && options->getOptionString().find("ddsNoAutoFlipWrite")!=std::string::npos;
        bool success = WriteDDSFile(&image, fout, !noAutoFlipDDSWrite);

        if(success)
            return WriteResult::FILE_SAVED;
        else
            return WriteResult::ERROR_IN_WRITING_FILE;
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(dds, ReaderWriterDDS)
