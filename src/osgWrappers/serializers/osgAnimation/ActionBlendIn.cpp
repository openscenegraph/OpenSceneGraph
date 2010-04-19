#include <osgAnimation/ActionBlendIn>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_ActionBlendIn,
                         new osgAnimation::ActionBlendIn,
                         osgAnimation::ActionBlendIn,
                         "osg::Object osgAnimation::Action osgAnimation::ActionBlendIn" )
{
}
