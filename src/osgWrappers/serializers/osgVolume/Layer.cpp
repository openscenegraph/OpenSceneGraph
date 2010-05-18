#include <osgVolume/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define FILTER_FUNCTIONS( PROP ) \
    static bool check##PROP( const osgVolume::Layer& ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osgVolume::Layer& layer ) { \
        DEF_GLENUM(mode); is >> mode; \
        layer.set##PROP( (osg::Texture::FilterMode)mode.get() ); \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osgVolume::Layer& layer ) { \
        os << GLENUM(layer.get##PROP()) << std::endl; \
        return true; \
    }

FILTER_FUNCTIONS( MinFilter )
FILTER_FUNCTIONS( MagFilter )

REGISTER_OBJECT_WRAPPER( osgVolume_Layer,
                         new osgVolume::Layer,
                         osgVolume::Layer,
                         "osg::Object osgVolume::Layer" )
{
    ADD_STRING_SERIALIZER( FileName, "" );  // _filename
    ADD_OBJECT_SERIALIZER( Locator, osgVolume::Locator, NULL );  // _locator
    ADD_VEC4_SERIALIZER( DefaultValue, osg::Vec4() );  // _defaultValue
    ADD_USER_SERIALIZER( MinFilter );  // _minFilter
    ADD_USER_SERIALIZER( MagFilter );  // _magFilter
    ADD_OBJECT_SERIALIZER( Property, osgVolume::Property, NULL );  // _property
}
