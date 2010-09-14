#include <osgParticle/AngularDampingOperator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleAngularDampingOperator,
                         new osgParticle::AngularDampingOperator,
                         osgParticle::AngularDampingOperator,
                         "osg::Object osgParticle::Operator osgParticle::AngularDampingOperator" )
{
    ADD_VEC3_SERIALIZER( Damping, osg::Vec3() );  // _damping
    ADD_FLOAT_SERIALIZER( CutoffLow, 0.0f );  // _cutoffLow
    ADD_FLOAT_SERIALIZER( CutoffHigh, FLT_MAX );  // _cutoffHigh
}
