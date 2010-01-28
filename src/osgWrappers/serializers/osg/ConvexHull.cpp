#include <osg/Shape>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( ConvexHull,
                         new osg::ConvexHull,
                         osg::ConvexHull,
                         "osg::Object osg::Shape osg::TriangleMesh osg::ConvexHull" )
{
}
