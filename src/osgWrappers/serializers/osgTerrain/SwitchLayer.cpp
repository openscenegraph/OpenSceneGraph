#include <osgTerrain/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_SwitchLayer,
                         new osgTerrain::SwitchLayer,
                         osgTerrain::SwitchLayer,
                         "osg::Object osgTerrain::Layer osgTerrain::CompositeLayer osgTerrain::SwitchLayer" )
{
    ADD_INT_SERIALIZER( ActiveLayer, -1 );  // _activeLayer
}
