#include <osgGA/FlightManipulator>

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_FlightManipulator,
                         new osgGA::FlightManipulator,
                         osgGA::FlightManipulator,
                         "osg::Object osgGA::FlightManipulator" )
{
}
