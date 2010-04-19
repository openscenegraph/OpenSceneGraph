#include <osgAnimation/Timeline>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_Timeline,
                         new osgAnimation::Timeline,
                         osgAnimation::Timeline,
                         "osg::Object osgAnimation::Action osgAnimation::Timeline" )
{
}
