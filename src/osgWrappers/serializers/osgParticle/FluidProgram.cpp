#include <osgParticle/FluidProgram>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleFluidProgram,
                         new osgParticle::FluidProgram,
                         osgParticle::FluidProgram,
                         "osg::Object osg::Node osgParticle::ParticleProcessor osgParticle::Program osgParticle::FluidProgram" )
{
    ADD_VEC3_SERIALIZER( Acceleration, osg::Vec3() );  // _acceleration
    ADD_FLOAT_SERIALIZER( FluidViscosity, 0.0f );  // _viscosity
    ADD_FLOAT_SERIALIZER( FluidDensity, 0.0f );  // _density
    ADD_VEC3_SERIALIZER( Wind, osg::Vec3() );  // _wind
}
