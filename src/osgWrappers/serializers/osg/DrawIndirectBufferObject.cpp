#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( DrawIndirectBufferObject,
                         new osg::DrawIndirectBufferObject,
                         osg::DrawIndirectBufferObject,
                         "osg::Object osg::BufferObject osg::DrawIndirectBufferObject" )
{
}
