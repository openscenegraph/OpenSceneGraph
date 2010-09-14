#include <osgParticle/ParticleSystemUpdater>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkParticleSystems( const osgParticle::ParticleSystemUpdater& updater )
{
    return updater.getNumParticleSystems()>0;
}

static bool readParticleSystems( osgDB::InputStream& is, osgParticle::ParticleSystemUpdater& updater )
{
    unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgParticle::ParticleSystem* ps = dynamic_cast<osgParticle::ParticleSystem*>( is.readObject() );
        if ( ps ) updater.addParticleSystem( ps );
    }
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeParticleSystems( osgDB::OutputStream& os, const osgParticle::ParticleSystemUpdater& updater )
{
    unsigned int size = updater.getNumParticleSystems();
    os << size << osgDB::BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << updater.getParticleSystem(i);
    }
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleParticleSystemUpdater,
                         new osgParticle::ParticleSystemUpdater,
                         osgParticle::ParticleSystemUpdater,
                         "osg::Object osg::Node osg::Geode osgParticle::ParticleSystemUpdater" )
{
    ADD_USER_SERIALIZER( ParticleSystems );  // _psv
}
