#include <osgTerrain/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkLayers( const osgTerrain::CompositeLayer& layer )
{
    return layer.getNumLayers()>0;
}

static bool readLayers( osgDB::InputStream& is, osgTerrain::CompositeLayer& layer )
{
    unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgTerrain::Layer* child = dynamic_cast<osgTerrain::Layer*>( is.readObject() );
        if ( child ) layer.addLayer( child );
    }
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeLayers( osgDB::OutputStream& os, const osgTerrain::CompositeLayer& layer )
{
    unsigned int size = layer.getNumLayers();
    os << size << osgDB::BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << layer.getLayer(i);
    }
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgTerrain_CompositeLayer,
                         new osgTerrain::CompositeLayer,
                         osgTerrain::CompositeLayer,
                         "osg::Object osgTerrain::Layer osgTerrain::CompositeLayer" )
{
    ADD_USER_SERIALIZER( Layers );  // _layers
}
