#include <osg/TexEnv>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( TexEnv,
                         new osg::TexEnv,
                         osg::TexEnv,
                         "osg::Object osg::StateAttribute osg::TexEnv" )
{
    BEGIN_ENUM_SERIALIZER( Mode, MODULATE );
        ADD_ENUM_VALUE( DECAL );
        ADD_ENUM_VALUE( MODULATE );
        ADD_ENUM_VALUE( BLEND );
        ADD_ENUM_VALUE( REPLACE );
        ADD_ENUM_VALUE( ADD );
    END_ENUM_SERIALIZER();  // _mode

    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
}
