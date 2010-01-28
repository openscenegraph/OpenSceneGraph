#include <osg/CullFace>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( CullFace,
                         new osg::CullFace,
                         osg::CullFace,
                         "osg::Object osg::StateAttribute osg::CullFace" )
{
    BEGIN_ENUM_SERIALIZER( Mode, BACK );
        ADD_ENUM_VALUE( FRONT );
        ADD_ENUM_VALUE( BACK );
        ADD_ENUM_VALUE( FRONT_AND_BACK );
    END_ENUM_SERIALIZER();  // _mode
}
