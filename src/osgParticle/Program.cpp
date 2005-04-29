#include <osgParticle/Program>
#include <osgParticle/ParticleProcessor>

#include <osg/CopyOp>

osgParticle::Program::Program()
:    ParticleProcessor()
{
}

osgParticle::Program::Program(const Program& copy, const osg::CopyOp& copyop)
:    ParticleProcessor(copy, copyop)
{
}
