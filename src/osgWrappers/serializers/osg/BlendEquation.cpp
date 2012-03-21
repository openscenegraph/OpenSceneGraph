#include <osg/BlendEquation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( BlendEquation,
                         new osg::BlendEquation,
                         osg::BlendEquation,
                         "osg::Object osg::StateAttribute osg::BlendEquation" )
{
    BEGIN_ENUM_SERIALIZER2( EquationRGB, osg::BlendEquation::Equation, FUNC_ADD );
        ADD_ENUM_VALUE( RGBA_MIN );
        ADD_ENUM_VALUE( RGBA_MAX );
        ADD_ENUM_VALUE( ALPHA_MIN );
        ADD_ENUM_VALUE( ALPHA_MAX );
        ADD_ENUM_VALUE( LOGIC_OP );
        ADD_ENUM_VALUE( FUNC_ADD );
        ADD_ENUM_VALUE( FUNC_SUBTRACT );
        ADD_ENUM_VALUE( FUNC_REVERSE_SUBTRACT );
    END_ENUM_SERIALIZER();  // _equationRGB

    BEGIN_ENUM_SERIALIZER2( EquationAlpha, osg::BlendEquation::Equation, FUNC_ADD );
        ADD_ENUM_VALUE( RGBA_MIN );
        ADD_ENUM_VALUE( RGBA_MAX );
        ADD_ENUM_VALUE( ALPHA_MIN );
        ADD_ENUM_VALUE( ALPHA_MAX );
        ADD_ENUM_VALUE( LOGIC_OP );
        ADD_ENUM_VALUE( FUNC_ADD );
        ADD_ENUM_VALUE( FUNC_SUBTRACT );
        ADD_ENUM_VALUE( FUNC_REVERSE_SUBTRACT );
    END_ENUM_SERIALIZER();  // _equationAlpha
}
