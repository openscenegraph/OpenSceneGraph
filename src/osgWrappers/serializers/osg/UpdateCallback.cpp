#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/Drawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER2(UpdateCallback,
                         new osg::DrawableUpdateCallback,
                         osg::DrawableUpdateCallback,
                         "osg::UpdateCallback",
                         "osg::Object osg::Callback osg::UpdateCallback") {}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
