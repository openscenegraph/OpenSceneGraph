#include <osgParticle/Shooter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleShooter,
                         /*new osgParticle::Shooter*/NULL,
                         osgParticle::Shooter,
                         "osg::Object osgParticle::Shooter" )
{
}
