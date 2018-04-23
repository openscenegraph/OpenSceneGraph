#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace wrapper_osgVertexBufferObject{
REGISTER_OBJECT_WRAPPER( VertexBufferObject,
                         new osg::VertexBufferObject,
                         osg::VertexBufferObject,
                         "osg::Object osg::BufferObject osg::VertexBufferObject" )
{
}
}

namespace wrapper_osgDrawIndirectBufferObject{
REGISTER_OBJECT_WRAPPER( DrawIndirectBufferObject,
                         new osg::DrawIndirectBufferObject,
                         osg::DrawIndirectBufferObject,
                         "osg::Object osg::BufferObject osg::DrawIndirectBufferObject" )
{
}
}
