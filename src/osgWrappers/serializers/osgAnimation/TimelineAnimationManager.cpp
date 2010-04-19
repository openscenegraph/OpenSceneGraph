#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/TimelineAnimationManager>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_TimelineAnimationManager,
                         new osgAnimation::TimelineAnimationManager,
                         osgAnimation::TimelineAnimationManager,
                         "osg::Object osg::NodeCallback osgAnimation::AnimationManagerBase osgAnimation::TimelineAnimationManager" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
