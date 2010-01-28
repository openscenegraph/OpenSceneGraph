#include <osgParticle/RandomRateCounter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleRandomRateCounter,
                         new osgParticle::RandomRateCounter,
                         osgParticle::RandomRateCounter,
                         "osg::Object osgParticle::Counter osgParticle::VariableRateCounter osgParticle::RandomRateCounter" )
{
}
