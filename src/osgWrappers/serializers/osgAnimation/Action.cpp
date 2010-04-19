#include <osgAnimation/Action>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_Action,
                         new osgAnimation::Action,
                         osgAnimation::Action,
                         "osg::Object osgAnimation::Action" )
{
    //ADD_USER_SERIALIZER( Callback );  // _framesCallback
    ADD_UINT_SERIALIZER( NumFrames, 25 );  // _numberFrame
    ADD_UINT_SERIALIZER( Loop, 1 );  // _loop
}
