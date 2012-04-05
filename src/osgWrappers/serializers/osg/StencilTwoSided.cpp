#include <osg/StencilTwoSided>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

BEGIN_USER_TABLE( Function, osg::StencilTwoSided );
    ADD_USER_VALUE( NEVER );
    ADD_USER_VALUE( LESS );
    ADD_USER_VALUE( EQUAL );
    ADD_USER_VALUE( LEQUAL );
    ADD_USER_VALUE( GREATER );
    ADD_USER_VALUE( NOTEQUAL );
    ADD_USER_VALUE( GEQUAL );
    ADD_USER_VALUE( ALWAYS );
END_USER_TABLE()

USER_READ_FUNC( Function, readFunction )
USER_WRITE_FUNC( Function, writeFunction )

BEGIN_USER_TABLE( Operation, osg::StencilTwoSided );
    ADD_USER_VALUE( KEEP );
    ADD_USER_VALUE( ZERO );
    ADD_USER_VALUE( REPLACE );
    ADD_USER_VALUE( INCR );
    ADD_USER_VALUE( DECR );
    ADD_USER_VALUE( INVERT );
    ADD_USER_VALUE( INCR_WRAP );
    ADD_USER_VALUE( DECR_WRAP );
END_USER_TABLE()

USER_READ_FUNC( Operation, readOperation )
USER_WRITE_FUNC( Operation, writeOperation )

#define STENCIL_INT_VALUE_FUNC( PROP, TYPE ) \
    static bool check##PROP( const osg::StencilTwoSided& attr ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osg::StencilTwoSided& attr ) { \
        TYPE value1; is >> is.PROPERTY("Front") >> value1; \
        TYPE value2; is >> is.PROPERTY("Back") >> value2; \
        attr.set##PROP(osg::StencilTwoSided::FRONT, value1); \
        attr.set##PROP(osg::StencilTwoSided::BACK, value2); return true; } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::StencilTwoSided& attr ) { \
        os << os.PROPERTY("Front") << attr.get##PROP(osg::StencilTwoSided::FRONT); \
        os << os.PROPERTY("Back") << attr.get##PROP(osg::StencilTwoSided::BACK); \
        os << std::endl; return true; }

#define STENCIL_USER_VALUE_FUNC( PROP, TYPE ) \
    static bool check##PROP( const osg::StencilTwoSided& attr ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osg::StencilTwoSided& attr ) { \
        is >> is.PROPERTY("Front"); int value1 = read##TYPE(is); \
        is >> is.PROPERTY("Back"); int value2 = read##TYPE(is); \
        attr.set##PROP(osg::StencilTwoSided::FRONT, (osg::StencilTwoSided::TYPE)value1); \
        attr.set##PROP(osg::StencilTwoSided::BACK, (osg::StencilTwoSided::TYPE)value2); \
        return true; } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::StencilTwoSided& attr ) { \
        os << os.PROPERTY("Front"); write##TYPE(os, (int)attr.get##PROP(osg::StencilTwoSided::FRONT)); \
        os << os.PROPERTY("Back"); write##TYPE(os, (int)attr.get##PROP(osg::StencilTwoSided::BACK)); \
        os << std::endl; return true; }

STENCIL_USER_VALUE_FUNC( Function, Function )
STENCIL_INT_VALUE_FUNC( FunctionRef, int )
STENCIL_INT_VALUE_FUNC( FunctionMask, unsigned int )
STENCIL_USER_VALUE_FUNC( StencilFailOperation, Operation )
STENCIL_USER_VALUE_FUNC( StencilPassAndDepthFailOperation, Operation )
STENCIL_USER_VALUE_FUNC( StencilPassAndDepthPassOperation, Operation )
STENCIL_INT_VALUE_FUNC( WriteMask, unsigned int )

REGISTER_OBJECT_WRAPPER( StencilTwoSided,
                         new osg::StencilTwoSided,
                         osg::StencilTwoSided,
                         "osg::Object osg::StateAttribute osg::StencilTwoSided" )
{
    ADD_USER_SERIALIZER( Function );  // _func
    ADD_USER_SERIALIZER( FunctionRef );  // _funcRef
    ADD_USER_SERIALIZER( FunctionMask );  // _funcMask
    ADD_USER_SERIALIZER( StencilFailOperation );  // _sfail
    ADD_USER_SERIALIZER( StencilPassAndDepthFailOperation );  // _zfail
    ADD_USER_SERIALIZER( StencilPassAndDepthPassOperation );  // _zpass
    ADD_USER_SERIALIZER( WriteMask );  // _writeMask
}
