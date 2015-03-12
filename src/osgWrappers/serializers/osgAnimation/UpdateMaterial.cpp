#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/UpdateMaterial>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_UpdateMaterial,
                         new osgAnimation::UpdateMaterial,
                         osgAnimation::UpdateMaterial,
                         "osg::Object osg::Callback osgAnimation::UpdateMaterial" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
