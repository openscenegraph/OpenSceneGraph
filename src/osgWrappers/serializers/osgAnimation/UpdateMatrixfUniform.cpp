#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/UpdateUniform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_UpdateMatrixfUniform,
                         new osgAnimation::UpdateMatrixfUniform,
                         osgAnimation::UpdateMatrixfUniform,
                         "osg::Object osg::Callback osg::UniformCallback osgAnimation::UpdateMatrixfUniform" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
