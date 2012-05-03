#include <osg/LightSource>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( LightSource,
                         new osg::LightSource,
                         osg::LightSource,
                         "osg::Object osg::Node osg::Group osg::LightSource" )
{
    ADD_OBJECT_SERIALIZER( Light, osg::Light, NULL );  // _light

    BEGIN_ENUM_SERIALIZER( ReferenceFrame, RELATIVE_RF );
        ADD_ENUM_VALUE( RELATIVE_RF );
        ADD_ENUM_VALUE( ABSOLUTE_RF );
    END_ENUM_SERIALIZER();  // _referenceFrame
}
