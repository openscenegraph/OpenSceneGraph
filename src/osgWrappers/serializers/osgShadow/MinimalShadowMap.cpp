#include <osgShadow/MinimalShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_MinimalShadowMap,
                         new osgShadow::MinimalShadowMap,
                         osgShadow::MinimalShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap osgShadow::StandardShadowMap osgShadow::MinimalShadowMap" )
{
    ADD_MATRIX_SERIALIZER( ModellingSpaceToWorldTransform, osg::Matrix() );  // _modellingSpaceToWorld
    ADD_FLOAT_SERIALIZER( MaxFarPlane, FLT_MAX );  // _maxFarPlane
    ADD_FLOAT_SERIALIZER( MinLightMargin, 0.0f );  // _minLightMargin

    BEGIN_ENUM_SERIALIZER( ShadowReceivingCoarseBoundAccuracy, BOUNDING_BOX );
        ADD_ENUM_VALUE( EMPTY_BOX );
        ADD_ENUM_VALUE( BOUNDING_SPHERE );
        ADD_ENUM_VALUE( BOUNDING_BOX );
    END_ENUM_SERIALIZER();  // _shadowReceivingCoarseBoundAccuracy
}
