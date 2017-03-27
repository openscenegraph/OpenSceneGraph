#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( ElementBufferObject,
                         new osg::ElementBufferObject,
                         osg::ElementBufferObject,
                         "osg::Object osg::BufferObject osg::ElementBufferObject" )
{
}
