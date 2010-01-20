#include <osg/Shape>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Sphere,
                         new osg::Sphere,
                         osg::Sphere,
                         "osg::Object osg::Shape osg::Sphere" )
{
    ADD_VEC3_SERIALIZER( Center, osg::Vec3() );  // _center
    ADD_FLOAT_SERIALIZER( Radius, 0.0f );  // _radius
}
