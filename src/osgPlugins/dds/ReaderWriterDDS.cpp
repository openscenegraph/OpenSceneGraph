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
*    CREATED BY:        Rune Schmidt Jensen, rsj@uni-dk
*
*    HISTORY:        Created   31.03.2003
*             Modified  13.05.2004
*                by George Tarantilis, gtaranti@nps.navy.mil
*
**********************************************************************/

#include <osg/Texture>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <stdio.h>


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

    // Uncompressed formats will usually use DDPF_RGB to indicate an RGB format,
    // while compressed formats will use DDPF_FOURCC with a four-character code.
    
    bool usingAlpha = ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS;

    // Uncompressed formats.
    if(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
    {
        switch(ddsd.ddpfPixelFormat.dwRGBBitCount)
        {
        case 32:
            internalFormat = 4;
            pixelFormat = GL_RGBA;
            break;
        case 24:
            internalFormat = 3;
            pixelFormat = GL_RGB;
            break;
        case 16:
        default:
            osg::notify(osg::WARN)<<"Warning:: unhandled pixel format in dds file, image not loaded"<<std::endl;
            return NULL;
        }
    }
    else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
    {
            internalFormat = usingAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;
            pixelFormat    = usingAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;              
    }
   else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHA)
    {
            internalFormat = GL_ALPHA;
            pixelFormat    = GL_ALPHA;              
    }
    // Compressed formats
    else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
    {
        switch(ddsd.ddpfPixelFormat.dwFourCC)
        {
        case FOURCC_DXT1:
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
            osg::notify(osg::WARN)<<"Warning:: unhandled pixel format in dds file, image not loaded."<<std::endl;
            return NULL; 
        }
    }
    else 
    {
        osg::notify(osg::WARN)<<"Warning:: unhandled pixel format in dds file, image not loaded."<<std::endl;
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

        osg::notify(osg::INFO) << "ReadDDSFile info : ddsd.dwMipMapCount = "<<ddsd.dwMipMapCount<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile info : s = "<<s<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile info : t = "<<t<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile info : power2_s="<<power2_s<<std::endl;
        osg::notify(osg::INFO) << "ReadDDSFile info : power2_t="<<power2_t<<std::endl;

        mipmaps.resize((unsigned int)osg::maximum(power2_s,power2_t),0);

        // Handle compressed mipmaps.
        if(ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
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
        if(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
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
                offset += (width*height*(ddsd.ddpfPixelFormat.dwRGBBitCount/8));
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

    if(size <= 0)
    {
        return NULL;
    }

    unsigned char* imageData = new unsigned char [size];
    if(!imageData)
    {
        return NULL;
    }

    // Read image data
    _istream.read((char*)imageData, size);

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
            ddpf.dwRBitMask        = 0x00ff0000;  
            ddpf.dwGBitMask        = 0x0000ff00;
            ddpf.dwBBitMask        = 0x000000ff;
            ddpf.dwRGBAlphaBitMask = 0xff000000;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_RGB);
            ddpf.dwRGBBitCount = pixelSize; 
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_BGRA:
        {
            ddpf.dwBBitMask        = 0x00ff0000;  
            ddpf.dwGBitMask        = 0x0000ff00;
            ddpf.dwRBitMask        = 0x000000ff;
            ddpf.dwRGBAlphaBitMask = 0xff000000;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_RGB);
            ddpf.dwRGBBitCount = pixelSize; 
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_LUMINANCE_ALPHA:
        {
            ddpf.dwRBitMask         = 0x00ff0000;
            ddpf.dwRGBAlphaBitMask  = 0x000000ff;
            PF_flags |= (DDPF_ALPHAPIXELS | DDPF_LUMINANCE);  
            ddpf.dwRGBBitCount = pixelSize; 
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_RGB:
        {
            ddpf.dwRBitMask         = 0x00ff0000;
            ddpf.dwGBitMask         = 0x0000ff00;
            ddpf.dwBBitMask         = 0x000000ff;            
            PF_flags |= DDPF_RGB;
            ddpf.dwRGBBitCount = pixelSize;
            ddsd.lPitch = img->getRowSizeInBytes();
            SD_flags |= DDSD_PITCH;
        }
        break;
    case GL_LUMINANCE:
        {
            ddpf.dwRBitMask         = 0x00ff0000;
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
    virtual const char* className() const
    { 
        return "DDS Image Reader/Writer"; 
    }

    virtual bool acceptsExtension(const std::string& extension) const
    { 
        return osgDB::equalCaseInsensitive(extension,"dds"); 
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
        
        std::ifstream stream(fileName.c_str(), std::ios::in | std::ios::binary);
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

        std::ofstream fout(file.c_str(), std::ios::out | std::ios::binary);
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
osgDB::RegisterReaderWriterProxy<ReaderWriterDDS> g_readerWriter_DDS_Proxy;
