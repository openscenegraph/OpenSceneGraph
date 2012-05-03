#include <osgVolume/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkLayers( const osgVolume::CompositeLayer& layer )
{
    return layer.getNumLayers()>0;
}

static bool readLayers( osgDB::InputStream& is, osgVolume::CompositeLayer& layer )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgVolume::Layer* child = dynamic_cast<osgVolume::Layer*>( is.readObject() );
        if ( child ) layer.addLayer( child );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeLayers( osgDB::OutputStream& os, const osgVolume::CompositeLayer& layer )
{
    unsigned int size = layer.getNumLayers();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << layer.getLayer(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgVolume_CompositeLayer,
                         new osgVolume::CompositeLayer,
                         osgVolume::CompositeLayer,
                         "osg::Object osgVolume::Layer osgVolume::CompositeLayer" )
{
    ADD_USER_SERIALIZER( Layers );  // _layers
}
