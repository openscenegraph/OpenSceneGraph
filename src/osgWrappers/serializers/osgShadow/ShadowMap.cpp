#include <osgShadow/ShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_ShadowMap,
                         new osgShadow::ShadowMap,
                         osgShadow::ShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ShadowMap" )
{
    ADD_UINT_SERIALIZER( TextureUnit, 1 );  // _shadowTextureUnit
    ADD_VEC2_SERIALIZER( PolygonOffset, osg::Vec2f() );  // _polyOffset
    ADD_VEC2_SERIALIZER( AmbientBias, osg::Vec2d() );  // _ambientBias

    ADD_SERIALIZER( (new osgDB::PropByRefSerializer<osgShadow::ShadowMap, osg::Vec2s>
                    ("TextureSize", osg::Vec2s(1024, 1024),
                     &osgShadow::ShadowMap::getTextureSize,
                     &osgShadow::ShadowMap::setTextureSize))
    );  // _textureSize
}
