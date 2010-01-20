#include <osg/Texture1D>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Texture1D,
                         new osg::Texture1D,
                         osg::Texture1D,
                         "osg::Object osg::StateAttribute osg::Texture osg::Texture1D" )
{
    ADD_IMAGE_SERIALIZER( Image, osg::Image, NULL );  // _image
    ADD_INT_SERIALIZER( TextureWidth, 0 );  // _textureWidth
}
