#include <osgVolume/Volume>
#include <osgVolume/VolumeTile>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkTileID( const osgVolume::VolumeTile& tile )
{ return true; }

static bool readTileID( osgDB::InputStream& is, osgVolume::VolumeTile& tile )
{
    osgVolume::TileID id;
    is >> id.level >> id.x >> id.y >> id.z;
    tile.setTileID( id );
    return true;
}

static bool writeTileID( osgDB::OutputStream& os, const osgVolume::VolumeTile& tile )
{
    const osgVolume::TileID& id = tile.getTileID();
    os << id.level << id.x << id.y << id.z << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgVolume_VolumeTile,
                         new osgVolume::VolumeTile,
                         osgVolume::VolumeTile,
                         "osg::Object osg::Node osg::Group osgVolume::VolumeTile" )
{
    ADD_OBJECT_SERIALIZER( Volume, osgVolume::Volume, NULL );  // _volume
    {
        UPDATE_TO_VERSION_SCOPED( 90 )
        REMOVE_SERIALIZER( Volume );
    }

    ADD_BOOL_SERIALIZER( Dirty, false );  // _dirty
    ADD_USER_SERIALIZER( TileID );  // _tileID
    ADD_OBJECT_SERIALIZER( VolumeTechnique, osgVolume::VolumeTechnique, NULL );  // _volumeTechnique
    ADD_OBJECT_SERIALIZER( Locator, osgVolume::Locator, NULL );  // _locator
    ADD_OBJECT_SERIALIZER( Layer, osgVolume::Layer, NULL );  // _layer
}
