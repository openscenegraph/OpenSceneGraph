#include <osgViewer/Config>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_ViewAcrossAllScreens,
                         new osgViewer::ViewAcrossAllScreens,
                         osgViewer::ViewAcrossAllScreens,
                         "osg::Object osgViewer::Config osgViewer::ViewAcrossAllScreens" )
{
}
