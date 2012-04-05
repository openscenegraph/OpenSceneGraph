#include <osgParticle/MultiSegmentPlacer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkVertices( const osgParticle::MultiSegmentPlacer& placer )
{
    return placer.numVertices()>0;
}

static bool readVertices( osgDB::InputStream& is, osgParticle::MultiSegmentPlacer& placer )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Vec3d vec; is >> vec;
        placer.addVertex( vec );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeVertices( osgDB::OutputStream& os, const osgParticle::MultiSegmentPlacer& placer )
{
    unsigned int size = placer.numVertices();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << osg::Vec3d(placer.getVertex(i));
    }
    os << std::endl;
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleMultiSegmentPlacer,
                         new osgParticle::MultiSegmentPlacer,
                         osgParticle::MultiSegmentPlacer,
                         "osg::Object osgParticle::Placer osgParticle::MultiSegmentPlacer" )
{
    ADD_USER_SERIALIZER( Vertices );  // _vx
}
