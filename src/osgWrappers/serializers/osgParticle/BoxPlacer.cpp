#include <osgParticle/BoxPlacer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define BOXPLACER_FUNCTION( PROP ) \
    static bool check##PROP( const osgParticle::BoxPlacer& obj ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osgParticle::BoxPlacer& obj ) { \
        float min, max; is >> min >> max; \
        obj.set##PROP( min, max ); return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osgParticle::BoxPlacer& obj ) { \
        const osgParticle::rangef& range = obj.get##PROP(); \
        os << range.minimum << range.maximum << std::endl; \
        return true; \
    }

BOXPLACER_FUNCTION( XRange )
BOXPLACER_FUNCTION( YRange )
BOXPLACER_FUNCTION( ZRange )

REGISTER_OBJECT_WRAPPER( osgParticleBoxPlacer,
                         new osgParticle::BoxPlacer,
                         osgParticle::BoxPlacer,
                         "osg::Object osgParticle::Placer osgParticle::CenteredPlacer osgParticle::BoxPlacer" )
{
    ADD_USER_SERIALIZER( XRange );  // _x_range
    ADD_USER_SERIALIZER( YRange );  // _y_range
    ADD_USER_SERIALIZER( ZRange );  // _z_range
}
