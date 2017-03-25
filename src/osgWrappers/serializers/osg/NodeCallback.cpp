#include <osg/Node>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( NodeCallback,
                         new osg::NodeCallback,
                         osg::NodeCallback,
                         "osg::Object osg::Callback osg::NodeCallback" )
{
}
