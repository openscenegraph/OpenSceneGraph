#include <osg/Image>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Image,
                         new osg::Image,
                         osg::Image,
                         "osg::Object osg::Image" )
{
    // Everything is done in OutputStream and InputStream classes
}
