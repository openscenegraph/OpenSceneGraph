#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/RigGeometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER2(osg_Drawable_UpdateCallback,
                         new osgAnimation::RigGeometry::UpdateVertex,
                         osgAnimation::RigGeometry::UpdateVertex,
                         "osgAnimation::UpdateVertex",
                         "osg::Object osg::UpdateCallback osgAnimation::UpdateVertex") {}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
