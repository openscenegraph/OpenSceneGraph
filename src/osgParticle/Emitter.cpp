#include <osgParticle/Emitter>
#include <osgParticle/ParticleProcessor>

#include <osg/CopyOp>

osgParticle::Emitter::Emitter()
:    ParticleProcessor(),
    _usedeftemp(true),
    _estimatedMaxNumOfParticles(0)
{
}

osgParticle::Emitter::Emitter(const Emitter& copy, const osg::CopyOp& copyop)
:     ParticleProcessor(copy, copyop),
    _usedeftemp(copy._usedeftemp),
    _ptemp(copy._ptemp),
    _estimatedMaxNumOfParticles(0)
{
}

void osgParticle::Emitter::setParticleSystem(ParticleSystem* ps)
{
    if (_ps==ps) return;

    ParticleProcessor::setParticleSystem(ps);

    if (_ps) _ps->adjustEstimatedMaxNumOfParticles(_estimatedMaxNumOfParticles);
}

void osgParticle::Emitter::setEstimatedMaxNumOfParticles(int num)
{
    if (_ps) _ps->adjustEstimatedMaxNumOfParticles(num-_estimatedMaxNumOfParticles);

    _estimatedMaxNumOfParticles = num;
}
