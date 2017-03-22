#include <osg/TextureBuffer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( TextureBuffer,
                         new osg::TextureBuffer,
                         osg::TextureBuffer,
                         "osg::Object osg::StateAttribute osg::Texture osg::TextureBuffer" )
{
    ADD_INT_SERIALIZER( TextureWidth, 0 );                       // _textureWidth
    ADD_OBJECT_SERIALIZER( BufferData, osg::BufferData, NULL );  // _bufferData
}
