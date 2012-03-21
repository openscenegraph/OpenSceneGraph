#include <osg/CameraView>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( CameraView,
                         new osg::CameraView,
                         osg::CameraView,
                         "osg::Object osg::Node osg::Group osg::Transform osg::CameraView" )
{
    ADD_VEC3D_SERIALIZER( Position, osg::Vec3d() );  // _position
    ADD_QUAT_SERIALIZER( Attitude, osg::Quat() );  // _attitude
    ADD_DOUBLE_SERIALIZER( FieldOfView, 60.0 );  // _fieldOfView

    BEGIN_ENUM_SERIALIZER( FieldOfViewMode, VERTICAL );
        ADD_ENUM_VALUE( UNCONSTRAINED );
        ADD_ENUM_VALUE( HORIZONTAL );
        ADD_ENUM_VALUE( VERTICAL );
    END_ENUM_SERIALIZER();  // _fieldOfViewMode

    ADD_DOUBLE_SERIALIZER( FocalLength, 0.0 );  // _focalLength
}
