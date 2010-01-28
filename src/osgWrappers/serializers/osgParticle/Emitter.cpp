#include <osgParticle/Emitter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

extern bool readParticle( osgDB::InputStream& is, osgParticle::Particle& p );
extern bool writeParticle( osgDB::OutputStream& os, const osgParticle::Particle& p );

static bool checkParticleTemplate( const osgParticle::Emitter& emitter )
{
    return !emitter.getUseDefaultTemplate();
}

static bool readParticleTemplate( osgDB::InputStream& is, osgParticle::Emitter& emitter )
{
    osgParticle::Particle p;
    readParticle( is, p );
    emitter.setParticleTemplate( p );
    return true;
}

static bool writeParticleTemplate( osgDB::OutputStream& os, const osgParticle::Emitter& emitter )
{
    const osgParticle::Particle& p = emitter.getParticleTemplate();
    writeParticle( os, p );
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleEmitter,
                         /*new osgParticle::Emitter*/NULL,
                         osgParticle::Emitter,
                         "osg::Object osg::Node osgParticle::ParticleProcessor osgParticle::Emitter" )
{
    ADD_BOOL_SERIALIZER( UseDefaultTemplate, true );  // _usedeftemp
    ADD_USER_SERIALIZER( ParticleTemplate );  // _ptemp
}
