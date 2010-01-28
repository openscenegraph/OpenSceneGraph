#include <osg/Shape>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Shape,
                         /*new osg::Shape*/NULL,
                         osg::Shape,
                         "osg::Object osg::Shape" )
{
}
