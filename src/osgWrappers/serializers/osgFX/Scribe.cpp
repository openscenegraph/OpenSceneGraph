#include <osgFX/Scribe>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgFX_Scribe,
                         new osgFX::Scribe,
                         osgFX::Scribe,
                         "osg::Object osg::Node osg::Group osgFX::Effect osgFX::Scribe" )
{
    ADD_VEC4_SERIALIZER( WireframeColor, osg::Vec4() );
    ADD_FLOAT_SERIALIZER( WireframeLineWidth, 0.0f );
}
