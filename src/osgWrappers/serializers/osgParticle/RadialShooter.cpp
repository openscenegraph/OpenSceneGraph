#include <osgParticle/RadialShooter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define RADIALSHOOTER_FLOAT_FUNCTION( PROP ) \
    static bool check##PROP( const osgParticle::RadialShooter& obj ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osgParticle::RadialShooter& obj ) { \
        float min, max; is >> min >> max; \
        obj.set##PROP( min, max ); return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osgParticle::RadialShooter& obj ) { \
        const osgParticle::rangef& range = obj.get##PROP(); \
        os << range.minimum << range.maximum << std::endl; \
        return true; \
    }

#define RADIALSHOOTER_VEC3_FUNCTION( PROP ) \
    static bool check##PROP( const osgParticle::RadialShooter& obj ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osgParticle::RadialShooter& obj ) { \
        osg::Vec3d min, max; is >> min >> max; \
        obj.set##PROP( min, max ); return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osgParticle::RadialShooter& obj ) { \
        const osgParticle::rangev3& range = obj.get##PROP(); \
        os << osg::Vec3d(range.minimum) << osg::Vec3d(range.maximum) << std::endl; \
        return true; \
    }

RADIALSHOOTER_FLOAT_FUNCTION( ThetaRange )
RADIALSHOOTER_FLOAT_FUNCTION( PhiRange )
RADIALSHOOTER_FLOAT_FUNCTION( InitialSpeedRange )
RADIALSHOOTER_VEC3_FUNCTION( InitialRotationalSpeedRange )

REGISTER_OBJECT_WRAPPER( osgParticleRadialShooter,
                         new osgParticle::RadialShooter,
                         osgParticle::RadialShooter,
                         "osg::Object osgParticle::Shooter osgParticle::RadialShooter" )
{
    ADD_USER_SERIALIZER( ThetaRange );  // _theta_range
    ADD_USER_SERIALIZER( PhiRange );  // _phi_range
    ADD_USER_SERIALIZER( InitialSpeedRange );  // _speed_range
    ADD_USER_SERIALIZER( InitialRotationalSpeedRange );  // _rot_speed_range
}
