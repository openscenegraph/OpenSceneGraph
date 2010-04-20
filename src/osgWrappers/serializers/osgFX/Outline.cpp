#include <osgFX/Outline>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgFX_Outline,
                         new osgFX::Outline,
                         osgFX::Outline,
                         "osg::Object osg::Node osg::Group osgFX::Effect osgFX::Outline" )
{
    ADD_FLOAT_SERIALIZER( Width, 0.0f );  // _width
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
}
