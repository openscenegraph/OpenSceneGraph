#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/Skeleton>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER2( osgAnimation_UpdateSkeleton,
                          new osgAnimation::Skeleton::UpdateSkeleton,
                          osgAnimation::Skeleton::UpdateSkeleton,
                          "osgAnimation::UpdateSkeleton",
                          "osg::Object osg::Callback osg::NodeCallback osgAnimation::UpdateSkeleton" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
