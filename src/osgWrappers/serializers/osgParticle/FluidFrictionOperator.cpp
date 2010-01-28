#include <osgParticle/FluidFrictionOperator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleFluidFrictionOperator,
                         new osgParticle::FluidFrictionOperator,
                         osgParticle::FluidFrictionOperator,
                         "osg::Object osgParticle::Operator osgParticle::FluidFrictionOperator" )
{
    ADD_FLOAT_SERIALIZER( FluidDensity, 0.0f );  // _density
    ADD_FLOAT_SERIALIZER( FluidDensity, 0.0f );  // _viscosity
    ADD_VEC3_SERIALIZER( Wind, osg::Vec3() );  // _wind
    ADD_FLOAT_SERIALIZER( OverrideRadius, 0.0f );  // _ovr_rad
}
