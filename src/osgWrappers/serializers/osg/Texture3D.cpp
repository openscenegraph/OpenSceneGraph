#include <osg/Texture3D>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Texture3D,
                         new osg::Texture3D,
                         osg::Texture3D,
                         "osg::Object osg::StateAttribute osg::Texture osg::Texture3D" )
{
    ADD_IMAGE_SERIALIZER( Image, osg::Image, NULL );  // _image
    ADD_INT_SERIALIZER( TextureWidth, 0 );  // _textureWidth
    ADD_INT_SERIALIZER( TextureHeight, 0 );  // _textureHeight
    ADD_INT_SERIALIZER( TextureDepth, 0 );  // _textureDepth
}
