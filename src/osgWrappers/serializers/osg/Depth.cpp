#include <osg/Depth>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Depth,
                         new osg::Depth,
                         osg::Depth,
                         "osg::Object osg::StateAttribute osg::Depth" )
{
    BEGIN_ENUM_SERIALIZER( Function, LESS );
        ADD_ENUM_VALUE( NEVER );
        ADD_ENUM_VALUE( LESS );
        ADD_ENUM_VALUE( EQUAL );
        ADD_ENUM_VALUE( LEQUAL );
        ADD_ENUM_VALUE( GREATER );
        ADD_ENUM_VALUE( NOTEQUAL );
        ADD_ENUM_VALUE( GEQUAL );
        ADD_ENUM_VALUE( ALWAYS );
    END_ENUM_SERIALIZER();  // _func

    ADD_DOUBLE_SERIALIZER( ZNear, 0.0 );  // _zNear
    ADD_DOUBLE_SERIALIZER( ZFar, 1.0 );  // _zFar
    ADD_BOOL_SERIALIZER( WriteMask, true );  // _depthWriteMask
}
