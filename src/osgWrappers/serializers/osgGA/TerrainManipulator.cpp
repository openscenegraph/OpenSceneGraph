#include <osgGA/TerrainManipulator>

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_TerrainManipulator,
                         new osgGA::TerrainManipulator,
                         osgGA::TerrainManipulator,
                         "osg::Object osgGA::TerrainManipulator" )
{
}
