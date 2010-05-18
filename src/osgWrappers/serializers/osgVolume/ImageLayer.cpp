#include <osgVolume/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_ImageLayer,
                         new osgVolume::ImageLayer,
                         osgVolume::ImageLayer,
                         "osg::Object osgVolume::Layer osgVolume::ImageLayer" )
{
    ADD_VEC4_SERIALIZER( TexelOffset, osg::Vec4() );  // _texelOffset
    ADD_VEC4_SERIALIZER( TexelScale, osg::Vec4() );  // _texelScale
    ADD_IMAGE_SERIALIZER( Image, osg::Image, NULL );  // _image
}
