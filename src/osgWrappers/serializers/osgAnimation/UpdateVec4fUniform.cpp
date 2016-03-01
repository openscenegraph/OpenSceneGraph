#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/UpdateUniform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_UpdateVec4fUniform,
                         new osgAnimation::UpdateVec4fUniform,
                         osgAnimation::UpdateVec4fUniform,
                         "osg::Object osg::Callback osg::UniformCallback osgAnimation::UpdateVec4fUniform" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
