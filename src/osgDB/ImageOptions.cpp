#include <osgDB/ImageOptions>


using namespace osgDB;


ImageOptions::ImageOptions()
{
    init();
}

ImageOptions::ImageOptions(const std::string& str)
{
    init();   
    _str = str;
}


void ImageOptions::init()
{
    _sourceImageSamplingMode = NEAREST;
    _sourceImageWindowMode = ALL_IMAGE;
    
    _destinationImageWindowMode = ALL_IMAGE;
    
    _destinationDataType = GL_NONE;
    _destinationPixelFormat = GL_NONE;
}
