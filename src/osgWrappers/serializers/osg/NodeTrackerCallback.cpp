#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/NodeTrackerCallback>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( NodeTrackerCallback,
                         new osg::NodeTrackerCallback,
                         osg::NodeTrackerCallback,
                         "osg::Object osg::NodeCallback osg::NodeTrackerCallback" )
{
    ADD_OBJECT_SERIALIZER( TrackNode, osg::Node, NULL );  // _trackNodePath
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
