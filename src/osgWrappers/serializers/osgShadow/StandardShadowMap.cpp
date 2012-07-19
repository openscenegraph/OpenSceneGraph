#include <osgShadow/StandardShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_StandardShadowMap,
                         new osgShadow::StandardShadowMap,
                         osgShadow::StandardShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap osgShadow::StandardShadowMap" )
{
    ADD_UINT_SERIALIZER( BaseTextureUnit, 0 );  // _baseTextureUnit
    ADD_UINT_SERIALIZER( ShadowTextureUnit, 1 );  // _shadowTextureUnit
    ADD_UINT_SERIALIZER( BaseTextureCoordIndex, 0 );  // _baseTextureCoordIndex
    ADD_UINT_SERIALIZER( ShadowTextureCoordIndex, 1 );  // _shadowTextureCoordIndex

    ADD_SERIALIZER( (new osgDB::PropByRefSerializer<osgShadow::StandardShadowMap, osg::Vec2s>
                    ("TextureSize", osg::Vec2s(1024, 1024),
                     &osgShadow::StandardShadowMap::getTextureSize,
                     &osgShadow::StandardShadowMap::setTextureSize))
    );  // _textureSize

    ADD_OBJECT_SERIALIZER( Light, osg::Light, NULL );  // _light
}
