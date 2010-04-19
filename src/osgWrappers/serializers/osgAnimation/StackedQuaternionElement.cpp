#include <osgAnimation/StackedQuaternionElement>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_StackedQuaternionElement,
                         new osgAnimation::StackedQuaternionElement,
                         osgAnimation::StackedQuaternionElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedQuaternionElement" )
{
    ADD_QUAT_SERIALIZER( Quaternion, osg::Quat() );  // _quaternion
}
