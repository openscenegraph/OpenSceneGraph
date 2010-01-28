#include <osg/FrontFace>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( FrontFace,
                         new osg::FrontFace,
                         osg::FrontFace,
                         "osg::Object osg::StateAttribute osg::FrontFace" )
{
    BEGIN_ENUM_SERIALIZER( Mode, COUNTER_CLOCKWISE );
        ADD_ENUM_VALUE( CLOCKWISE );
        ADD_ENUM_VALUE( COUNTER_CLOCKWISE );
    END_ENUM_SERIALIZER();  // _mode
}
