#include <osg/Point>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Point,
                         new osg::Point,
                         osg::Point,
                         "osg::Object osg::StateAttribute osg::Point" )
{
    ADD_FLOAT_SERIALIZER( Size, 0.0f );  // _size
    ADD_FLOAT_SERIALIZER( FadeThresholdSize, 0.0f );  // _fadeThresholdSize
    ADD_VEC3_SERIALIZER( DistanceAttenuation, osg::Vec3() );  // _distanceAttenuation
    ADD_FLOAT_SERIALIZER( MinSize, 0.0f );  // _minSize
    ADD_FLOAT_SERIALIZER( MaxSize, 0.0f );  // _maxSize
}
