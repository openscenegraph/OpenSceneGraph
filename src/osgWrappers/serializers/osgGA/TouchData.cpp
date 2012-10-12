#include <osgGA/GUIEventAdapter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_TouchData,
                         new osgGA::GUIEventAdapter::TouchData,
                         osgGA::GUIEventAdapter::TouchData,
                         "osg::Object osgGA::TouchData" )
{
}
