#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( VertexBufferObject,
                         new osg::VertexBufferObject,
                         osg::VertexBufferObject,
                         "osg::Object osg::BufferObject osg::VertexBufferObject" )
{
}
