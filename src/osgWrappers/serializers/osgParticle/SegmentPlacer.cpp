#include <osgParticle/SegmentPlacer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleSegmentPlacer,
                         new osgParticle::SegmentPlacer,
                         osgParticle::SegmentPlacer,
                         "osg::Object osgParticle::Placer osgParticle::SegmentPlacer" )
{
    ADD_VEC3_SERIALIZER( VertexA, osg::Vec3() );  // _vertexA
    ADD_VEC3_SERIALIZER( VertexB, osg::Vec3() );  // _vertexB
}
