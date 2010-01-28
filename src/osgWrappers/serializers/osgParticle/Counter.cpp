#include <osgParticle/Counter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleCounter,
                         /*new osgParticle::Counter*/NULL,
                         osgParticle::Counter,
                         "osg::Object osgParticle::Counter" )
{
}
