#include <osgParticle/ExplosionOperator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleExplosionOperator,
                         new osgParticle::ExplosionOperator,
                         osgParticle::ExplosionOperator,
                         "osg::Object osgParticle::Operator osgParticle::ExplosionOperator" )
{
    ADD_VEC3_SERIALIZER( Center, osg::Vec3() );  // _center
    ADD_FLOAT_SERIALIZER( Radius, 1.0f );  // _radius
    ADD_FLOAT_SERIALIZER( Magnitude, 1.0f );  // _magnitude
    ADD_FLOAT_SERIALIZER( Epsilon, 1e-3 );  // _epsilon
    ADD_FLOAT_SERIALIZER( Sigma, 1.0f );  // _sigma
}
