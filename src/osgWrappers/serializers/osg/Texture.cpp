#include <osg/Texture>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define WRAP_FUNCTIONS( PROP, VALUE ) \
    static bool check##PROP( const osg::Texture& tex ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osg::Texture& tex ) { \
        DEF_GLENUM(mode); is >> mode; \
        tex.setWrap( VALUE, (osg::Texture::WrapMode)mode.get() ); \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::Texture& tex ) { \
        os << GLENUM(tex.getWrap(VALUE)) << std::endl; \
        return true; \
    }

WRAP_FUNCTIONS( WRAP_S, osg::Texture::WRAP_S )
WRAP_FUNCTIONS( WRAP_T, osg::Texture::WRAP_T )
WRAP_FUNCTIONS( WRAP_R, osg::Texture::WRAP_R )

#define FILTER_FUNCTIONS( PROP, VALUE ) \
    static bool check##PROP( const osg::Texture& tex ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osg::Texture& tex ) { \
        DEF_GLENUM(mode); is >> mode; \
        tex.setFilter( VALUE, (osg::Texture::FilterMode)mode.get() ); \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::Texture& tex ) { \
        os << GLENUM(tex.getFilter(VALUE)) << std::endl; \
        return true; \
    }

FILTER_FUNCTIONS( MIN_FILTER, osg::Texture::MIN_FILTER )
FILTER_FUNCTIONS( MAG_FILTER, osg::Texture::MAG_FILTER )

#define GL_FORMAT_FUNCTIONS( PROP ) \
    static bool check##PROP( const osg::Texture& tex ) { \
        return tex.get##PROP()!=GL_NONE; \
    } \
    static bool read##PROP( osgDB::InputStream& is, osg::Texture& tex ) { \
        DEF_GLENUM(mode); is >> mode; tex.set##PROP( mode.get() ); \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::Texture& tex ) { \
        os << GLENUM(tex.get##PROP()) << std::endl; \
        return true; \
    }

GL_FORMAT_FUNCTIONS( SourceFormat )
GL_FORMAT_FUNCTIONS( SourceType )

static bool checkInternalFormat( const osg::Texture& tex )
{ return tex.getInternalFormatMode()==osg::Texture::USE_USER_DEFINED_FORMAT; }

static bool readInternalFormat( osgDB::InputStream& is, osg::Texture& tex )
{
    DEF_GLENUM(mode); is >> mode;
    if ( tex.getInternalFormatMode()==osg::Texture::USE_USER_DEFINED_FORMAT )
        tex.setInternalFormat( mode.get() );
    return true;
}

static bool writeInternalFormat( osgDB::OutputStream& os, const osg::Texture& tex )
{
    if ( os.isBinary() && tex.getInternalFormatMode()!=osg::Texture::USE_USER_DEFINED_FORMAT )
        os << GLENUM(GL_NONE) << std::endl;  // Avoid use of OpenGL extensions
    else
        os << GLENUM(tex.getInternalFormat()) << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Texture,
                         /*new osg::Texture*/NULL,
                         osg::Texture,
                         "osg::Object osg::StateAttribute osg::Texture" )
{
    ADD_USER_SERIALIZER( WRAP_S );  // _wrap_s
    ADD_USER_SERIALIZER( WRAP_T );  // _wrap_t
    ADD_USER_SERIALIZER( WRAP_R );  // _wrap_r
    ADD_USER_SERIALIZER( MIN_FILTER );  // _min_filter
    ADD_USER_SERIALIZER( MAG_FILTER );  // _mag_filter
    ADD_FLOAT_SERIALIZER( MaxAnisotropy, 1.0f );  // _maxAnisotropy
    ADD_BOOL_SERIALIZER( UseHardwareMipMapGeneration, true );  // _useHardwareMipMapGeneration
    ADD_BOOL_SERIALIZER( UnRefImageDataAfterApply, false );  // _unrefImageDataAfterApply
    ADD_BOOL_SERIALIZER( ClientStorageHint, false );  // _clientStorageHint
    ADD_BOOL_SERIALIZER( ResizeNonPowerOfTwoHint, true );  // _resizeNonPowerOfTwoHint
    ADD_VEC4D_SERIALIZER( BorderColor, osg::Vec4d(0.0,0.0,0.0,0.0) );  // _borderColor
    ADD_GLINT_SERIALIZER( BorderWidth, 0 );  // _borderWidth

    BEGIN_ENUM_SERIALIZER( InternalFormatMode, USE_IMAGE_DATA_FORMAT );
        ADD_ENUM_VALUE( USE_IMAGE_DATA_FORMAT );
        ADD_ENUM_VALUE( USE_USER_DEFINED_FORMAT );
        ADD_ENUM_VALUE( USE_ARB_COMPRESSION );
        ADD_ENUM_VALUE( USE_S3TC_DXT1_COMPRESSION );
        ADD_ENUM_VALUE( USE_S3TC_DXT3_COMPRESSION );
        ADD_ENUM_VALUE( USE_S3TC_DXT5_COMPRESSION );
        ADD_ENUM_VALUE( USE_PVRTC_2BPP_COMPRESSION );
        ADD_ENUM_VALUE( USE_PVRTC_4BPP_COMPRESSION );
        ADD_ENUM_VALUE( USE_ETC_COMPRESSION );
        ADD_ENUM_VALUE( USE_RGTC1_COMPRESSION );
        ADD_ENUM_VALUE( USE_RGTC2_COMPRESSION );
        ADD_ENUM_VALUE( USE_S3TC_DXT1c_COMPRESSION );
        ADD_ENUM_VALUE( USE_S3TC_DXT1a_COMPRESSION );
    END_ENUM_SERIALIZER();  // _internalFormatMode

    ADD_USER_SERIALIZER( InternalFormat );  // _internalFormat
    ADD_USER_SERIALIZER( SourceFormat );  // _sourceFormat
    ADD_USER_SERIALIZER( SourceType );  // _sourceType
    ADD_BOOL_SERIALIZER( ShadowComparison, false );  // _use_shadow_comparison

    BEGIN_ENUM_SERIALIZER( ShadowCompareFunc, LEQUAL );
        ADD_ENUM_VALUE( NEVER );
        ADD_ENUM_VALUE( LESS );
        ADD_ENUM_VALUE( EQUAL );
        ADD_ENUM_VALUE( LEQUAL );
        ADD_ENUM_VALUE( GREATER );
        ADD_ENUM_VALUE( NOTEQUAL );
        ADD_ENUM_VALUE( GEQUAL );
        ADD_ENUM_VALUE( ALWAYS );
    END_ENUM_SERIALIZER();  // _shadow_compare_func

    BEGIN_ENUM_SERIALIZER( ShadowTextureMode, LUMINANCE );
        ADD_ENUM_VALUE( LUMINANCE );
        ADD_ENUM_VALUE( INTENSITY );
        ADD_ENUM_VALUE( ALPHA );
    END_ENUM_SERIALIZER();  // _shadow_texture_mode

    ADD_FLOAT_SERIALIZER( ShadowAmbient, 0.0f );  // _shadow_ambient
}
