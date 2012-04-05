#include <osg/Material>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define MATERIAL_FUNC( PROP, TYPE ) \
    static bool check##PROP( const osg::Material& attr ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osg::Material& attr ) { \
        bool frontAndBack; TYPE value1, value2; \
        is >> frontAndBack; \
        is >> is.PROPERTY("Front") >> value1; \
        is >> is.PROPERTY("Back") >> value2; \
        if ( frontAndBack ) \
            attr.set##PROP(osg::Material::FRONT_AND_BACK, value1); \
        else { \
            attr.set##PROP(osg::Material::FRONT, value1); \
            attr.set##PROP(osg::Material::BACK, value2); \
        } \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::Material& attr ) { \
        os << attr.get##PROP##FrontAndBack(); \
        os << os.PROPERTY("Front") << TYPE(attr.get##PROP(osg::Material::FRONT)); \
        os << os.PROPERTY("Back") << TYPE(attr.get##PROP(osg::Material::BACK)) << std::endl; \
        return true; \
    }

MATERIAL_FUNC( Ambient, osg::Vec4f )
MATERIAL_FUNC( Diffuse, osg::Vec4f )
MATERIAL_FUNC( Specular, osg::Vec4f )
MATERIAL_FUNC( Emission, osg::Vec4f )
MATERIAL_FUNC( Shininess, float )

REGISTER_OBJECT_WRAPPER( Material,
                         new osg::Material,
                         osg::Material,
                         "osg::Object osg::StateAttribute osg::Material" )
{
    BEGIN_ENUM_SERIALIZER( ColorMode, OFF );
        ADD_ENUM_VALUE( AMBIENT );
        ADD_ENUM_VALUE( DIFFUSE );
        ADD_ENUM_VALUE( SPECULAR );
        ADD_ENUM_VALUE( EMISSION );
        ADD_ENUM_VALUE( AMBIENT_AND_DIFFUSE );
        ADD_ENUM_VALUE( OFF );
    END_ENUM_SERIALIZER();  // _colorMode

    ADD_USER_SERIALIZER( Ambient );  // _ambient
    ADD_USER_SERIALIZER( Diffuse );  // _diffuse
    ADD_USER_SERIALIZER( Specular );  // _specular
    ADD_USER_SERIALIZER( Emission );  // _emission
    ADD_USER_SERIALIZER( Shininess );  // _shininess
}
