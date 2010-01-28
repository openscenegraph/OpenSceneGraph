#include <osgParticle/Program>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleProgram,
                         /*new osgParticle::Program*/NULL,
                         osgParticle::Program,
                         "osg::Object osg::Node osgParticle::ParticleProcessor osgParticle::Program" )
{
}
