#include <osgParticle/CenteredPlacer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleCenteredPlacer,
                         /*new osgParticle::CenteredPlacer*/NULL,
                         osgParticle::CenteredPlacer,
                         "osg::Object osgParticle::Placer osgParticle::CenteredPlacer" )
{
    ADD_VEC3_SERIALIZER( Center, osg::Vec3() );  // center_
}
