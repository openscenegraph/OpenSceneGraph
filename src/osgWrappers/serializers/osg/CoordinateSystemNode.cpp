#include <osg/CoordinateSystemNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( CoordinateSystemNode,
                         new osg::CoordinateSystemNode,
                         osg::CoordinateSystemNode,
                         "osg::Object osg::Node osg::Group osg::CoordinateSystemNode" )
{
    ADD_STRING_SERIALIZER( Format, "" );  // _format
    ADD_STRING_SERIALIZER( CoordinateSystem, "" );  // _cs
    ADD_OBJECT_SERIALIZER( EllipsoidModel, osg::EllipsoidModel, NULL );  // _ellipsoidModel
}
