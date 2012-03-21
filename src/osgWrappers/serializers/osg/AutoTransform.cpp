#include <osg/AutoTransform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( AutoTransform,
                         new osg::AutoTransform,
                         osg::AutoTransform,
                         "osg::Object osg::Node osg::Group osg::Transform osg::AutoTransform" )
{
    ADD_DOUBLE_SERIALIZER( MinimumScale, 0.0 );  // _minimumScale
    ADD_DOUBLE_SERIALIZER( MaximumScale, 0.0 );  // _maximumScale
    ADD_VEC3D_SERIALIZER( Position, osg::Vec3d() );  // _position
    ADD_VEC3D_SERIALIZER( Scale, osg::Vec3d() );  // _scale
    ADD_VEC3D_SERIALIZER( PivotPoint, osg::Vec3d() );  // _pivotPoint
    ADD_FLOAT_SERIALIZER( AutoUpdateEyeMovementTolerance, 0.0f );  // _autoUpdateEyeMovementTolerance

    BEGIN_ENUM_SERIALIZER( AutoRotateMode, NO_ROTATION );
        ADD_ENUM_VALUE( NO_ROTATION );
        ADD_ENUM_VALUE( ROTATE_TO_SCREEN );
        ADD_ENUM_VALUE( ROTATE_TO_CAMERA );
    END_ENUM_SERIALIZER();  // _autoRotateMode

    ADD_BOOL_SERIALIZER( AutoScaleToScreen, false );  // _autoScaleToScreen
    ADD_QUAT_SERIALIZER( Rotation, osg::Quat() );  // _rotation
    ADD_FLOAT_SERIALIZER( AutoScaleTransitionWidthRatio, 0.25f );  // _autoScaleTransitionWidthRatio
}
