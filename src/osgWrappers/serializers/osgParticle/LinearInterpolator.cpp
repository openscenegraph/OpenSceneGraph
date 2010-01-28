#include <osgParticle/LinearInterpolator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleLinearInterpolator,
                         new osgParticle::LinearInterpolator,
                         osgParticle::LinearInterpolator,
                         "osg::Object osgParticle::Interpolator osgParticle::LinearInterpolator" )
{
}
