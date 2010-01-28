#include <osgParticle/SmokeEffect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleSmokeEffect,
                         new osgParticle::SmokeEffect,
                         osgParticle::SmokeEffect,
                         "osg::Object osg::Node osg::Group osgParticle::ParticleEffect osgParticle::SmokeEffect" )
{
}
