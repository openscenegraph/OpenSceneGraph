#include <osg/ClipControl>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( ClipControl,
                         new osg::ClipControl,
                         osg::ClipControl,
                         "osg::Object osg::StateAttribute osg::ClipControl" )
{
    BEGIN_ENUM_SERIALIZER( Origin, LOWER_LEFT );
        ADD_ENUM_VALUE( LOWER_LEFT );
        ADD_ENUM_VALUE( UPPER_LEFT );
    END_ENUM_SERIALIZER();  // _origin

    BEGIN_ENUM_SERIALIZER( DepthMode, NEGATIVE_ONE_TO_ONE );
        ADD_ENUM_VALUE( NEGATIVE_ONE_TO_ONE );
        ADD_ENUM_VALUE( ZERO_TO_ONE );
    END_ENUM_SERIALIZER();  // _depthMode
}
