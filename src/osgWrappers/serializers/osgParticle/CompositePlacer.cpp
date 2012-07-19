#include <osgParticle/CompositePlacer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkPlacers( const osgParticle::CompositePlacer& cp )
{
    return cp.getNumPlacers()>0;
}

static bool readPlacers( osgDB::InputStream& is, osgParticle::CompositePlacer& cp )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgParticle::Placer* p = dynamic_cast<osgParticle::Placer*>( is.readObject() );
        if ( p ) cp.addPlacer( p );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writePlacers( osgDB::OutputStream& os, const osgParticle::CompositePlacer& cp )
{
    unsigned int size = cp.getNumPlacers();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << cp.getPlacer(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleCompositePlacer,
                         new osgParticle::CompositePlacer,
                         osgParticle::CompositePlacer,
                         "osg::Object osgParticle::Placer osgParticle::CompositePlacer" )
{
    ADD_USER_SERIALIZER( Placers );  // _placers
}
