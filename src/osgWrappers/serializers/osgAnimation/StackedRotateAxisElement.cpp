#include <osgAnimation/StackedRotateAxisElement>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_StackedRotateAxisElement,
                         new osgAnimation::StackedRotateAxisElement,
                         osgAnimation::StackedRotateAxisElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedRotateAxisElement" )
{
    ADD_VEC3_SERIALIZER( Axis, osg::Vec3() );  // _axis
    ADD_DOUBLE_SERIALIZER( Angle, 0.0 );  // _angle
}
