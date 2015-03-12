#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/RigGeometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER(osgAnimation_UpdateRigGeometry,
                        new osgAnimation::UpdateRigGeometry,
                        osgAnimation::UpdateRigGeometry,
                        "osg::Object osg::Callback osg::UpdateCallback osgAnimation::UpdateRigGeometry") {}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
