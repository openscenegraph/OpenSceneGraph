#include <osgParticle/Emitter>
#include <osgParticle/ParticleProcessor>

#include <osg/CopyOp>

osgParticle::Emitter::Emitter()
:    ParticleProcessor(), 
    usedeftemp_(true)
{
}

osgParticle::Emitter::Emitter(const Emitter &copy, const osg::CopyOp &copyop)
:     ParticleProcessor(copy, copyop),
    usedeftemp_(copy.usedeftemp_), 
    ptemp_(copy.ptemp_)
{
}
