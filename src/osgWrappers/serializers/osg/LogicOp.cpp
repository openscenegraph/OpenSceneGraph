#include <osg/LogicOp>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( LogicOp,
                         new osg::LogicOp,
                         osg::LogicOp,
                         "osg::Object osg::StateAttribute osg::LogicOp" )
{
    BEGIN_ENUM_SERIALIZER( Opcode, COPY );
        ADD_ENUM_VALUE( CLEAR );
        ADD_ENUM_VALUE( SET );
        ADD_ENUM_VALUE( COPY );
        ADD_ENUM_VALUE( COPY_INVERTED );
        ADD_ENUM_VALUE( NOOP );
        ADD_ENUM_VALUE( INVERT );
        ADD_ENUM_VALUE( AND );
        ADD_ENUM_VALUE( NAND );
        ADD_ENUM_VALUE( OR );
        ADD_ENUM_VALUE( NOR );
        ADD_ENUM_VALUE( XOR );
        ADD_ENUM_VALUE( EQUIV );
        ADD_ENUM_VALUE( AND_REVERSE );
        ADD_ENUM_VALUE( AND_INVERTED );
        ADD_ENUM_VALUE( OR_REVERSE );
        ADD_ENUM_VALUE( OR_INVERTED );
    END_ENUM_SERIALIZER();  // _opcode
}
