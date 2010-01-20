#include <osg/PositionAttitudeTransform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( PositionAttitudeTransform,
                         new osg::PositionAttitudeTransform,
                         osg::PositionAttitudeTransform,
                         "osg::Object osg::Node osg::Group osg::Transform osg::PositionAttitudeTransform" )
{
    ADD_VEC3D_SERIALIZER( Position, osg::Vec3d() );  // _position
    ADD_QUAT_SERIALIZER( Attitude, osg::Quat() );  // _attitude
    ADD_VEC3D_SERIALIZER( Scale, osg::Vec3d() );  // _scale
    ADD_VEC3D_SERIALIZER( PivotPoint, osg::Vec3d() );  // _pivotPoint
}
