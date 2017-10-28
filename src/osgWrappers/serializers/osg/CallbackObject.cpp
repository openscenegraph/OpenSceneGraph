#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/Node>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( CallbackObject,
                         new osg::CallbackObject,
                         osg::CallbackObject,
                         "osg::Object osg::Callback osg::CallbackObject" )
{
}

//MY
#undef OBJECT_CAST
#define OBJECT_CAST static_cast
