#include <osgParticle/SmokeTrailEffect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleSmokeTrailEffect,
                         new osgParticle::SmokeTrailEffect,
                         osgParticle::SmokeTrailEffect,
                         "osg::Object osg::Node osg::Group osgParticle::ParticleEffect osgParticle::SmokeTrailEffect" )
{
}
