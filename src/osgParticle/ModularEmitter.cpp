#include <osgParticle/ModularEmitter>
#include <osgParticle/Emitter>

osgParticle::ModularEmitter::ModularEmitter()
:    Emitter(),
    _counter(new RandomRateCounter),
    _placer(new PointPlacer),
    _shooter(new RadialShooter)
{
}
    
osgParticle::ModularEmitter::ModularEmitter(const ModularEmitter& copy, const osg::CopyOp& copyop)
:    Emitter(copy, copyop),
    _counter(static_cast<Counter *>(copyop(copy._counter.get()))), 
    _placer(static_cast<Placer *>(copyop(copy._placer.get()))), 
    _shooter(static_cast<Shooter *>(copyop(copy._shooter.get())))
{
}

void osgParticle::ModularEmitter::emit(double dt) 
{
    int n = _counter->numParticlesToCreate(dt);
    for (int i=0; i<n; ++i) {
        Particle* P = getParticleSystem()->createParticle(getUseDefaultTemplate()? 0: &getParticleTemplate());
        if (P) {
            _placer->place(P);
            _shooter->shoot(P);
            if (getReferenceFrame() == RELATIVE_RF) {
                P->transformPositionVelocity(getLocalToWorldMatrix());
            }
        }
    }
}
