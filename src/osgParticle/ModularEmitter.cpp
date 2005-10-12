#include <osgParticle/ModularEmitter>
#include <osgParticle/Emitter>
#include <osgParticle/ConnectedParticleSystem>
#include <osg/Notify>

osgParticle::ModularEmitter::ModularEmitter()
:    Emitter(),
    _numParticleToCreateMovementCompensationRatio(0.0f),
    _counter(new RandomRateCounter),
    _placer(new PointPlacer),
    _shooter(new RadialShooter)
{
}
    
osgParticle::ModularEmitter::ModularEmitter(const ModularEmitter& copy, const osg::CopyOp& copyop):
    Emitter(copy, copyop),
    _numParticleToCreateMovementCompensationRatio(copy._numParticleToCreateMovementCompensationRatio),
    _counter(static_cast<Counter *>(copyop(copy._counter.get()))), 
    _placer(static_cast<Placer *>(copyop(copy._placer.get()))), 
    _shooter(static_cast<Shooter *>(copyop(copy._shooter.get())))
{
}

void osgParticle::ModularEmitter::emit(double dt) 
{
    ConnectedParticleSystem* cps = dynamic_cast<ConnectedParticleSystem*>(getParticleSystem());

    if (getReferenceFrame() == RELATIVE_RF)
    {
        const osg::Matrix& ltw = getLocalToWorldMatrix();
        const osg::Matrix& previous_ltw = getPreviousLocalToWorldMatrix();

        int n = _counter->numParticlesToCreate(dt);

        if (_numParticleToCreateMovementCompensationRatio>0.0f)
        {
            // compute the distance moved between frames
            const osg::Vec3 controlPosition = _placer->getControlPosition();
            osg::Vec3 previousPosition = controlPosition * previous_ltw;
            osg::Vec3 currentPosition = controlPosition * ltw;
            float distance = (currentPosition-previousPosition).length();

            float size = getUseDefaultTemplate() ? 
                        getParticleSystem()->getDefaultParticleTemplate().getSizeRange().minimum :
                        getParticleTemplate().getSizeRange().minimum;

            float num_extra_samples = _numParticleToCreateMovementCompensationRatio*distance/size;
            float rounded_down = floor(num_extra_samples);
            float remainder = num_extra_samples-rounded_down;

            n = osg::maximum(n, int(rounded_down) +  (((float) rand() < remainder * (float)RAND_MAX) ? 1 : 0));
        }
        
        for (int i=0; i<n; ++i)
        {
            Particle* P = getParticleSystem()->createParticle(getUseDefaultTemplate()? 0: &getParticleTemplate());
            if (P)
            {
                _placer->place(P);
                _shooter->shoot(P);
                
                // now need to transform the position and velocity because we having a moving model.
                float r = ((float)rand()/(float)RAND_MAX);
                P->transformPositionVelocity(ltw, previous_ltw, r);
                //P->transformPositionVelocity(ltw);
                
                if (cps) P->setUpTexCoordsAsPartOfConnectedParticleSystem(cps);
                
            }
            else
            {
                osg::notify(osg::NOTICE)<<"run out of particle"<<std::endl;
            }
        }
    }
    else
    {
        int n = _counter->numParticlesToCreate(dt);
        for (int i=0; i<n; ++i)
        {
            Particle* P = getParticleSystem()->createParticle(getUseDefaultTemplate()? 0: &getParticleTemplate());
            if (P)
            {
                _placer->place(P);
                _shooter->shoot(P);

                if (cps) P->setUpTexCoordsAsPartOfConnectedParticleSystem(cps);
            }
        }
    }
}
