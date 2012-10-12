#include <osgGA/MultiTouchTrackballManipulator>

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_MultiTouchTrackballManipulator,
                         new osgGA::MultiTouchTrackballManipulator,
                         osgGA::MultiTouchTrackballManipulator,
                         "osg::Object osgGA::MultiTouchTrackballManipulator" )
{
}
