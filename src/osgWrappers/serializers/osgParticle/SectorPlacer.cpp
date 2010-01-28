#include <osgParticle/SectorPlacer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define SECTORPLACER_FUNCTION( PROP ) \
    static bool check##PROP( const osgParticle::SectorPlacer& obj ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osgParticle::SectorPlacer& obj ) { \
        float min, max; is >> min >> max; \
        obj.set##PROP( min, max ); return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osgParticle::SectorPlacer& obj ) { \
        const osgParticle::rangef& range = obj.get##PROP(); \
        os << range.minimum << range.maximum << std::endl; \
        return true; \
    }

SECTORPLACER_FUNCTION( RadiusRange )
SECTORPLACER_FUNCTION( PhiRange )

REGISTER_OBJECT_WRAPPER( osgParticleSectorPlacer,
                         new osgParticle::SectorPlacer,
                         osgParticle::SectorPlacer,
                         "osg::Object osgParticle::Placer osgParticle::CenteredPlacer osgParticle::SectorPlacer" )
{
    ADD_USER_SERIALIZER( RadiusRange );  // _rad_range
    ADD_USER_SERIALIZER( PhiRange );  // _phi_range
}
