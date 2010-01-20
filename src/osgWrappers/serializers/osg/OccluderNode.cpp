#include <osg/OccluderNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( OccluderNode,
                         new osg::OccluderNode,
                         osg::OccluderNode,
                         "osg::Object osg::Node osg::Group osg::OccluderNode" )
{
    ADD_OBJECT_SERIALIZER( Occluder, osg::ConvexPlanarOccluder, NULL );  // _occluder
}
