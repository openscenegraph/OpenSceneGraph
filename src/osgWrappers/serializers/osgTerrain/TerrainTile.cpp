#include <osgTerrain/TerrainTile>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _tileID
static bool checkTileID( const osgTerrain::TerrainTile& tile )
{
    return tile.getTileID().valid();
}

static bool readTileID( osgDB::InputStream& is, osgTerrain::TerrainTile& tile )
{
    osgTerrain::TileID id;
    is >> id.level >> id.x >> id.y;
    tile.setTileID( id );
    return true;
}

static bool writeTileID( osgDB::OutputStream& os, const osgTerrain::TerrainTile& tile )
{
    const osgTerrain::TileID& id = tile.getTileID();
    os << id.level << id.x << id.y << std::endl;
    return true;
}

// _colorLayers
static bool checkColorLayers( const osgTerrain::TerrainTile& tile )
{
    return tile.getNumColorLayers()>0;
}

static bool readColorLayers( osgDB::InputStream& is, osgTerrain::TerrainTile& tile )
{
    unsigned int numValidLayers = 0; is >> numValidLayers >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<numValidLayers; ++i )
    {
        unsigned int layerNum=0; is >> is.PROPERTY("Layer") >> layerNum;
        osg::ref_ptr<osgTerrain::Layer> layer = is.readObjectOfType<osgTerrain::Layer>();
        if ( layer ) tile.setColorLayer( layerNum, layer );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeColorLayers( osgDB::OutputStream& os, const osgTerrain::TerrainTile& tile )
{
    unsigned int numValidLayers = 0;
    for ( unsigned int i=0; i<tile.getNumColorLayers(); ++i )
    {
        if (tile.getColorLayer(i)) ++numValidLayers;
    }

    os << numValidLayers << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<tile.getNumColorLayers(); ++i )
    {
        if (tile.getColorLayer(i)) os << os.PROPERTY("Layer") << i << tile.getColorLayer(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

struct TerrainTileFinishedObjectReadCallback : public osgDB::FinishedObjectReadCallback
{
    virtual void objectRead(osgDB::InputStream& is, osg::Object& obj)
    {
        osgTerrain::TerrainTile& tile = static_cast<osgTerrain::TerrainTile&>(obj);

        if (is.getOptions())
        {
            osg::ref_ptr<osg::Node> node;
            if (is.getOptions()->getTerrain().lock(node))
            {
                tile.setTerrain(node->asTerrain());
            }
        }

        if ( osgTerrain::TerrainTile::getTileLoadedCallback().valid() )
            osgTerrain::TerrainTile::getTileLoadedCallback()->loaded( &tile, is.getOptions() );
        }
};


REGISTER_OBJECT_WRAPPER( osgTerrain_TerrainTile,
                         new osgTerrain::TerrainTile,
                         osgTerrain::TerrainTile,
                         "osg::Object osg::Node osg::Group osgTerrain::TerrainTile" )
{
    ADD_USER_SERIALIZER( TileID );  // _tileID
    ADD_OBJECT_SERIALIZER( TerrainTechnique, osgTerrain::TerrainTechnique, NULL );  // _terrainTechnique
    ADD_OBJECT_SERIALIZER( Locator, osgTerrain::Locator, NULL );  // _locator
    ADD_OBJECT_SERIALIZER( ElevationLayer, osgTerrain::Layer, NULL );  // _elevationLayer
    ADD_USER_SERIALIZER( ColorLayers );  // _colorLayers
    ADD_BOOL_SERIALIZER( RequiresNormals, true );  // _requiresNormals
    ADD_BOOL_SERIALIZER( TreatBoundariesToValidDataAsDefaultValue, false );  // _treatBoundariesToValidDataAsDefaultValue
    BEGIN_ENUM_SERIALIZER( BlendingPolicy, INHERIT );
        ADD_ENUM_VALUE( INHERIT );
        ADD_ENUM_VALUE( DO_NOT_SET_BLENDING );
        ADD_ENUM_VALUE( ENABLE_BLENDING );
        ADD_ENUM_VALUE( ENABLE_BLENDING_WHEN_ALPHA_PRESENT );
    END_ENUM_SERIALIZER();  // BlendingPolicy

    wrapper->addFinishedObjectReadCallback( new TerrainTileFinishedObjectReadCallback() );
}
