/**********************************************************************
*
*    FILE:            ReaderWriterVTF.cpp
*
*    DESCRIPTION:    Class for reading a Valve Texture Format (VTF) file
*                    into an osg::Image.
*
*                    Borrows heavily from the DDS plugin for OSG, as well
*                    as the Valve Source SDK
*
*    CREATED BY:     Jason Daly (jdaly@ist.ucf.edu)
*
*    HISTORY:        Created   27.10.2008
*
**********************************************************************/

#include <osg/Texture>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <iomanip>
#include <stdio.h>


enum VTFFlags
{
    VTF_FLAGS_POINTSAMPLE                          = 0x00000001,
    VTF_FLAGS_TRILINEAR                            = 0x00000002,
    VTF_FLAGS_CLAMP_S                              = 0x00000004,
    VTF_FLAGS_CLAMP_T                              = 0x00000008,
    VTF_FLAGS_ANISOTROPIC                          = 0x00000010,
    VTF_FLAGS_HINT_DXT5                            = 0x00000020,
    VTF_FLAGS_NOCOMPRESS                           = 0x00000040,
    VTF_FLAGS_NORMAL                               = 0x00000080,
    VTF_FLAGS_NOMIP                                = 0x00000100,
    VTF_FLAGS_NOLOD                                = 0x00000200,
    VTF_FLAGS_MINMIP                               = 0x00000400,
    VTF_FLAGS_PROCEDURAL                           = 0x00000800,
    VTF_FLAGS_ONEBITALPHA                          = 0x00001000,
    VTF_FLAGS_EIGHTBITALPHA                        = 0x00002000,
    VTF_FLAGS_ENVMAP                               = 0x00004000,
    VTF_FLAGS_RENDERTARGET                         = 0x00008000,
    VTF_FLAGS_DEPTHRENDERTARGET                    = 0x00010000,
    VTF_FLAGS_NODEBUGOVERRIDE                      = 0x00020000,
    VTF_FLAGS_SINGLECOPY                           = 0x00040000,
    VTF_FLAGS_ONEOVERMIPLEVELINALPHA               = 0x00080000,
    VTF_FLAGS_PREMULTCOLORBYONEOVERMIPLEVEL        = 0x00100000,
    VTF_FLAGS_NORMALTODUDV                         = 0x00200000,
    VTF_FLAGS_ALPHATESTMIPGENERATION               = 0x00400000,
    VTF_FLAGS_NODEPTHBUFFER                        = 0x00800000,
    VTF_FLAGS_NICEFILTERED                         = 0x01000000,
    VTF_FLAGS_CLAMP_U                              = 0x02000000,
    VTF_FLAGS_PRESWIZZLED                          = 0x04000000,
    VTF_FLAGS_CACHEABLE                            = 0x08000000,
    VTF_FLAGS_UNFILTERABLE_OK                      = 0x10000000,
    VTF_FLAGS_LASTFLAG                             = 0x10000000
};


enum VTFCubeMapFaceIndex
{
    VTF_CUBEMAP_FACE_RIGHT = 0,
    VTF_CUBEMAP_FACE_LEFT,
    VTF_CUBEMAP_FACE_BACK,
    VTF_CUBEMAP_FACE_FRONT,
    VTF_CUBEMAP_FACE_UP,
    VTF_CUBEMAP_FACE_DOWN,
    VTF_CUBEMAP_FACE_SPHEREMAP,
    VTF_CUBEMAP_FACE_COUNT
};


enum VTFLookDir
{
    VTF_LOOK_DOWN_X = 0,
    VTF_LOOK_DOWN_NEGX,
    VTF_LOOK_DOWN_Y = 0,
    VTF_LOOK_DOWN_NEGY,
    VTF_LOOK_DOWN_Z = 0,
    VTF_LOOK_DOWN_NEGZ
};


enum VTFImageFormat
{
    VTF_FORMAT_UNKNOWN    = -1,
    VTF_FORMAT_RGBA8888   = 0,
    VTF_FORMAT_ABGR8888,
    VTF_FORMAT_RGB888,
    VTF_FORMAT_BGR888,
    VTF_FORMAT_RGB565,
    VTF_FORMAT_I8,
    VTF_FORMAT_IA88,
    VTF_FORMAT_P8,
    VTF_FORMAT_A8,
    VTF_FORMAT_RGB888_BLUESCREEN,
    VTF_FORMAT_BGR888_BLUESCREEN,
    VTF_FORMAT_ARGB8888,
    VTF_FORMAT_BGRA8888,
    VTF_FORMAT_DXT1,
    VTF_FORMAT_DXT3,
    VTF_FORMAT_DXT5,
    VTF_FORMAT_BGRX8888,
    VTF_FORMAT_BGR565,
    VTF_FORMAT_BGRX5551,
    VTF_FORMAT_BGRA4444,
    VTF_FORMAT_DXT1_ONEBITALPHA,
    VTF_FORMAT_BGRA5551,
    VTF_FORMAT_UV88,
    VTF_FORMAT_UVWQ8888,
    VTF_FORMAT_RGBA16161616F,
    VTF_FORMAT_RGBA16161616,
    VTF_FORMAT_UVLX8888,
    VTF_FORMAT_R32F,
    VTF_FORMAT_RGB323232F,
    VTF_FORMAT_RGBA32323232F,
    VTF_NUM_IMAGE_FORMATS
};


#define VTF_FORMAT_DEFAULT   ((VTFImageFormat)-2)


struct VTFFileHeader
{
    char             magic_number[4];
    unsigned int     file_version[2];
    unsigned int     header_size;
    unsigned short   image_width;
    unsigned short   image_height;
    unsigned int     image_flags;
    unsigned short   num_frames;
    unsigned short   start_frame;

    unsigned char    padding_0[4];
    osg::Vec3f       reflectivity_value;
    unsigned char    padding_1[4];

    float            bump_scale;
    unsigned int     image_format;
    unsigned char    num_mip_levels;
    unsigned char    low_res_image_format;
    unsigned char    padding_2[3];
    unsigned char    low_res_image_width;
    unsigned char    low_res_image_height;
    unsigned short   image_depth;
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


bool ConvertImageFormat(unsigned int vtfFormat, int& internalFormat,
                        int& pixelFormat, int& dataType)
{
    bool supported;

    // Assume a supported format to start
    supported = true;

    // Decode the format
    switch (static_cast<int>(vtfFormat))
    {
        case VTF_FORMAT_DEFAULT:
            supported = false;
            break;

        case VTF_FORMAT_UNKNOWN:
            supported = false;
            break;

        case VTF_FORMAT_RGBA8888:
            internalFormat = GL_RGBA;
            pixelFormat = GL_RGBA;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_ABGR8888:
            internalFormat = GL_RGBA;
            pixelFormat = GL_ABGR_EXT;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_RGB888:
            internalFormat = GL_RGB;
            pixelFormat = GL_RGB;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_BGR888:
            internalFormat = GL_RGB;
            pixelFormat = GL_BGR;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_RGB565:
            internalFormat = GL_RGB;
            pixelFormat = GL_RGB;
            dataType = GL_UNSIGNED_SHORT_5_6_5;
            break;

        case VTF_FORMAT_I8:
            internalFormat = GL_LUMINANCE;
            pixelFormat = GL_LUMINANCE;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_IA88:
            internalFormat = GL_LUMINANCE_ALPHA;
            pixelFormat = GL_LUMINANCE_ALPHA;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_P8:
            // 8-bit paletted image, not supported
            supported = false;
            break;

        case VTF_FORMAT_A8:
            internalFormat = GL_ALPHA;
            pixelFormat = GL_ALPHA;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_RGB888_BLUESCREEN:
            // Ignore the "bluescreen" specification for now
            internalFormat = GL_RGB;
            pixelFormat = GL_RGB;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_BGR888_BLUESCREEN:
            // Ignore the "bluescreen" specification for now
            internalFormat = GL_RGB;
            pixelFormat = GL_BGR;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_ARGB8888:
            // ARGB not supported
            supported = false;
            break;

        case VTF_FORMAT_BGRA8888:
            internalFormat = GL_RGBA;
            pixelFormat = GL_BGRA;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_DXT1:
            internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            pixelFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_DXT3:
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_DXT5:
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_BGRX8888:
            internalFormat = GL_RGB;
            pixelFormat = GL_BGRA;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_BGR565:
            internalFormat = GL_RGB;
            pixelFormat = GL_BGR;
            dataType = GL_UNSIGNED_SHORT_5_6_5_REV;
            break;

        case VTF_FORMAT_BGRX5551:
            internalFormat = GL_RGB;
            pixelFormat = GL_BGRA;
            dataType = GL_UNSIGNED_SHORT_5_5_5_1;
            break;

        case VTF_FORMAT_BGRA4444:
            internalFormat = GL_RGBA;
            pixelFormat = GL_BGRA;
            dataType = GL_UNSIGNED_SHORT_4_4_4_4;
            break;

        case VTF_FORMAT_DXT1_ONEBITALPHA:
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            dataType = GL_UNSIGNED_BYTE;
            break;

        case VTF_FORMAT_BGRA5551:
            internalFormat = GL_RGBA;
            pixelFormat = GL_BGRA;
            dataType = GL_UNSIGNED_SHORT_5_5_5_1;
            break;

        case VTF_FORMAT_UV88:
            supported = false;
            break;

        case VTF_FORMAT_UVWQ8888:
            supported = false;
            break;

        case VTF_FORMAT_RGBA16161616F:
            internalFormat = GL_RGBA;
            pixelFormat = GL_RGBA;
            dataType = GL_HALF_FLOAT;
            break;

        case VTF_FORMAT_RGBA16161616:
            internalFormat = GL_RGBA;
            pixelFormat = GL_RGBA;
            dataType = GL_UNSIGNED_SHORT;
            break;

        case VTF_FORMAT_UVLX8888:
            supported = false;
            break;

        default:
            supported = false;
            break;
    }

    // Return whether or not the format is supported
    return supported;
}


osg::Image* ReadVTFFile(std::istream& _istream)
{
    VTFFileHeader     vtf_header;
    bool              supported;
    int               internalFormat;
    int               pixelFormat;
    int               dataType;
    int               s, t, r;
    unsigned int      lrSize;
    unsigned char *   imageData;
    unsigned int      size;
    int               mip;
    int               mipSize;
    int               mipOffset;

    // Validate the file with the 'VTF\0' magic number
    _istream.read(&vtf_header.magic_number[0], 4);
    if ((vtf_header.magic_number[0] != 'V') ||
        (vtf_header.magic_number[1] != 'T') ||
        (vtf_header.magic_number[2] != 'F') ||
        (vtf_header.magic_number[3] != 0))
    {
        // Not a VTF file, so bail
        OSG_WARN << "VTF file is invalid" << std::endl;
        return NULL;
    }

    // Now, read the rest of the header
    _istream.read((char *)&vtf_header.file_version[0], 8);
    _istream.read((char *)&vtf_header.header_size, 4);
    _istream.read((char *)&vtf_header.image_width, 2);
    _istream.read((char *)&vtf_header.image_height, 2);
    _istream.read((char *)&vtf_header.image_flags, 4);
    _istream.read((char *)&vtf_header.num_frames, 2);
    _istream.read((char *)&vtf_header.start_frame, 2);
    _istream.ignore(4);
    _istream.read((char *)&vtf_header.reflectivity_value, 12);
    _istream.ignore(4);
    _istream.read((char *)&vtf_header.bump_scale, 4);
    _istream.read((char *)&vtf_header.image_format, 4);
    _istream.read((char *)&vtf_header.num_mip_levels, 1);
    _istream.read((char *)&vtf_header.low_res_image_format, 4);
    _istream.read((char *)&vtf_header.low_res_image_width, 1);
    _istream.read((char *)&vtf_header.low_res_image_height, 1);

    // No depth in textures earlier than version 7.2
    if ((vtf_header.file_version[0] < 7) ||
        ((vtf_header.file_version[0] == 7) &&
         (vtf_header.file_version[1] < 2)))
    {
        // No depth in header, set it to 1
        vtf_header.image_depth = 1;
    }
    else
    {
        // Read the image depth
        _istream.read((char *)&vtf_header.image_depth, 2);
    }

    // Skip past the rest of the header's space
    std::streampos filePos = _istream.tellg();
    _istream.ignore(vtf_header.header_size - filePos);

    // Environment maps not supported
    if (vtf_header.image_flags & VTF_FLAGS_ENVMAP)
    {
        OSG_WARN << "VTF Environment maps not supported";
        OSG_WARN << std::endl;
        return NULL;
    }

    OSG_INFO << "VTF Header: (" << sizeof(VTFFileHeader);
    OSG_INFO << " bytes)" << std::endl;
    OSG_INFO << "   magic_number = ";
    OSG_INFO << vtf_header.magic_number[0];
    OSG_INFO << vtf_header.magic_number[1];
    OSG_INFO << vtf_header.magic_number[2];
    OSG_INFO << vtf_header.magic_number[3] << std:: endl;
    OSG_INFO << "   file_version = ";
    OSG_INFO << vtf_header.file_version[0] << ".";
    OSG_INFO << vtf_header.file_version[1] << std:: endl;
    OSG_INFO << "   header_size  = ";
    OSG_INFO << vtf_header.header_size << std::endl;
    OSG_INFO << "   image_width  = ";
    OSG_INFO << vtf_header.image_width << std::endl;
    OSG_INFO << "   image_height = ";
    OSG_INFO << vtf_header.image_height << std::endl;
    OSG_INFO << "   num_frames   = ";
    OSG_INFO << vtf_header.num_frames << std::endl;
    OSG_INFO << "   start_frame  = ";
    OSG_INFO << vtf_header.start_frame << std::endl;
    OSG_INFO << "   reflectivity = ";
    OSG_INFO << vtf_header.reflectivity_value.x() << ", ";
    OSG_INFO << vtf_header.reflectivity_value.y() << ", ";
    OSG_INFO << vtf_header.reflectivity_value.z() << std::endl;
    OSG_INFO << "   bump_scale   = ";
    OSG_INFO << vtf_header.bump_scale << std::endl;
    OSG_INFO << "   image_format = ";
    OSG_INFO << vtf_header.image_format << std::endl;
    OSG_INFO << "   num_mip_lvls = ";
    OSG_INFO << (int)vtf_header.num_mip_levels << std::endl;
    OSG_INFO << "   lr_image_fmt = ";
    OSG_INFO << (int)vtf_header.low_res_image_format << std::endl;
    OSG_INFO << "   lr_width     = ";
    OSG_INFO << (int)vtf_header.low_res_image_width << std::endl;
    OSG_INFO << "   lr_height    = ";
    OSG_INFO << (int)vtf_header.low_res_image_height << std::endl;
    OSG_INFO << "   image_depth  = ";
    OSG_INFO << (int)vtf_header.image_depth << std::endl;

    // Before we get to the real image, we need to skip over the "low res"
    // image that's often stored along with VTF textures, so get the
    // low-res image dimensions
    s = vtf_header.low_res_image_width;
    t = vtf_header.low_res_image_height;
    r = 1;
    OSG_INFO << "Low-res s = " << s << std::endl;
    OSG_INFO << "Low-res t = " << t << std::endl;

    // See if the low-res image is there
    lrSize = 0;
    if ((s > 0) && (t > 0))
    {
        supported = ConvertImageFormat(vtf_header.low_res_image_format,
                                       internalFormat, pixelFormat, dataType);

        // If we don't recognize the format, we can't locate the real image
        // in the file, so we have to bail
        if (!supported)
        {
            OSG_WARN << "Low-res image format is not supported";
            OSG_WARN << " (" << vtf_header.low_res_image_format;
            OSG_WARN << ")" << std::endl;
            return NULL;
        }

        // Allocate an osg::Image for the lo-res image metadata
        osg::ref_ptr<osg::Image> loResImage = new osg::Image();

        // Set the image metadata, and figure out how many bytes to read
        loResImage->setImage(s, t, r, internalFormat, pixelFormat, dataType,
                             0, osg::Image::USE_NEW_DELETE);
        lrSize = loResImage->getTotalSizeInBytes();

        // Skip over the low-res image data
        OSG_INFO << "Low-res size = " << lrSize << std::endl;
        _istream.ignore(lrSize);
    }

    // Compute the base position of the high-res image data
    // unsigned int base = vtf_header.header_size + lrSize;

    // Now, get the internal format, pixel format, and data type from the
    // full-size image format, and check whether the format is supported
    supported = ConvertImageFormat(vtf_header.image_format, internalFormat,
                                   pixelFormat, dataType);

    // Bail if the format isn't supported
    if (!supported)
    {
        OSG_WARN << "Image format is not supported (";
        OSG_WARN << vtf_header.image_format << ")";
        OSG_WARN << std::endl;
        return NULL;
    }

    // Get the dimensions of the image
    s = vtf_header.image_width;
    t = vtf_header.image_height;
    r = vtf_header.image_depth;

    // VTF allows either 0 or 1 for 2D images
    if (r == 0)
        r = 1;

    // NOTE:  VTF supports animated textures and cube maps.   Currently, we
    //        only handle a single frame of data, so multiple frames
    //        are ignored.  Same for cube maps (only one face is loaded).

    // Create the mipmap offsets vector
    osg::Image::MipmapDataType mipmaps;

    // Deal with mipmaps, if necessary
    if (vtf_header.num_mip_levels > 1)
    {
        // Set up the offsets vector
        float power2_s = logf((float)s)/logf((float)2);
        float power2_t = logf((float)t)/logf((float)2);
        mipmaps.resize((unsigned int)osg::maximum(power2_s,power2_t),0);

        // Calculate the dimensions of each mipmap
        if ((vtf_header.image_format == VTF_FORMAT_DXT1) ||
            (vtf_header.image_format == VTF_FORMAT_DXT1_ONEBITALPHA) ||
            (vtf_header.image_format == VTF_FORMAT_DXT3) ||
            (vtf_header.image_format == VTF_FORMAT_DXT5))
        {
            // Handle S3TC compressed mipmaps
            int width = vtf_header.image_width;
            int height = vtf_header.image_height;
            int blockSize;

            if ((vtf_header.image_format == VTF_FORMAT_DXT1) ||
                (vtf_header.image_format == VTF_FORMAT_DXT1_ONEBITALPHA))
               blockSize = 8;
            else
               blockSize = 16;

            int offset = 0;
            for (unsigned int k = 1;
                 (k < vtf_header.num_mip_levels) && (width || height);
                 ++k)
            {
                // Clamp dimensions to 1
                if (width == 0)
                    width = 1;
                if (height == 0)
                    height = 1;

                // Compute and store the offset into the final image data
                offset += (((width+3)/4) * ((height+3)/4) * blockSize);
                mipmaps[k-1] = offset;

                // Get the next level's dimensions
                width >>= 1;
                height >>= 1;
            }
        }
        else
        {
            // Handle uncompressed mipmaps
            int offset = 0;
            int width = vtf_header.image_width;
            int height = vtf_header.image_height;
            int depth = vtf_header.image_depth;
            for (unsigned int k = 1;
                 (k < vtf_header.num_mip_levels) && (width || height || depth);
                 ++k)
            {
                if (width == 0)
                    width = 1;
                if (height == 0)
                    height = 1;
                if (depth == 0)
                    depth = 1;

                // Compute and store the offset into the final image data
                offset += depth * height *
                    osg::Image::computeRowWidthInBytes(width, pixelFormat,
                                                       dataType, 1 );
                mipmaps[k-1] = offset;

                // Get the next level's dimensions
                width >>= 1;
                height >>= 1;
                depth >>= 1;
            }
        }
    }

    // Allocate the resulting osg::Image
    osg::ref_ptr<osg::Image> osgImage = new osg::Image();

    // Set the image meta-data, including dimensions, format, data type,
    // and mipmap levels.  Everything but the image data itself.  We'll use
    // this to compute the total image size, so we know how much data to read
    // from the file
    osgImage->setImage(s, t, r, internalFormat, pixelFormat, dataType,
                       0, osg::Image::USE_NEW_DELETE);
    if (mipmaps.size() > 0)
        osgImage->setMipmapLevels(mipmaps);

    // Compute the total image size
    size = osgImage->getTotalSizeInBytesIncludingMipmaps();
    OSG_INFO << "ReadVTFFile info : size = " << size << std::endl;
    if(size <= 0)
    {
        OSG_WARN << "ReadVTFFile warning: size <= 0" << std::endl;
        return NULL;
    }

    // Prepare to read the image data
    imageData = new unsigned char [size];
    if(!imageData)
    {
        OSG_WARN << "ReadVTFFile warning: imageData == NULL";
        OSG_WARN << std::endl;
        return NULL;
    }

    // See if we have mipmaps
    if (vtf_header.num_mip_levels > 1)
    {
        // VTF stores the mipmaps in reverse order from what OpenGL expects, so
        // we need to read them from the file and store them in order in the
        // image data array
        for (mip = vtf_header.num_mip_levels - 2; mip >= 0; mip--)
        {
            // Look up the offset for this mip level
            mipOffset = mipmaps[mip];

            // Calculate the size of the mipmap
            if (mip == vtf_header.num_mip_levels-2)
                mipSize = size - mipOffset;
            else
                mipSize = mipmaps[mip+1] - mipOffset;

            // Read the image data
            _istream.read((char*)&imageData[mipOffset], mipSize);
        }

        // We've read all of the mipmaps except the largest (the original
        // image), so do that now
        mipSize = mipmaps[0];
        _istream.read((char*)imageData, mipSize);
    }
    else
    {
        // Just read the image data
        _istream.read((char*)imageData, size);
    }

/*
    // Check if alpha information embedded in the 8-byte encoding blocks
    if (checkIfUsingOneBitAlpha)
    {
        const DXT1TexelsBlock *texelsBlock =
            reinterpret_cast<const DXT1TexelsBlock*>(imageData);

        // Only do the check on the first mipmap level
        unsigned int numBlocks = mipmaps.size()>0 ? mipmaps[0] / 8 : size / 8;

        for (int i=numBlocks; i>0; --i, ++texelsBlock)
        {
            if (texelsBlock->color_0<=texelsBlock->color_1)
            {
                // Texture is using the 1-bit alpha encoding, so we need to
                // update the assumed pixel format
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                pixelFormat    = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                break;
            }
        }
    }
*/

    // Now, set the actual image data and mipmap levels
    osgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType,
                       imageData, osg::Image::USE_NEW_DELETE);
    if (mipmaps.size()>0)  osgImage->setMipmapLevels(mipmaps);

    // Finally, return the image
    return osgImage.release();
}


bool WriteVTFFile(const osg::Image* /*img*/, std::ostream& /*fout*/)
{
    // Not supported
    return false;
}


class ReaderWriterVTF : public osgDB::ReaderWriter
{
public:
    virtual const char* className() const
    {
        return "VTF Image Reader/Writer";
    }

    virtual bool acceptsExtension(const std::string& extension) const
    {
        return osgDB::equalCaseInsensitive(extension, "vtf");
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
        osg::Image* osgImage = ReadVTFFile(fin);
        if (osgImage==NULL) return ReadResult::FILE_NOT_HANDLED;

        if (options && options->getOptionString().find("vtf_flip")!=std::string::npos)
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
        bool success = WriteVTFFile(&image, fout);

        if(success)
            return WriteResult::FILE_SAVED;
        else
            return WriteResult::ERROR_IN_WRITING_FILE;
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(vtf, ReaderWriterVTF)
