#include <osgAnimation/ActionAnimation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_ActionAnimation,
                         new osgAnimation::ActionAnimation,
                         osgAnimation::ActionAnimation,
                         "osg::Object osgAnimation::Action osgAnimation::ActionAnimation" )
{
}
