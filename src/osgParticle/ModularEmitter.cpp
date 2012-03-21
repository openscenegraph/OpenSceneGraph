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

void osgParticle::ModularEmitter::emitParticles(double dt)
{
    ConnectedParticleSystem* cps = dynamic_cast<ConnectedParticleSystem*>(getParticleSystem());

    osg::Matrix worldToPs;
    osg::MatrixList worldMats = getParticleSystem()->getWorldMatrices();
    if (!worldMats.empty())
    {
        const osg::Matrix psToWorld = worldMats[0];
        worldToPs = osg::Matrix::inverse(psToWorld);
    }

    if (getReferenceFrame() == RELATIVE_RF)
    {
        const osg::Matrix& ltw = getLocalToWorldMatrix();
        const osg::Matrix& previous_ltw = getPreviousLocalToWorldMatrix();
        const osg::Matrix emitterToPs = ltw * worldToPs;
        const osg::Matrix prevEmitterToPs = previous_ltw * worldToPs;

        int n = _counter->numParticlesToCreate(dt);

        if (_numParticleToCreateMovementCompensationRatio>0.0f)
        {
            // compute the distance moved between frames
            const osg::Vec3d controlPosition
                = osg::Vec3d(_placer->getControlPosition());
            osg::Vec3d previousPosition = controlPosition * previous_ltw;
            osg::Vec3d currentPosition = controlPosition * ltw;
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

                // Now need to transform the position and velocity because we having a moving model.
                float r = ((float)rand()/(float)RAND_MAX);
                P->transformPositionVelocity(emitterToPs, prevEmitterToPs, r);
                //P->transformPositionVelocity(ltw);

                if (cps) P->setUpTexCoordsAsPartOfConnectedParticleSystem(cps);

            }
            else
            {
                OSG_NOTICE<<"run out of particle"<<std::endl;
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
                P->setPosition(P->getPosition() * worldToPs);
                _shooter->shoot(P);
                P->setVelocity(osg::Matrix::transform3x3(P->getVelocity(),
                                                         worldToPs));

                if (cps) P->setUpTexCoordsAsPartOfConnectedParticleSystem(cps);
            }
        }
    }
}
