#include <osgFX/Cartoon>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgFX_Cartoon,
                         new osgFX::Cartoon,
                         osgFX::Cartoon,
                         "osg::Object osg::Node osg::Group osgFX::Effect osgFX::Cartoon" )
{
    ADD_VEC4_SERIALIZER( OutlineColor, osg::Vec4() );
    ADD_FLOAT_SERIALIZER( OutlineLineWidth, 0.0f );
    ADD_INT_SERIALIZER( LightNumber, 0 );  // _lightnum
}
