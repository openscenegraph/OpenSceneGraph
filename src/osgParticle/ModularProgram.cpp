#include <osgParticle/ModularProgram>
#include <osgParticle/Program>
#include <osgParticle/ParticleSystem>
#include <osgParticle/Particle>

osgParticle::ModularProgram::ModularProgram()
: Program()
{
}

osgParticle::ModularProgram::ModularProgram(const ModularProgram& copy, const osg::CopyOp& copyop)
: Program(copy, copyop)
{
    Operator_vector::const_iterator ci;
    for (ci=copy._operators.begin(); ci!=copy._operators.end(); ++ci) {
        _operators.push_back(static_cast<Operator* >(copyop(ci->get())));
    }
}

void osgParticle::ModularProgram::execute(double dt)
{
    Operator_vector::iterator ci;
    Operator_vector::iterator ci_end = _operators.end();

    ParticleSystem* ps = getParticleSystem();
    for (ci=_operators.begin(); ci!=ci_end; ++ci) {
        (*ci)->beginOperate(this);
        (*ci)->operateParticles(ps, dt);
        (*ci)->endOperate();
    }
}
