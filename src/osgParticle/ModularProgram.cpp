#include <osgParticle/ModularProgram>
#include <osgParticle/Program>
#include <osgParticle/ParticleSystem>
#include <osgParticle/Particle>

osgParticle::ModularProgram::ModularProgram()
: Program()
{
}

osgParticle::ModularProgram::ModularProgram(const ModularProgram &copy, const osg::CopyOp &copyop)
: Program(copy, copyop)
{
    Operator_vector::const_iterator ci;
    for (ci=copy.operators_.begin(); ci!=copy.operators_.end(); ++ci) {
        operators_.push_back(static_cast<Operator *>(copyop(ci->get())));
    }
}

void osgParticle::ModularProgram::execute(double dt)
{
    Operator_vector::iterator ci;
    Operator_vector::iterator ci_end = operators_.end();

    ParticleSystem *ps = getParticleSystem();
    for (ci=operators_.begin(); ci!=ci_end; ++ci) {        
        (*ci)->beginOperate(this);
        int n = ps->numParticles();
        for (int i=0; i<n; ++i) {
            Particle *P = ps->getParticle(i);
            if (P->isAlive() && (*ci)->isEnabled()) {
                (*ci)->operate(P, dt);
            }
        }
        (*ci)->endOperate();
    }
}
