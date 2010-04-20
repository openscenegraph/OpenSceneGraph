#include <osgShadow/SoftShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_SoftShadowMap,
                         new osgShadow::SoftShadowMap,
                         osgShadow::SoftShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ShadowMap osgShadow::SoftShadowMap" )
{
    ADD_FLOAT_SERIALIZER( SoftnessWidth, 0.0f );  // _softnessWidth
    ADD_FLOAT_SERIALIZER( JitteringScale, 32.0f );  // _jitteringScale
    ADD_UINT_SERIALIZER( JitterTextureUnit, 2 );  // _jitterTextureUnit
    ADD_FLOAT_SERIALIZER( Bias, 0.0f );
}
