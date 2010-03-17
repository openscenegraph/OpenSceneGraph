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
// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directx9_c/Opaque_and_1_Bit_Alpha_Textures.asp
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


osg::Image* ReadDDSFile(std::istream& _istream)
{

    DDSURFACEDESC2 ddsd;

    char filecode[4];
    
    _istream.read(filecode, 4);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        return NULL;
    }
    // Get the surface desc.
    _istream.read((char*)(&ddsd), sizeof(ddsd));


    // Size of 2d images - 3d images don't set dwLinearSize
    //###[afarre_051904]
    /*unsigned int size = ddsd.dwMipMapCount > 1 ? ddsd.dwLinearSize * (ddsd.ddpfPixelFormat.dwFourCC==FOURCC_DXT1 ? 2: 4) : ddsd.dwLinearSize;

    if(size <= 0)
    {
        osg::notify(osg::WARN)<<"Warning:: dwLinearSize is not defined in dds file, image not loaded."<<std::endl;
        return NULL;
    }*/

    osg::ref_ptr<osg::Image> osgImage = new osg::Image();    
    
    //Check valid structure sizes
    if(ddsd.dwSize != 124 && ddsd.ddpfPixelFormat.dwSize != 32)
    {
        return NULL;
    }

    bool is3dImage = false;
    int depth = 1;

    // Check for volume image
    if( ddsd.dwDepth > 0 && (ddsd.dwFlags & DDSD_DEPTH))
    {
        is3dImage = true;
        depth = ddsd.dwDepth;
    }

    // Retreive image properties.
    int s = ddsd.dwWidth;
    int t = ddsd.dwHeight;
    int r = depth; 
    unsigned int dataType = GL_UNSIGNED_BYTE;
    unsigned int pixelFormat = 0;
    unsigned int internalFormat = 0;

    // Handle some esoteric formats
    if(ddsd.ddpfPixelFormat.dwFlags & DDPF_BUMPDUDV) 
    {
        osg::notify(osg::WARN) << "ReadDDSFile warning: DDPF_BUMPDUDV format is not supported" << std::endl;
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
        osg::notify(osg::WARN) << "ReadDDSFile warning: DDPF_BUMPLUMINANCE format is not supported" << std::endl;
        return NULL;
//         ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
//         // handle as RGB
//         // L6V5U5 -- 655 is not supported data type in GL
//         // X8L8V8U8 -- just as RGB
    }
    
    // Uncompressed formats will usually use DDPF_RGB to indicate an RGB format,
    // while compressed formats will use DDPF_FOURCC with a four-character code.
    
    bool usingAlpha = ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS;
    bool checkIfUsingOneBitAlpha = false;

    // Uncompressed formats.
    if(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
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
                    osg::notify(osg::INFO) << "ReadDDSFile info : format = " << f.name << std::endl;
                    internalFormat = f.internalFormat;
                    pixelFormat    = f.pixelFormat;
                    dataType       = f.dataType;
                    found = true;
                    break;
                }
                else
                {
                    osg::notify(osg::INFO) << "ReadDDSFile info : " << f.name
                                           << " format is not supported" << std::endl;
                    return NULL;                   
                }
            }            
        }

        if ( !found )
        {
            osg::notify(osg::WARN) << "ReadDDSFile warning: unhandled RGB pixel format in dds file, image not loaded" << std::endl;
            osg::notify(osg::INFO) << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwRGBBitCount     = "
                                   << ddsd.ddpfPixelFormat.dwRGBBitCount << std::endl;
            osg::notify(osg::INFO) << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwRBitMask        = 0x"
                                   << std::hex << std::setw(8) << std::setfill('0') 
                                   << ddsd.ddpfPixelFormat.dwRBitMask << std::endl;
            osg::notify(osg::INFO) << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwGBitMask        = 0x"
                                   << std::hex << std::setw(8) << std::setfill('0') 
                                   << ddsd.ddpfPixelFormat.dwGBitMask << std::endl;
            osg::notify(osg::INFO) << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwBBitMask        = 0x"
                                   << std::hex << std::setw(8) << std::setfill('0') 
                                   << ddsd.ddpfPixelFormat.dwBBitMask << std::endl;
            osg::notify(osg::INFO) << "ReadDDSFile info : ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x"
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
                osg::notify(osg::INFO) << "ReadDDSFile info : format = L4A4" << std::endl;
                pixelFormat = GL_LUMINANCE4_ALPHA4; // invalid enumerant?
            }
            else if ( usingAlpha && ddsd.ddpfPixelFormat.dwLuminanceBitDepth == 32 )
            {
                osg::notify(osg::INFO) << "ReadDDSFile info : format = L16A16" << std::endl;
                dataType = GL_UNSIGNED_SHORT;
            }
            else if ( !usingAlpha && ddsd.ddpfPixelFormat.dwLuminanceBitDepth == 16 )
            {
                osg::notify(osg::INFO) << "ReadDDSFile info : format = L16" << std::endl;
                dataType = GL_UNSIGNED_SHORT;
            }
            else if ( usingAlpha )
            {
                osg::notify(osg::INFO) << "ReadDDSFile info : format = L8A8" << std::endl;
            }
            else
            {
                osg::notify(osg::INFO) << "ReadDDSFile info : format = L8" << std::endl;
            }
//             else if ( ddsd.ddpfPixelFormat.dwLuminanceBitDepth == (usingAlpha ? 64 : 32) )
//             {
//                 dataType = GL_UNSIGNED_INT;
//             }
    }
    else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHA)
    {
            osg::notify(osg::INFO) << "ReadDDSFile info : format = ALPHA" << std::endl;
            internalFormat = GL_ALPHA;
            pixelFormat    = GL_ALPHA;              
    }
    // Compressed formats
    else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
    {
        // TODO: Image::isImageTranslucent() doesn't work with S3TC compressed files
        switch(ddsd.ddpfPixelFormat.dwFourCC)
        {
        case FOURCC_DXT1:
            osg::notify(osg::INFO) << "ReadDDSFile info : format = DXT1" << std::endl;
            if (usingAlpha)
            {
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            }
            else
            {
                internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                pixelFormat    = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                checkIfUsingOneBitAlpha = true;
            }
            break;
        case FOURCC_DXT3:
            osg::notify(osg::INFO) << "ReadDDSFile info : format = DXT3" << std::endl;
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            break;
        case FOURCC_DXT5:
            osg::notify(osg::INFO) << "ReadDDSFile info : format = DXT5" << std::endl;
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        case 0x00000024: // A16B16G16R16
            osg::notify(osg::INFO) << "ReadDDSFile info : format = A16B16G16R16" << std::endl;
            internalFormat = GL_RGBA;
            pixelFormat    = GL_RGBA;
            dataType       = GL_UNSIGNED_SHORT;
            break;
        case 0x00000071: // A16B16G16R16F
            osg::notify(osg::INFO) << "ReadDDSFile info : format = A16B16G16R16F" << std::endl;
            internalFormat = GL_RGBA; // why no transparency?
            pixelFormat    = GL_RGBA;
            dataType       = GL_HALF_FLOAT_NV; 
            break;
        case 0x0000006E: // Q16W16V16U16
            osg::notify(osg::INFO) << "ReadDDSFile info : format = Q16W16V16U16" << std::endl;
            internalFormat = GL_RGBA;
            pixelFormat    = GL_RGBA;
            dataType       = GL_UNSIGNED_SHORT;
            break;
        case 0x00000070: // G16R16F
            osg::notify(osg::INFO) << "ReadDDSFile info : G16R16F format is not supported"
                                   << std::endl;
            return NULL;
//             internalFormat = GL_RGB;
//             pixelFormat    = must be GL_RED and GL_GREEN
//             dataType       = GL_HALF_FLOAT_NV;
            break;
        case 0x00000073: // G32R32F
            osg::notify(osg::INFO) << "ReadDDSFile info : G32R32F format is not supported"
                                   << std::endl;
            return NULL;
//             internalFormat = GL_RGB;
//             pixelFormat    = must be GL_RED and GL_GREEN
//             dataType       = GL_FLOAT;
            break;
        case 0x00000072: // R32F
            osg::notify(osg::INFO) << "ReadDDSFile info : format = R32F" << std::endl;
            internalFormat = GL_RGB;
            pixelFormat    = GL_RED;
            dataType       = GL_FLOAT;
            break;
        case 0x0000006F: // R16F
            osg::notify(osg::INFO) << "ReadDDSFile info : format = R16F" << std::endl;
            internalFormat = GL_RGB;
            pixelFormat    = GL_RED;
            dataType       = GL_HALF_FLOAT_NV;
            break;
        case 0x00000074: // A32B32G32R32F
            osg::notify(osg::INFO) << "ReadDDSFile info : format = A32B32G32R32F" << std::endl;
            internalFormat = GL_RGBA;
            pixelFormat    = GL_RGBA;
            dataType       = GL_FLOAT;
            break;
        case 0x00000075: // CxV8U8
            osg::notify(osg::INFO) << "ReadDDSFile info : CxV8U8 format is not supported" << std::endl;
            return NULL;

        case MAKEFOURCC( 'U', 'Y', 'V', 'Y' ): // not supported in OSG
        case MAKEFOURCC( 'U', 'Y', 'V', '2' ): // not supported in OSG
        case MAKEFOURCC( 'R', 'G', 'B', 'G' ): // R8G8_B8G8 -- what is it?
        case MAKEFOURCC( 'G', 'R', 'G', 'B' ): // G8R8_G8B8 -- what is it?
            //break;

        default:
            osg::notify(osg::WARN) << "ReadDDSFile warning: unhandled FOURCC pixel format ("
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
    else 
    {
        osg::notify(osg::WARN) << "ReadDDSFile warning: unhandled pixel format (ddsd.ddpfPixelFormat.dwFlags"
                               << " = 0x" << std::hex << std::setw(8) << std::setfill('0')
                               << ddsd.ddpfPixelFormat.dwFlags << std::dec
                               << ") in dds file, image not loaded."<<std::endl;
        return NULL;
    }

    //###[afarre_051904]
    /*if (is3dImage)
        size = osg::Image::computeNumComponents(pixelFormat) * ddsd.dwWidth * ddsd.dwHeight * depth;


    //delayed allocation og image data after all checks
    unsigned char* imageData = new unsigned char [size];
    if(!imageData)
    {
        return NULL;
    }

    // Read image data
    _istream.read((char*)imageData, size);

    
    // NOTE: We need to set the image data before setting the mipmap data, this
    // is because the setImage method clears the _mipmapdata vector in osg::Image.
    // Set image data and properties.
    osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, imageData, osg::Image::USE_NEW_DELETE);
    */

    // Now set mipmap data (offsets into image raw data)
    //###[afarre_051904]
    osg::Image::MipmapDataType mipmaps; 

    // Take care of mipmaps if any.
    if (ddsd.dwMipMapCount>1)
    {
        // Now set mipmap data (offsets into image raw data).
        //###[afarre_051904]osg::Image::MipmapDataType mipmaps;

        //This is to complete mipmap sequence until level Nx1

        //debugging messages        
        float power2_s = logf((float)s)/logf((float)2);
        float power2_t = logf((float)t)/logf((float)2);

        osg::notify(osg::INFO) << "ReadDDSFile INFO : ddsd.dwMipMapCount = "<<ddsd.dwMipMapCount<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile INFO : s = "<<s<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile INFO : t = "<<t<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile INFO : power2_s="<<power2_s<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile INFO : power2_t="<<power2_t<<std::endl;

        mipmaps.resize((unsigned int)osg::maximum(power2_s,power2_t),0);

        // Handle S3TC compressed mipmaps.
        if( (ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
            &&
            (ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT1 ||
             ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT3 ||
             ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT5))
        {
            int width = ddsd.dwWidth;
            int height = ddsd.dwHeight;
            int blockSize = (ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT1) ? 8 : 16;
            int offset = 0;
            for (unsigned int k = 1; k < ddsd.dwMipMapCount && (width || height); ++k)
            {
                if (width == 0)
                    width = 1;
                if (height == 0)
                    height = 1;
                offset += (((width+3)/4) * ((height+3)/4) * blockSize);
                mipmaps[k-1] = offset;
                width >>= 1;
                height >>= 1;
            }
            //###[afarre_051904] osgImage->setMipmapData(mipmaps);
        }
        // Handle uncompressed mipmaps
        else
        {
            int offset = 0;
            int width = ddsd.dwWidth;
            int height = ddsd.dwHeight;
            for (unsigned int k = 1; k < ddsd.dwMipMapCount && (width || height); ++k)
            {
                if (width == 0)
                    width = 1;
                if (height == 0)
                    height = 1;
                offset += height *
                    osg::Image::computeRowWidthInBytes( width, pixelFormat, dataType, 1 );
                mipmaps[k-1] = offset;
                width >>= 1;
                height >>= 1;
            }
            //###[afarre_051904] osgImage->setMipmapData(mipmaps);
        }
    }

    osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, 0, osg::Image::USE_NEW_DELETE);
    if (mipmaps.size()>0)  osgImage->setMipmapLevels(mipmaps);
    unsigned int size = osgImage->getTotalSizeInBytesIncludingMipmaps();

    osg::notify(osg::INFO) << "ReadDDSFile INFO : size = " << size << std::endl;
    
    if(size <= 0)
    {
        osg::notify(osg::WARN) << "ReadDDSFile warning: size <= 0" << std::endl;
        return NULL;
    }

    unsigned char* imageData = new unsigned char [size];
    if(!imageData)
    {
        osg::notify(osg::WARN) << "ReadDDSFile warning: imageData == NULL" << std::endl;
        return NULL;
    }

    // Read image data
    _istream.read((char*)imageData, size);

    // Check if alpha information embedded in the 8-byte encoding blocks
    if (checkIfUsingOneBitAlpha)
    {
        const DXT1TexelsBlock *texelsBlock = reinterpret_cast<const DXT1TexelsBlock*>(imageData);

        // Only do the check on the first mipmap level
        unsigned int numBlocks = mipmaps.size()>0 ? mipmaps[0] / 8 : size / 8;

        for (int i=numBlocks; i>0; --i, ++texelsBlock)
        {
            if (texelsBlock->color_0<=texelsBlock->color_1)
            {
                // Texture is using the 1-bit alpha encoding, so we need to update the assumed pixel format
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                break;
            }
        }
    }

    osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, imageData, osg::Image::USE_NEW_DELETE);
    if (mipmaps.size()>0)  osgImage->setMipmapLevels(mipmaps);
         
    // Return Image.
    return osgImage.release();
}


/*
osg::Image::MipmapDataType mipmaps;
osgImage->setMipmapData(mipmaps);
osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, 0, osg::Image::USE_NEW_DELETE);
printf("INVENTO===> gtsibim:%d  grsib:%d   mi_size:%d   lPitch%d\n", 
    osgImage->getTotalSizeInBytesIncludingMipmaps(), 
    osgImage->getRowSizeInBytes(), size, ddsd.lPitch);
printf("CORRECTO**> gtsibim:%d  grsib:%d   mi_size:%d   lPitch%d\n", 
    osgImage->getTotalSizeInBytesIncludingMipmaps(), 
    osgImage->getRowSizeInBytes(), size, ddsd.lPitch);

 */


bool WriteDDSFile(const osg::Image *img, std::ostream& fout)
{

    // Initialize ddsd structure and its members 
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT  ddpf;
    //DDCOLORKEY     ddckCKDestOverlay;
    //DDCOLORKEY     ddckCKDestBlt;
    //DDCOLORKEY     ddckCKSrcOverlay;
    //DDCOLORKEY     ddckCKSrcBlt;
    DDSCAPS2       ddsCaps;

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
    unsigned int imageSize      = img->getImageSizeInBytes();
    bool is3dImage = false;

    ddsd.dwWidth  = img->s();
    ddsd.dwHeight = img->t();
    int r = img->r();

    if(r > 1)  /* check for 3d image */
    {
        is3dImage = true;
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
    case GL_LUMINANCE:
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
            ddpf.dwFourCC = FOURCC_DXT1;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_FOURCC);
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        {
            ddpf.dwFourCC = FOURCC_DXT3;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_FOURCC);
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        {
            ddpf.dwFourCC = FOURCC_DXT5;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_FOURCC);
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        {
            ddpf.dwFourCC = FOURCC_DXT1;
            PF_flags |= DDPF_FOURCC;  /* No alpha here */
            ddsd.dwLinearSize = imageSize;
            SD_flags |= DDSD_LINEARSIZE;
        }
        break;
    default:
        osg::notify(osg::WARN)<<"Warning:: unhandled pixel format in image, file cannot be written."<<std::endl;
        return false;
    }

   
    // set even more flags
    if(img->isMipmap() && !is3dImage)
    {
        SD_flags   |= DDSD_MIPMAPCOUNT;
        CAPS_flags |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
        ddsd.dwMipMapCount = img->getNumMipmapLevels();
        
        osg::notify(osg::INFO)<<"writing out with mipmaps ddsd.dwMipMapCount"<<ddsd.dwMipMapCount<<std::endl;
    }
    else
    {
        osg::notify(osg::INFO)<<"no mipmaps to write out."<<std::endl;
    }


    // Assign flags and structure members of ddsd
    ddsd.dwFlags    = SD_flags;
    ddpf.dwFlags    = PF_flags;
    ddsCaps.dwCaps  = CAPS_flags;
    ddsCaps.dwCaps2 = CAPS2_flags;

    ddsd.ddpfPixelFormat = ddpf;
    ddsd.ddsCaps = ddsCaps;


    // Write DDS file
    fout.write("DDS ", 4); /* write FOURCC */
    fout.write(reinterpret_cast<char*>(&ddsd), sizeof(ddsd)); /* write file header */

    //    int isize = img->getTotalSizeInBytesIncludingMipmaps();
    if(!is3dImage)
    {
        fout.write(reinterpret_cast<const char*>(img->data()), img->getTotalSizeInBytesIncludingMipmaps());
    }
    else  /* 3d image */
    {
        for(int i = 0; i < r; ++i)
        {
            fout.write(reinterpret_cast<const char*>(img->data(0, 0, i)), imageSize);
        }
    }

    // Check for correct saving
    if (fout.fail())
    {
        return false;
    }

    // If we get that far the file was saved properly //
    return true; 
}


class ReaderWriterDDS : public osgDB::ReaderWriter
{
public:

    ReaderWriterDDS()
    {
        supportsExtension("dds","DDS image format");
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
        osg::Image* osgImage = ReadDDSFile(fin);
        if (osgImage==NULL) return ReadResult::FILE_NOT_HANDLED;
        
        if (options && options->getOptionString().find("dds_flip")!=std::string::npos)
        {
            osgImage->flipVertical();
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

        return writeImage(image,fout,options);
    }

    virtual WriteResult writeImage(const osg::Image& image,std::ostream& fout,const Options*) const
    {
        bool success = WriteDDSFile(&image, fout);

        if(success)
            return WriteResult::FILE_SAVED;
        else
            return WriteResult::ERROR_IN_WRITING_FILE;
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(dds, ReaderWriterDDS)
