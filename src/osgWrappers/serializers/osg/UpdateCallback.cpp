#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/Drawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER2(osg_Drawable_UpdateCallback,
                         new osg::Drawable::UpdateCallback,
                         osg::Drawable::UpdateCallback,
                         "osg::UpdateCallback",
                         "osg::Object osg::UpdateCallback") {}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
