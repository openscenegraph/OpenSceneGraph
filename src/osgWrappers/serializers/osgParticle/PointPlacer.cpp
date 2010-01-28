#include <osgParticle/PointPlacer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticlePointPlacer,
                         new osgParticle::PointPlacer,
                         osgParticle::PointPlacer,
                         "osg::Object osgParticle::Placer osgParticle::CenteredPlacer osgParticle::PointPlacer" )
{
}
