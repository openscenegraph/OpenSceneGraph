#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/MorphGeometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER(osgAnimation_UpdateMorphGeometry,
                        new osgAnimation::UpdateMorphGeometry,
                        osgAnimation::UpdateMorphGeometry,
                        "osg::Object osg::Callback osg::UpdateCallback osgAnimation::UpdateMorphGeometry") {}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
