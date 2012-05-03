#include <osg/Multisample>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Multisample,
                         new osg::Multisample,
                         osg::Multisample,
                         "osg::Object osg::StateAttribute osg::Multisample" )
{
    ADD_FLOAT_SERIALIZER( Coverage, 0.0f );  // _coverage
    ADD_BOOL_SERIALIZER( Invert, false );  // _invert

    BEGIN_ENUM_SERIALIZER2( Hint, osg::Multisample::Mode, DONT_CARE );
        ADD_ENUM_VALUE( FASTEST );
        ADD_ENUM_VALUE( NICEST );
        ADD_ENUM_VALUE( DONT_CARE );
    END_ENUM_SERIALIZER();  // _mode
}
