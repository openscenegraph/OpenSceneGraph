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
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string type;
        is >> type;
        if ( type=="Object" )
        {
            osgTerrain::Layer* child = dynamic_cast<osgTerrain::Layer*>( is.readObject() );
            if ( child ) layer.addLayer( child );
        }
        else if ( type=="File" )
        {
            std::string compoundname;
            is.readWrappedString( compoundname );
            layer.addLayer( compoundname );
        }
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeLayers( osgDB::OutputStream& os, const osgTerrain::CompositeLayer& layer )
{
    unsigned int size = layer.getNumLayers();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        const osgTerrain::Layer* child = layer.getLayer(i);
        std::string type = child ? std::string("Object") : std::string("File");
        os << type;
        if ( child )
        {
            os << child;
        }
        else
        {
            os.writeWrappedString( layer.getCompoundName(i) );
            os << std::endl;
        }
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgTerrain_CompositeLayer,
                         new osgTerrain::CompositeLayer,
                         osgTerrain::CompositeLayer,
                         "osg::Object osgTerrain::Layer osgTerrain::CompositeLayer" )
{
    ADD_USER_SERIALIZER( Layers );  // _layers
}
