#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/UpdateUniform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_UpdateFloatUniform,
                         new osgAnimation::UpdateFloatUniform,
                         osgAnimation::UpdateFloatUniform,
                         "osg::Object osg::Callback osg::UniformCallback osgAnimation::UpdateFloatUniform" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
