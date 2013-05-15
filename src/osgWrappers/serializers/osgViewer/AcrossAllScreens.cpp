#include <osgViewer/config/AcrossAllScreens>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_AcrossAllScreens,
                         new osgViewer::AcrossAllScreens,
                         osgViewer::AcrossAllScreens,
                         "osg::Object osgViewer::Config osgViewer::AcrossAllScreens" )
{
}
