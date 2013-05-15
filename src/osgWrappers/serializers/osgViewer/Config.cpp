#include <osgViewer/Config>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_Config,
                         new osgViewer::Config,
                         osgViewer::Config,
                         "osg::Object osgViewer::Config" )
{
}
