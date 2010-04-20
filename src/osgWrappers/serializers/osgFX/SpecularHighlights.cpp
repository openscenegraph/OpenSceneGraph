#include <osgFX/SpecularHighlights>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgFX_SpecularHighlights,
                         new osgFX::SpecularHighlights,
                         osgFX::SpecularHighlights,
                         "osg::Object osg::Node osg::Group osgFX::Effect osgFX::SpecularHighlights" )
{
    ADD_INT_SERIALIZER( LightNumber, 0 );  // _lightnum
    ADD_INT_SERIALIZER( TextureUnit, 0 );  // _unit
    ADD_VEC4_SERIALIZER( SpecularColor, osg::Vec4() );  // _color
    ADD_FLOAT_SERIALIZER( SpecularExponent, 0.0f );  // _sexp
}
