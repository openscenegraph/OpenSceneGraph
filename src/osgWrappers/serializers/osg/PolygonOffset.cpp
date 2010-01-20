#include <osg/PolygonOffset>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( PolygonOffset,
                         new osg::PolygonOffset,
                         osg::PolygonOffset,
                         "osg::Object osg::StateAttribute osg::PolygonOffset" )
{
    ADD_FLOAT_SERIALIZER( Factor, 0.0f );  // _factor
    ADD_FLOAT_SERIALIZER( Units, 0.0f );  // _units
}
