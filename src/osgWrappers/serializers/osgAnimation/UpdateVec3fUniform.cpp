#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/UpdateUniform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_UpdateVec3fUniform,
                         new osgAnimation::UpdateVec3fUniform,
                         osgAnimation::UpdateVec3fUniform,
                         "osg::Object osg::Callback osg::UniformCallback osgAnimation::UpdateVec3fUniform" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
