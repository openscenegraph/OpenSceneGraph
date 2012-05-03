#include <osgParticle/FluidProgram>

osgParticle::FluidProgram::FluidProgram():
    Program()
{
    setFluidToAir();
}

osgParticle::FluidProgram::FluidProgram(const FluidProgram& copy, const osg::CopyOp& copyop):
    Program(copy, copyop),
    _acceleration(copy._acceleration),
    _viscosity(copy._viscosity),
    _density(copy._density),
    _wind(copy._wind),
    _viscosityCoefficient(copy._viscosityCoefficient),
    _densityCoefficient(copy._densityCoefficient)
{
}

void osgParticle::FluidProgram::execute(double dt)
{
    const float four_over_three = 4.0f/3.0f;
    ParticleSystem* ps = getParticleSystem();
    int n = ps->numParticles();
    for (int i=0; i<n; ++i)
    {
        Particle* particle = ps->getParticle(i);
        if (particle->isAlive())
        {
            float radius = particle->getRadius();
            float Area = osg::PI*radius*radius;
            float Volume = Area*radius*four_over_three;

            // compute force due to gravity + boyancy of displacing the fluid that the particle is emersed in.
            osg::Vec3 accel_gravity = _acceleration * ((particle->getMass() - _density*Volume) * particle->getMassInv());

            // compute force due to friction
            osg::Vec3 relative_wind = particle->getVelocity()-_wind;
            osg::Vec3 wind_force = - relative_wind * Area * (_viscosityCoefficient + _densityCoefficient*relative_wind.length());
            osg::Vec3 wind_accel = wind_force * particle->getMassInv();

            double compenstated_dt = dt;
            if (relative_wind.length2() < dt*dt*wind_accel.length2())
            {
                // OSG_NOTICE<<"** Could be critical: dt="<<dt<<" critical_dt="<<sqrtf(critical_dt2)<<std::endl;
                double critical_dt2 = relative_wind.length2()/wind_accel.length2();
                compenstated_dt = sqrtf(critical_dt2)*0.8f;
            }

            particle->addVelocity(accel_gravity*dt + wind_accel*compenstated_dt);


        }
    }
}
