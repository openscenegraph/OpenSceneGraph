#include <osgParticle/ParticleEffect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _particleSystem
static bool checkParticleSystem( const osgParticle::ParticleEffect& effect )
{
    return (!effect.getUseLocalParticleSystem()) && (effect.getParticleSystem()!=NULL);
}

static bool readParticleSystem( osgDB::InputStream& is, osgParticle::ParticleEffect& effect )
{
    is >> is.BEGIN_BRACKET;
    effect.setUseLocalParticleSystem( false );
    effect.setParticleSystem( static_cast<osgParticle::ParticleSystem*>(is.readObject()) );
    is >> is.END_BRACKET;
    return true;
}

static bool writeParticleSystem( osgDB::OutputStream& os, const osgParticle::ParticleEffect& effect )
{
    os << os.BEGIN_BRACKET << std::endl;
    os << effect.getParticleSystem();
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleParticleEffect,
                         /*new osgParticle::ParticleEffect*/NULL,
                         osgParticle::ParticleEffect,
                         "osg::Object osg::Node osg::Group osgParticle::ParticleEffect" )
{
    ADD_USER_SERIALIZER( ParticleSystem );  // _particleSystem
    ADD_STRING_SERIALIZER( TextureFileName, "" );  // _textureFileName
    ADD_VEC3_SERIALIZER( Position, osg::Vec3() );  // _position
    ADD_FLOAT_SERIALIZER( Scale, 0.0f );  // _scale
    ADD_FLOAT_SERIALIZER( Intensity, 0.0f );  // _intensity
    ADD_DOUBLE_SERIALIZER( StartTime, 0.0 );  // _startTime
    ADD_DOUBLE_SERIALIZER( EmitterDuration, 0.0 );  // _emitterDuration
    ADD_VEC3_SERIALIZER( Wind, osg::Vec3() );  // _wind
}
