#include <osg/TexGen>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define PLANE_FUNCTION( PROP, COORD ) \
    static bool check##PROP( const osg::TexGen& tex ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osg::TexGen& tex ) { \
        osg::Plane plane; is >> plane; \
        tex.setPlane(COORD, plane); \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::TexGen& tex ) { \
        os << tex.getPlane(COORD) << std::endl; \
        return true; \
    }

PLANE_FUNCTION( PlaneS, osg::TexGen::S )
PLANE_FUNCTION( PlaneT, osg::TexGen::T )
PLANE_FUNCTION( PlaneR, osg::TexGen::R )
PLANE_FUNCTION( PlaneQ, osg::TexGen::Q )

REGISTER_OBJECT_WRAPPER( TexGen,
                         new osg::TexGen,
                         osg::TexGen,
                         "osg::Object osg::StateAttribute osg::TexGen" )
{
    BEGIN_ENUM_SERIALIZER( Mode, OBJECT_LINEAR );
        ADD_ENUM_VALUE( OBJECT_LINEAR );
        ADD_ENUM_VALUE( EYE_LINEAR );
        ADD_ENUM_VALUE( SPHERE_MAP );
        ADD_ENUM_VALUE( NORMAL_MAP );
        ADD_ENUM_VALUE( REFLECTION_MAP );
    END_ENUM_SERIALIZER();  // _mode

    ADD_USER_SERIALIZER( PlaneS );
    ADD_USER_SERIALIZER( PlaneT );
    ADD_USER_SERIALIZER( PlaneR );
    ADD_USER_SERIALIZER( PlaneQ );  //_plane_s, _plane_t, _plane_r, _plane_q
}
