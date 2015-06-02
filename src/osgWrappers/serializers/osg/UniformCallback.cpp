#include <osg/Node>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( UniformCallback,
                         new osg::UniformCallback,
                         osg::UniformCallback,
                         "osg::Object osg::Callback osg::UniformCallback" )
{
}
