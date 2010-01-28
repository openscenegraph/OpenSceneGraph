#include <osgParticle/ParticleSystem>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

extern bool readParticle( osgDB::InputStream& is, osgParticle::Particle& p );
extern bool writeParticle( osgDB::OutputStream& os, const osgParticle::Particle& p );

// _def_bbox
static bool checkDefaultBoundingBox( const osgParticle::ParticleSystem& ps )
{
    return ps.getDefaultBoundingBox().valid();
}

static bool readDefaultBoundingBox( osgDB::InputStream& is, osgParticle::ParticleSystem& ps )
{
    osg::Vec3d min, max;
    is >> osgDB::BEGIN_BRACKET;
    is >> osgDB::PROPERTY("Minimum") >> min;
    is >> osgDB::PROPERTY("Maximum") >> max;
    is >> osgDB::END_BRACKET;
    ps.setDefaultBoundingBox( osg::BoundingBox(min, max) );
    return true;
}

static bool writeDefaultBoundingBox( osgDB::OutputStream& os, const osgParticle::ParticleSystem& ps )
{
    const osg::BoundingBox& bb = ps.getDefaultBoundingBox();
    os << osgDB::BEGIN_BRACKET << std::endl;
    os << osgDB::PROPERTY("Minimum") << osg::Vec3d(bb._min) << std::endl;
    os << osgDB::PROPERTY("Maximum") << osg::Vec3d(bb._max) << std::endl;
    os << osgDB::END_BRACKET;
    os << std::endl;
    return true;
}

// _defaultParticleTemplate
static bool checkDefaultParticleTemplate( const osgParticle::ParticleSystem& ps )
{
    return true;
}

static bool readDefaultParticleTemplate( osgDB::InputStream& is, osgParticle::ParticleSystem& ps )
{
    osgParticle::Particle p;
    readParticle( is, p );
    ps.setDefaultParticleTemplate( p );
    return true;
}

static bool writeDefaultParticleTemplate( osgDB::OutputStream& os, const osgParticle::ParticleSystem& ps )
{
    const osgParticle::Particle& p = ps.getDefaultParticleTemplate();
    writeParticle( os, p );
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleParticleSystem,
                         new osgParticle::ParticleSystem,
                         osgParticle::ParticleSystem,
                         "osg::Object osg::Drawable osgParticle::ParticleSystem" )
{
    ADD_USER_SERIALIZER( DefaultBoundingBox );  // _def_bbox
    
    BEGIN_ENUM_SERIALIZER2( ParticleAlignment, osgParticle::ParticleSystem::Alignment, BILLBOARD );
        ADD_ENUM_VALUE( BILLBOARD );
        ADD_ENUM_VALUE( FIXED );
    END_ENUM_SERIALIZER();  // _alignment
    
    ADD_VEC3_SERIALIZER( AlignVectorX, osg::Vec3() );  // _align_X_axis
    ADD_VEC3_SERIALIZER( AlignVectorY, osg::Vec3() );  // _align_Y_axis
    
    BEGIN_ENUM_SERIALIZER( ParticleScaleReferenceFrame, WORLD_COORDINATES );
        ADD_ENUM_VALUE( LOCAL_COORDINATES );
        ADD_ENUM_VALUE( WORLD_COORDINATES );
    END_ENUM_SERIALIZER();  // _particleScaleReferenceFrame
    
    ADD_BOOL_SERIALIZER( DoublePassRendering, false );  // _doublepass
    ADD_BOOL_SERIALIZER( Frozen, false );  // _frozen
    ADD_USER_SERIALIZER( DefaultParticleTemplate );  // _def_ptemp
    ADD_BOOL_SERIALIZER( FreezeOnCull, false );  // _freeze_on_cull
}
