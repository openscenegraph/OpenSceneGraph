#include <osgParticle/FluidFrictionOperator>
#include <osgParticle/ModularProgram>
#include <osgParticle/Operator>
#include <osgParticle/Particle>
#include <osg/Notify>

osgParticle::FluidFrictionOperator::FluidFrictionOperator()
: Operator(), ovr_rad_(0)
{
    setFluidToAir();
}

osgParticle::FluidFrictionOperator::FluidFrictionOperator(const FluidFrictionOperator &copy, const osg::CopyOp &copyop)
:    Operator(copy, copyop),
    A_(copy.A_),
    B_(copy.B_),
    density_(copy.density_), 
    viscosity_(copy.viscosity_),
    ovr_rad_(copy.ovr_rad_)
{
}

void osgParticle::FluidFrictionOperator::operate(Particle *P, double dt)
{
    float r = (ovr_rad_ > 0)? ovr_rad_ : P->getRadius();
    osg::Vec3 v = P->getVelocity();

    float vm = v.normalize();
    float R = A_ * r * vm + B_ * r * r * vm * vm;
    
    osg::Vec3 Fr(-R * v.x(), -R * v.y(), -R * v.z());

    if (current_program_->getReferenceFrame() == ModularProgram::RELATIVE_TO_PARENTS) {
        Fr = current_program_->rotateLocalToWorld(Fr);
    }

    P->addVelocity(Fr * (P->getMassInv() * dt));
}
