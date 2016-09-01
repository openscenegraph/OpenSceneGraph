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

    const Particle& particleTemplate = getUseDefaultTemplate() ? getParticleSystem()->getDefaultParticleTemplate() : getParticleTemplate();

    double duration = particleTemplate.getLifeTime();
    if (!getEndless() && getLifeTime()<duration) duration = getLifeTime();
    // duration += 5.0f/60.0f;

    unsigned int num_before_end_of_lifetime = _counter->getEstimatedMaxNumOfParticles(duration);
    float esimateMaxNumScale = 1.1f;

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
            const osg::Vec3d controlPosition = osg::Vec3d(_placer->getControlPosition());
            osg::Vec3d previousPosition = controlPosition * previous_ltw;
            osg::Vec3d currentPosition = controlPosition * ltw;
            float distance = (currentPosition-previousPosition).length();

            float size = particleTemplate.getSizeRange().minimum;

            float num_extra_samples = _numParticleToCreateMovementCompensationRatio*distance/size;
            float rounded_down = floor(num_extra_samples);
            float remainder = num_extra_samples-rounded_down;

            n = osg::maximum(n, int(rounded_down) +  (((float) rand() < remainder * (float)RAND_MAX) ? 1 : 0));

            unsigned int num_for_duration = static_cast<unsigned int>((num_extra_samples/dt) * duration);
            if (num_for_duration>num_before_end_of_lifetime)
            {
                num_before_end_of_lifetime = num_for_duration;
            }


        }

        num_before_end_of_lifetime = static_cast<unsigned int>( ceilf(static_cast<float>(num_before_end_of_lifetime) * esimateMaxNumScale));

        setEstimatedMaxNumOfParticles(num_before_end_of_lifetime);


        // double num_per_second = (static_cast<double>(n)/dt);
        // OSG_NOTICE<<"emitParticle count="<<_counter->className()<<" ps="<<std::hex<<getParticleSystem()<<std::dec<<", n="<<n<<", num_per_second="<<num_per_second<<", num_before_end_of_lifetime="<<num_before_end_of_lifetime<<std::endl;
        // if (getEndless()) { OSG_NOTICE<<"    Emitter::getLifeTime()=ENDLESS particle.getLifeTime()="<<particleTemplate.getLifeTime()<<" duration="<<duration<<std::endl; }
        // else { OSG_NOTICE<<"    Emitter::getLifeTime()="<<getLifeTime()<<" particle.getLifeTime()="<<particleTemplate.getLifeTime()<<" duration="<<duration<<std::endl; }

        for (int i=0; i<n; ++i)
        {
            Particle* P = getParticleSystem()->createParticle(&particleTemplate);
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

        num_before_end_of_lifetime = static_cast<unsigned int>( ceilf(static_cast<float>(num_before_end_of_lifetime) * esimateMaxNumScale));

        setEstimatedMaxNumOfParticles(num_before_end_of_lifetime);

        // double num_per_second = (static_cast<double>(n)/dt);
        // OSG_NOTICE<<"emitParticle ps="<<std::hex<<getParticleSystem()<<std::dec<<", num_per_second="<<num_per_second<<", num_before_end_of_lifetime="<<num_before_end_of_lifetime<<std::endl;
        // if (getEndless()) { OSG_NOTICE<<"    Emitter::getLifeTime()=ENDLESS particle.getLifeTime()="<<particleTemplate.getLifeTime()<<" duration="<<duration<<std::endl; }
        // else { OSG_NOTICE<<"    Emitter::getLifeTime()="<<getLifeTime()<<" particle.getLifeTime()="<<particleTemplate.getLifeTime()<<" duration="<<duration<<std::endl; }

        for (int i=0; i<n; ++i)
        {
            Particle* P = getParticleSystem()->createParticle(&particleTemplate);
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
