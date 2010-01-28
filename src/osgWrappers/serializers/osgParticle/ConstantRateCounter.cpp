#include <osgParticle/ConstantRateCounter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleConstantRateCounter,
                         new osgParticle::ConstantRateCounter,
                         osgParticle::ConstantRateCounter,
                         "osg::Object osgParticle::Counter osgParticle::ConstantRateCounter" )
{
    ADD_INT_SERIALIZER( MinimumNumberOfParticlesToCreate, 0 );  // _minimumNumberOfParticlesToCreate
    ADD_DOUBLE_SERIALIZER( NumberOfParticlesPerSecondToCreate, 0.0 );  // _numberOfParticlesPerSecondToCreate
}
