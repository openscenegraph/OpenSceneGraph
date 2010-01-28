#include <osg/Light>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Light,
                         new osg::Light,
                         osg::Light,
                         "osg::Object osg::StateAttribute osg::Light" )
{
    ADD_INT_SERIALIZER( LightNum, 0 );  // _lightnum
    ADD_VEC4_SERIALIZER( Ambient, osg::Vec4() );  // _ambient
    ADD_VEC4_SERIALIZER( Diffuse, osg::Vec4() );  // _diffuse
    ADD_VEC4_SERIALIZER( Specular, osg::Vec4() );  // _specular
    ADD_VEC4_SERIALIZER( Position, osg::Vec4() );  // _position
    ADD_VEC3_SERIALIZER( Direction, osg::Vec3() );  // _direction
    ADD_FLOAT_SERIALIZER( ConstantAttenuation, 1.0f );  // _constant_attenuation
    ADD_FLOAT_SERIALIZER( LinearAttenuation, 0.0f );  // _linear_attenuation
    ADD_FLOAT_SERIALIZER( QuadraticAttenuation, 0.0f );  // _quadratic_attenuation
    ADD_FLOAT_SERIALIZER( SpotExponent, 0.0f );  // _spot_exponent
    ADD_FLOAT_SERIALIZER( SpotCutoff, 180.0f );  // _spot_cutoff
}
