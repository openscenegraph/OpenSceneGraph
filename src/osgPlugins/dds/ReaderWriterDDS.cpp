/**********************************************************************
 *
 *	FILE:			ReaderWriterdds.cpp
 *
 *	DESCRIPTION:	Class for reading a DDS file into an osg::Image.
 *
 *					Example on reading a DDS file code can be found at:
 *					http://developer.nvidia.com/docs/IO/1327/ATT/
 *					ARB_texture_compression.pdf
 *					Author: Sébastien Dominé, NVIDIA Corporation
 *
 *	CREATED BY:		Rune Schmidt Jensen, rsj@uni-dk
 *
 *	HISTORY:		Created 31.03.2003
 *
  **********************************************************************/

#include <osg/Texture>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

#include <stdio.h>

// NOTICE ON WIN32:
// typedef DWORD unsigned long;
// sizeof(DWORD) = 4
typedef struct _DDCOLORKEY
{
    unsigned long	dwColorSpaceLowValue;
	unsigned long	dwColorSpaceHighValue;
} DDCOLORKEY;


typedef struct _DDPIXELFORMAT
{
    unsigned long	dwSize;
    unsigned long	dwFlags;
    unsigned long	dwFourCC;
    union
    {
	unsigned long	dwRGBBitCount;
	unsigned long	dwYUVBitCount;
	unsigned long	dwZBufferBitDepth;
	unsigned long	dwAlphaBitDepth;
    };
    union
    {
	unsigned long	dwRBitMask;
	unsigned long	dwYBitMask;
    };
    union
    {
	unsigned long	dwGBitMask;
	unsigned long	dwUBitMask;
    };
    union
    {
	unsigned long	dwBBitMask;
	unsigned long	dwVBitMask;
    };
    union
    {
	unsigned long	dwRGBAlphaBitMask;
	unsigned long	dwYUVAlphaBitMask;
	unsigned long	dwRGBZBitMask;
	unsigned long	dwYUVZBitMask;
    };
} DDPIXELFORMAT;

typedef struct _DDSCAPS2
{
    unsigned long       dwCaps;
    unsigned long       dwCaps2;
    unsigned long       dwCaps3;
    union
    {
        unsigned long       dwCaps4;
        unsigned long       dwVolumeDepth;
    };
} DDSCAPS2;

typedef struct _DDSURFACEDESC2 {
    unsigned long         dwSize;
    unsigned long         dwFlags;
    unsigned long         dwHeight;
    unsigned long         dwWidth;
    union
    {
        long      lPitch;
        unsigned long      dwLinearSize;
    };
    unsigned long         dwBackBufferCount;
    union
    {
        unsigned long     dwMipMapCount;
        unsigned long     dwRefreshRate;
    };
    unsigned long         dwAlphaBitDepth;
    unsigned long         dwReserved;
    unsigned long*        lpSurface;
    DDCOLORKEY    ddckCKDestOverlay;
    DDCOLORKEY    ddckCKDestBlt;
    DDCOLORKEY    ddckCKSrcOverlay;
    DDCOLORKEY    ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2      ddsCaps;
    unsigned long         dwTextureStage;
} DDSURFACEDESC2; 

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((unsigned long)(char)(ch0) | ((unsigned long)(char)(ch1) << 8) |   \
                ((unsigned long)(char)(ch2) << 16) | ((unsigned long)(char)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

/*
 * FOURCC codes for DX compressed-texture pixel formats
 */
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

osg::Image* ReadDDSFile(const char *filename){
	osg::Image* osgImage = new osg::Image();	

	DDSURFACEDESC2 ddsd;

	char filecode[4];
	FILE *fp;

	// Open file.
	fp = fopen(filename, "rb");
	if (fp == NULL)
		return NULL;

	// Verify that this is a DDS file.
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fclose(fp);
		return NULL;
	}

	// Get the surface desc.
	fread(&ddsd, sizeof(ddsd), 1, fp);

	// Read image data.
	unsigned int size = ddsd.dwMipMapCount > 1 ? ddsd.dwLinearSize * 2 : ddsd.dwLinearSize;
	unsigned char* imageData = (unsigned char*) malloc(size * sizeof(unsigned char));
	fread(imageData, 1, size, fp);
	// Close the file.
	fclose(fp);

	// Retreive image properties.
	int s = ddsd.dwWidth;
	int t = ddsd.dwHeight;
    int r = ddsd.dwMipMapCount>1?ddsd.dwMipMapCount:0;
	unsigned int dataType = GL_UNSIGNED_BYTE;
	unsigned int pixelFormat;
	unsigned int internalFormat;
	switch(ddsd.ddpfPixelFormat.dwFourCC){
		case FOURCC_DXT1:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case FOURCC_DXT3:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case FOURCC_DXT5:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			free(imageData);
			return NULL;
	}

	// Set image data and properties.
	osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, imageData, osg::Image::USE_NEW_DELETE);

	// Return Image.
	return osgImage;
}

class ReaderWriterDDS : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { 
			return "DDS Image Reader"; 
		}

        virtual bool acceptsExtension(const std::string& extension) { 
			return osgDB::equalCaseInsensitive(extension,"dds"); 
		}

        virtual ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*) {
			osg::Image* osgImage = ReadDDSFile(fileName.c_str());
            if (osgImage==NULL) return ReadResult::FILE_NOT_HANDLED;
            return osgImage;

        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterDDS> g_readerWriter_DDS_Proxy;
