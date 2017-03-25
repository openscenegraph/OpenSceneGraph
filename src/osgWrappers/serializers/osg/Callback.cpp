#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/Node>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Callback,
                         new osg::Callback,
                         osg::Callback,
                         "osg::Object osg::Callback" )
{
    ADD_OBJECT_SERIALIZER( NestedCallback, osg::Callback, NULL );  // _nestedCallback
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
