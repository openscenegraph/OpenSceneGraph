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
    _A(copy._A),
    _B(copy._B)
{
}

void osgParticle::FluidProgram::execute(double dt)
{
    const float four_over_three = 4.0f/3.0f;
    ParticleSystem *ps = getParticleSystem();
    int n = ps->numParticles();
    for (int i=0; i<n; ++i)
    {
        Particle *particle = ps->getParticle(i);
        if (particle->isAlive())
        {
            float radius = particle->getRadius();
            float Area = osg::PI*radius*radius;
            float Volume = Area*radius*four_over_three;
        
            // compute force due to gravity + boyancy of displacing the fluid that the particle is emersed in.
            osg::Vec3 force = _acceleration * (particle->getMass() - _density*Volume);

            // compute force due to friction
            osg::Vec3 relative_wind = particle->getVelocity()-_wind;            
            force -= relative_wind * Area * (_A + _B*relative_wind.length());            
            
            // divide force by mass to get acceleration.
            force *= particle->getMassInv()*dt;
            
            particle->addVelocity(force);
        }
    }
}
