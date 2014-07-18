#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgUtil/CullVisitor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( CullVisitor,
                         new osgUtil::CullVisitor,
                         osgUtil::CullVisitor,
                         "osg::Object osg::NodeVisitor osgUtil::CullVisitor" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
