#include <osgAnimation/Skeleton>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_Skeleton,
                         new osgAnimation::Skeleton,
                         osgAnimation::Skeleton,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgAnimation::Skeleton" )
{
}
