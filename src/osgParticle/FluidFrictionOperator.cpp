#include <osgParticle/FluidFrictionOperator>
#include <osgParticle/ModularProgram>
#include <osgParticle/Operator>
#include <osgParticle/Particle>
#include <osg/Notify>

osgParticle::FluidFrictionOperator::FluidFrictionOperator():
     Operator(),
     A_(0),
     B_(0),
     density_(0),
     viscosity_(0),
     ovr_rad_(0),
     current_program_(0)
{
    setFluidToAir();
}

osgParticle::FluidFrictionOperator::FluidFrictionOperator(const FluidFrictionOperator &copy, const osg::CopyOp &copyop)
:    Operator(copy, copyop),
    A_(copy.A_),
    B_(copy.B_),
    density_(copy.density_), 
    viscosity_(copy.viscosity_),
    ovr_rad_(copy.ovr_rad_),
    current_program_(0)
{
}

void osgParticle::FluidFrictionOperator::operate(Particle *P, double)
{
    float r = (ovr_rad_ > 0)? ovr_rad_ : P->getRadius();
    osg::Vec3 v = P->getVelocity();

    float vm = v.normalize();
    float R = A_ * r * vm + B_ * r * r * vm * vm;
    
    osg::Vec3 Fr(-R * v.x(), -R * v.y(), -R * v.z());

    if (current_program_->getReferenceFrame() == ModularProgram::RELATIVE_TO_PARENTS) {
        Fr = current_program_->rotateLocalToWorld(Fr);
    }

	// correct unwanted velocity increments
	osg::Vec3 dv = Fr * (P->getMassInv() * 0.01);
	float dvl = dv.length();
	if (dvl > vm) {
		dv *= vm / dvl;
	}

    P->addVelocity(dv);
}
