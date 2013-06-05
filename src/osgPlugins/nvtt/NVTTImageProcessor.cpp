
#include <osg/Texture>
#include <osgDB/Registry>

#include <nvtt/nvtt.h>
#include <string.h>

class NVTTProcessor : public osgDB::ImageProcessor
{
public:
    virtual void compress(osg::Image& image, osg::Texture::InternalFormatMode compressedFormat, bool generateMipMap, bool resizeToPowerOfTwo, CompressionMethod method, CompressionQuality quality);
    virtual void generateMipMap(osg::Image& image, bool resizeToPowerOfTwo, CompressionMethod method);

protected:

    void process( osg::Image& texture, nvtt::Format format, bool generateMipMap, bool resizeToPowerOfTwo, CompressionMethod method, CompressionQuality quality);

    struct VPBErrorHandler : public nvtt::ErrorHandler
    {
        virtual void error(nvtt::Error e);
    };

    struct OSGImageOutputHandler : public nvtt::OutputHandler
    {
        typedef std::vector<unsigned char> MipMapData;

        std::vector<MipMapData*> _mipmaps;
        int _width;
        int _height;
        int _currentMipLevel;
        int _currentNumberOfWritenBytes;
        nvtt::Format _format;
        bool _discardAlpha;

        OSGImageOutputHandler(nvtt::Format format, bool discardAlpha);
        virtual ~OSGImageOutputHandler();

        // create the osg image from the given format
        bool assignImage(osg::Image& image);

        /// Indicate the start of a new compressed image that's part of the final texture.
        virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel);

        virtual void endImage() {}

        /// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
        virtual bool writeData(const void * data, int size);
    };

    // Convert RGBA to BGRA : nvtt only accepts BGRA pixel format
    void convertRGBAToBGRA( std::vector<unsigned char>& outputData, const osg::Image& image );

    // Convert RGB to BGRA : nvtt only accepts BGRA pixel format
    void convertRGBToBGRA( std::vector<unsigned char>& outputData, const osg::Image& image );

};

/// Error handler.
void NVTTProcessor::VPBErrorHandler::error(nvtt::Error e)
{
    switch (e)
    {
    case nvtt::Error_Unknown:
        OSG_WARN<<" NVTT : unknown error"<<std::endl;
        break;
    case nvtt::Error_InvalidInput:
        OSG_WARN<<" NVTT : invalid input"<<std::endl;
        break;
    case nvtt::Error_UnsupportedFeature:
        OSG_WARN<<" NVTT : unsupported feature"<<std::endl;
        break;
    case nvtt::Error_CudaError:
        OSG_WARN<<" NVTT : cuda error"<<std::endl;
        break;
        case nvtt::Error_FileOpen:
        OSG_WARN<<" NVTT : file open error"<<std::endl;
        break;
        case nvtt::Error_FileWrite:
        OSG_WARN<<" NVTT : file write error"<<std::endl;
        break;
    default: break;
    }
}

/// Output handler.
NVTTProcessor::OSGImageOutputHandler::OSGImageOutputHandler(nvtt::Format format, bool discardAlpha)
    : _format(format), _discardAlpha(discardAlpha)
{
}

NVTTProcessor::OSGImageOutputHandler::~OSGImageOutputHandler()
{
    for (unsigned int n=0; n<_mipmaps.size(); n++)
    {
        delete _mipmaps[n];
    }
    _mipmaps.clear();
}

// create the osg image from the given format
bool NVTTProcessor::OSGImageOutputHandler::assignImage(osg::Image& image)
{
    // convert nvtt format to OpenGL pixel format
    GLint pixelFormat;
    switch (_format)
    {
    case nvtt::Format_RGBA:
        pixelFormat = _discardAlpha ? GL_RGB : GL_RGBA;
        break;
    case nvtt::Format_DXT1:
        pixelFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        break;
    case nvtt::Format_DXT1a:
        pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case nvtt::Format_DXT3:
        pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case nvtt::Format_DXT5:
        pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        OSG_WARN<<" Invalid or not supported format"<<std::endl;
        return false;
    }

    // Compute the total size and the mipmap offsets
    osg::Image::MipmapDataType mipmapOffsets(_mipmaps.size()-1);
    unsigned int totalSize = _mipmaps[0]->size();
    for (unsigned int n=1; n<_mipmaps.size(); n++)
    {
        mipmapOffsets[n-1] = totalSize;
        totalSize += _mipmaps[n]->size();
    }

    // Allocate data and copy it
    unsigned char* data = new unsigned char[ totalSize ];
    unsigned char* ptr = data;
    for (unsigned int n=0; n<_mipmaps.size(); n++)
    {
        memcpy( ptr, &(*_mipmaps[n])[0], _mipmaps[n]->size() );
        ptr += _mipmaps[n]->size();
    }

    image.setImage(_width,_height,1,pixelFormat,pixelFormat,GL_UNSIGNED_BYTE,data,osg::Image::USE_NEW_DELETE);
    image.setMipmapLevels(mipmapOffsets);

    return true;
}

/// Indicate the start of a new compressed image that's part of the final texture.
void NVTTProcessor::OSGImageOutputHandler::beginImage(int size, int width, int height, int depth, int face, int miplevel)
{
    // store the new width/height of the texture
    if (miplevel == 0)
    {
        _width = width;
        _height = height;
    }
    // prepare to receive mipmap data
    if (miplevel >= static_cast<int>(_mipmaps.size()))
    {
        _mipmaps.resize(miplevel+1);
    }
    _mipmaps[miplevel] = new MipMapData(size);
    _currentMipLevel = miplevel;
    _currentNumberOfWritenBytes = 0;
}

/// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
bool NVTTProcessor::OSGImageOutputHandler::writeData(const void * data, int size)
{
    // Copy mipmap data
    std::vector<unsigned char>& dstData = *_mipmaps[_currentMipLevel];
    memcpy( &dstData[_currentNumberOfWritenBytes], data, size );
    _currentNumberOfWritenBytes += size;
    return true;
}

// Convert RGBA to BGRA : nvtt only accepts BGRA pixel format
void NVTTProcessor::convertRGBAToBGRA( std::vector<unsigned char>& outputData, const osg::Image& image )
{
    unsigned int n=0;
    for(int row=0; row<image.t(); ++row)
    {
        const unsigned char* data = image.data(0,row);
        for(int column=0; column<image.s(); ++column)
        {
            outputData[n] = data[column*4+2];
            outputData[n+1] = data[column*4+1];
            outputData[n+2] = data[column*4];
            outputData[n+3] = data[column*4+3];
            n+=4;
        }
    }
}

// Convert RGB to BGRA : nvtt only accepts BGRA pixel format
void NVTTProcessor::convertRGBToBGRA( std::vector<unsigned char>& outputData, const osg::Image& image )
{
    unsigned int n=0;
    for(int row=0; row<image.t(); ++row)
    {
        const unsigned char* data = image.data(0,row);
        for(int column=0; column<image.s(); ++column)
        {
            outputData[n] = data[column*3+2];
            outputData[n+1] = data[column*3+1];
            outputData[n+2] = data[column*3];
            outputData[n+3] = 255;
            n+=4;
        }
    }
}

// Main interface with NVTT
void NVTTProcessor::process( osg::Image& image, nvtt::Format format, bool generateMipMap, bool resizeToPowerOfTwo, CompressionMethod method, CompressionQuality quality)
{
    // Fill input options
    nvtt::InputOptions inputOptions;
    inputOptions.setTextureLayout(nvtt::TextureType_2D, image.s(), image.t() );
    inputOptions.setNormalMap(false);
    inputOptions.setConvertToNormalMap(false);
    inputOptions.setGamma(2.2f, 2.2f);
    inputOptions.setNormalizeMipmaps(false);
    inputOptions.setWrapMode(nvtt::WrapMode_Clamp);
    if (resizeToPowerOfTwo)
    {
        inputOptions.setRoundMode(nvtt::RoundMode_ToNearestPowerOfTwo);
    }
    inputOptions.setMipmapGeneration(generateMipMap);

    if (image.getPixelFormat() == GL_RGBA)
    {
        inputOptions.setAlphaMode( nvtt::AlphaMode_Transparency );
    }
    else
    {
        inputOptions.setAlphaMode( nvtt::AlphaMode_None );
    }
    std::vector<unsigned char> imageData( image.s() * image.t() * 4 );
    if (image.getPixelFormat() == GL_RGB)
    {
        convertRGBToBGRA( imageData, image );
    }
    else
    {
        convertRGBAToBGRA( imageData, image );
    }
    inputOptions.setMipmapData(&imageData[0],image.s(),image.t());

    // Fill compression options
    nvtt::CompressionOptions compressionOptions;
    switch(quality)
    {
      case FASTEST:
        compressionOptions.setQuality( nvtt::Quality_Fastest );
        break;
      case NORMAL:
        compressionOptions.setQuality( nvtt::Quality_Normal );
        break;
      case PRODUCTION:
        compressionOptions.setQuality( nvtt::Quality_Production);
        break;
      case HIGHEST:
        compressionOptions.setQuality( nvtt::Quality_Highest);
        break;
    }
    compressionOptions.setFormat( format );
    //compressionOptions.setQuantization(false,false,false);
    if (format == nvtt::Format_RGBA)
    {
        if (image.getPixelFormat() == GL_RGB)
        {
            compressionOptions.setPixelFormat(24,0xff,0xff00,0xff0000,0);
        }
        else
        {
            compressionOptions.setPixelFormat(32,0xff,0xff00,0xff0000,0xff000000);
        }
    }

    // Handler
    OSGImageOutputHandler outputHandler(format,image.getPixelFormat() == GL_RGB);
    VPBErrorHandler errorHandler;

    // Fill output options
    nvtt::OutputOptions outputOptions;
    outputOptions.setOutputHandler(&outputHandler);
    outputOptions.setErrorHandler(&errorHandler);
    outputOptions.setOutputHeader(false);

    // Process the compression now
    nvtt::Compressor compressor;
    if(method == USE_GPU)
    {
        compressor.enableCudaAcceleration(true);
        if(!compressor.isCudaAccelerationEnabled())
        {
            OSG_WARN<< "CUDA acceleration was enabled but it is not available. CPU will be used."<<std::endl;
        }
    }
    else
    {
        compressor.enableCudaAcceleration(false);
    }

    compressor.process(inputOptions,compressionOptions,outputOptions);

    outputHandler.assignImage(image);
}

void NVTTProcessor::compress(osg::Image& image, osg::Texture::InternalFormatMode compressedFormat, bool generateMipMap, bool resizeToPowerOfTwo, CompressionMethod method, CompressionQuality quality)
{
    nvtt::Format format;
    switch (compressedFormat)
    {
    case osg::Texture::USE_S3TC_DXT1_COMPRESSION:
        if (image.getPixelFormat() == GL_RGBA)
            format = nvtt::Format_DXT1a;
        else
            format = nvtt::Format_DXT1;
        break;
    case osg::Texture::USE_S3TC_DXT1c_COMPRESSION:
        format = nvtt::Format_DXT1;
        break;
    case osg::Texture::USE_S3TC_DXT1a_COMPRESSION:
        format = nvtt::Format_DXT1a;
        break;
    case osg::Texture::USE_S3TC_DXT3_COMPRESSION:
        format = nvtt::Format_DXT3;
        break;
    case osg::Texture::USE_S3TC_DXT5_COMPRESSION:
        format = nvtt::Format_DXT5;
        break;
    default:
        OSG_WARN<<" Invalid or not supported compress format"<<std::endl;
        return;
    }

    process( image, format, generateMipMap, resizeToPowerOfTwo, method, quality );
}

void NVTTProcessor::generateMipMap(osg::Image& image, bool resizeToPowerOfTwo, CompressionMethod method)
{
    process( image, nvtt::Format_RGBA, true, resizeToPowerOfTwo, method, NORMAL);
}

REGISTER_OSGIMAGEPROCESSOR(nvtt, NVTTProcessor)
