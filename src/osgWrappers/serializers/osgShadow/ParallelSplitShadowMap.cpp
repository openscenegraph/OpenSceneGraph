#include <osgShadow/ParallelSplitShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>



REGISTER_OBJECT_WRAPPER( osgShadow_ParallelSplitShadowMap,
                         new osgShadow::ParallelSplitShadowMap,
                         osgShadow::ParallelSplitShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ParallelSplitShadowMap" )
{
    ADD_VEC2F_SERIALIZER( PolygonOffset, osg::Vec2f() );  // _polgyonOffset
    ADD_UINT_SERIALIZER( TextureResolution, 1024 );  // _resolution
    ADD_DOUBLE_SERIALIZER( MaxFarDistance, 0.0 );  // _setMaxFarDistance
    ADD_DOUBLE_SERIALIZER( MoveVCamBehindRCamFactor, 0.0 );  // _move_vcam_behind_rcam_factor
    ADD_DOUBLE_SERIALIZER( MinNearDistanceForSplits, 5.0 );  // _split_min_near_dist
    ADD_OBJECT_SERIALIZER( UserLight, osg::Light, NULL );  // _userLight
    ADD_VEC2_SERIALIZER( AmbientBias, osg::Vec2() );  // _ambientBias

    BEGIN_ENUM_SERIALIZER2( SplitCalculationMode, osgShadow::ParallelSplitShadowMap::SplitCalcMode, SPLIT_EXP );
        ADD_ENUM_VALUE( SPLIT_LINEAR );
        ADD_ENUM_VALUE( SPLIT_EXP );
    END_ENUM_SERIALIZER();  // _SplitCalcMode
}
