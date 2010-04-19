#include <osgAnimation/ActionStripAnimation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_ActionStripAnimation,
                         new osgAnimation::ActionStripAnimation,
                         osgAnimation::ActionStripAnimation,
                         "osg::Object osgAnimation::Action osgAnimation::ActionStripAnimation" )
{
}
