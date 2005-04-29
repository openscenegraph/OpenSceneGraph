#include <osgParticle/Emitter>
#include <osgParticle/ParticleProcessor>

#include <osg/CopyOp>

osgParticle::Emitter::Emitter()
:    ParticleProcessor(), 
    _usedeftemp(true)
{
}

osgParticle::Emitter::Emitter(const Emitter& copy, const osg::CopyOp& copyop)
:     ParticleProcessor(copy, copyop),
    _usedeftemp(copy._usedeftemp), 
    _ptemp(copy._ptemp)
{
}
