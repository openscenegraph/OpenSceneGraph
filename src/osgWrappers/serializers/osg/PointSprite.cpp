#include <osg/PointSprite>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( PointSprite,
                         new osg::PointSprite,
                         osg::PointSprite,
                         "osg::Object osg::StateAttribute osg::PointSprite" )
{
    BEGIN_ENUM_SERIALIZER( CoordOriginMode, UPPER_LEFT );
        ADD_ENUM_VALUE( UPPER_LEFT );
        ADD_ENUM_VALUE( LOWER_LEFT );
    END_ENUM_SERIALIZER();  // _coordOriginMode
}
