#include <osgParticle/PrecipitationEffect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticlePrecipitationEffect,
                         new osgParticle::PrecipitationEffect,
                         osgParticle::PrecipitationEffect,
                         "osg::Object osg::Node osgParticle::PrecipitationEffect" )
{
    ADD_VEC3_SERIALIZER( Wind, osg::Vec3() );  // _wind
    ADD_FLOAT_SERIALIZER( ParticleSpeed, 0.0f );  // _particleSpeed
    ADD_FLOAT_SERIALIZER( ParticleSize, 0.0f );  // _particleSize
    ADD_VEC4_SERIALIZER( ParticleColor, osg::Vec4() );  // _particleColor
    ADD_FLOAT_SERIALIZER( MaximumParticleDensity, 0.0f );  // _maximumParticleDensity
    ADD_VEC3_SERIALIZER( CellSize, osg::Vec3() );  // _cellSize
    ADD_FLOAT_SERIALIZER( NearTransition, 0.0f );  // _nearTransition
    ADD_FLOAT_SERIALIZER( FarTransition, 0.0f );  // _farTransition
    ADD_BOOL_SERIALIZER( UseFarLineSegments, false );  // _useFarLineSegments
    ADD_OBJECT_SERIALIZER( Fog, osg::Fog, NULL );  // _fog
}
