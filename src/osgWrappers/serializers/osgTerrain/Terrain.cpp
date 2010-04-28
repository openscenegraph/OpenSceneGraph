#include <osgTerrain/Terrain>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_Terrain,
                         new osgTerrain::Terrain,
                         osgTerrain::Terrain,
                         "osg::Object osg::Node osg::Group osg::CoordinateSystemNode osgTerrain::Terrain" )
{
    ADD_FLOAT_SERIALIZER( SampleRatio, 1.0f );  // _sampleRatio
    ADD_FLOAT_SERIALIZER( VerticalScale, 1.0f );  // _verticalScale

    BEGIN_ENUM_SERIALIZER4( osgTerrain::TerrainTile, BlendingPolicy, INHERIT );
        ADD_ENUM_CLASS_VALUE( osgTerrain::TerrainTile, INHERIT );
        ADD_ENUM_CLASS_VALUE( osgTerrain::TerrainTile, DO_NOT_SET_BLENDING );
        ADD_ENUM_CLASS_VALUE( osgTerrain::TerrainTile, ENABLE_BLENDING );
        ADD_ENUM_CLASS_VALUE( osgTerrain::TerrainTile, ENABLE_BLENDING_WHEN_ALPHA_PRESENT );
    END_ENUM_SERIALIZER();  // BlendingPolicy
}
