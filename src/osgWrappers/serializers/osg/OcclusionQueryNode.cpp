#include <osg/OcclusionQueryNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( OcclusionQueryNode,
                         new osg::OcclusionQueryNode,
                         osg::OcclusionQueryNode,
                         "osg::Object osg::Node osg::Group osg::OcclusionQueryNode" )
{
    ADD_BOOL_SERIALIZER( QueriesEnabled, true );  // _enabled
    ADD_UINT_SERIALIZER( VisibilityThreshold, 0 );  // _visThreshold
    ADD_UINT_SERIALIZER( QueryFrameCount, 0 );  // _queryFrameCount
    ADD_BOOL_SERIALIZER( DebugDisplay, false );  // _debugBB
}
