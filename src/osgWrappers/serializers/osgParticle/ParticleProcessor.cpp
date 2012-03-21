#include <osgParticle/ParticleProcessor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleParticleProcessor,
                         /*new osgParticle::ParticleProcessor*/NULL,
                         osgParticle::ParticleProcessor,
                         "osg::Object osg::Node osgParticle::ParticleProcessor" )
{
    BEGIN_ENUM_SERIALIZER( ReferenceFrame, RELATIVE_RF );
        ADD_ENUM_VALUE( RELATIVE_RF );
        ADD_ENUM_VALUE( ABSOLUTE_RF );
    END_ENUM_SERIALIZER();  // _rf

    ADD_BOOL_SERIALIZER( Enabled, true );  // _enabled
    ADD_OBJECT_SERIALIZER( ParticleSystem, osgParticle::ParticleSystem, NULL );  // _ps
    ADD_BOOL_SERIALIZER( Endless, true );  // _endless
    ADD_DOUBLE_SERIALIZER( LifeTime, 0.0 );  // _lifeTime
    ADD_DOUBLE_SERIALIZER( StartTime, 0.0 );  // _startTime
    ADD_DOUBLE_SERIALIZER( CurrentTime, 0.0 );  // _currentTime
    ADD_DOUBLE_SERIALIZER( ResetTime, 0.0 );  // _resetTime
}
