#include <osgFX/BumpMapping>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgFX_BumpMapping,
                         new osgFX::BumpMapping,
                         osgFX::BumpMapping,
                         "osg::Object osg::Node osg::Group osgFX::Effect osgFX::BumpMapping" )
{
    ADD_INT_SERIALIZER( LightNumber, 0 );  // _lightnum
    ADD_INT_SERIALIZER( DiffuseTextureUnit, 1 );  // _diffuse_unit
    ADD_INT_SERIALIZER( NormalMapTextureUnit, 0 );  // _normal_unit
    ADD_OBJECT_SERIALIZER( OverrideDiffuseTexture, osg::Texture2D, NULL );  // _diffuse_tex
    ADD_OBJECT_SERIALIZER( OverrideNormalMapTexture, osg::Texture2D, NULL );  // _normal_tex
}
