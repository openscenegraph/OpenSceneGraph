#include <osgGA/GUIEventHandler>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_GUIEventHandler,
                         new osgGA::GUIEventHandler,
                         osgGA::GUIEventHandler,
                         "osg::Object osgGA::GUIEventHandler" )
{
}
