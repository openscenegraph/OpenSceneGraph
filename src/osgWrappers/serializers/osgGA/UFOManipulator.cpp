#include <osgGA/UFOManipulator>

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_UFOManipulator,
                         new osgGA::UFOManipulator,
                         osgGA::UFOManipulator,
                         "osg::Object osgGA::UFOManipulator" )
{
}
