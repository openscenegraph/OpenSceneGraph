#include <osgTerrain/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_HeightFieldLayer,
                         new osgTerrain::HeightFieldLayer,
                         osgTerrain::HeightFieldLayer,
                         "osg::Object osgTerrain::Layer osgTerrain::HeightFieldLayer" )
{
    ADD_OBJECT_SERIALIZER( HeightField, osg::HeightField, NULL );  // _heightField
}
