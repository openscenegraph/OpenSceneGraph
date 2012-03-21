#include <osg/Fog>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Fog,
                         new osg::Fog,
                         osg::Fog,
                         "osg::Object osg::StateAttribute osg::Fog" )
{
    BEGIN_ENUM_SERIALIZER( Mode, LINEAR );
        ADD_ENUM_VALUE( LINEAR );
        ADD_ENUM_VALUE( EXP );
        ADD_ENUM_VALUE( EXP2 );
    END_ENUM_SERIALIZER();  // _mode

    ADD_FLOAT_SERIALIZER( Start, 0.0f );  // _start
    ADD_FLOAT_SERIALIZER( End, 1.0f );  // _end
    ADD_FLOAT_SERIALIZER( Density, 1.0f );  // _density
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
    ADD_GLENUM_SERIALIZER( FogCoordinateSource, GLint, GL_NONE );  // _fogCoordinateSource
    ADD_BOOL_SERIALIZER( UseRadialFog, false );    // _useRadialFog
}
