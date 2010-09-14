#include <osgParticle/OrbitOperator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleOrbitOperator,
                         new osgParticle::OrbitOperator,
                         osgParticle::OrbitOperator,
                         "osg::Object osgParticle::Operator osgParticle::OrbitOperator" )
{
    ADD_VEC3_SERIALIZER( Center, osg::Vec3() );  // _center
    ADD_FLOAT_SERIALIZER( Magnitude, 1.0f );  // _magnitude
    ADD_FLOAT_SERIALIZER( Epsilon, 1e-3 );  // _epsilon
    ADD_FLOAT_SERIALIZER( MaxRadius, FLT_MAX );  // _maxRadius
}
