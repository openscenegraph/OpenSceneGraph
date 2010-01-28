#include <osgParticle/FireEffect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleFireEffect,
                         new osgParticle::FireEffect,
                         osgParticle::FireEffect,
                         "osg::Object osg::Node osg::Group osgParticle::ParticleEffect osgParticle::FireEffect" )
{
}
