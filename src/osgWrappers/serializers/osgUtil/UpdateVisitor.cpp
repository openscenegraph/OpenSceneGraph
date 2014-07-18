#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgUtil/UpdateVisitor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( UpdateVisitor,
                         new osgUtil::UpdateVisitor,
                         osgUtil::UpdateVisitor,
                         "osg::Object osg::NodeVisitor osgUtil::UpdateVisitor" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
