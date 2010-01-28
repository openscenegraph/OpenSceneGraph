#include <osg/Shape>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Capsule,
                         new osg::Capsule,
                         osg::Capsule,
                         "osg::Object osg::Shape osg::Capsule" )
{
    ADD_VEC3_SERIALIZER( Center, osg::Vec3() );  // _center
    ADD_FLOAT_SERIALIZER( Radius, 0.0f );  // _radius
    ADD_FLOAT_SERIALIZER( Height, 0.0f );  // _height
    ADD_QUAT_SERIALIZER( Rotation, osg::Quat() );  // _rotation
}
