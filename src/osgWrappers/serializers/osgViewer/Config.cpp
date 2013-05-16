#include <osgViewer/View>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_ViewConfig,
                         new osgViewer::ViewConfig,
                         osgViewer::ViewConfig,
                         "osg::Object osgViewer::ViewConfig" )
{
}
