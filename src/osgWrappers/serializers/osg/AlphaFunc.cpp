#include <osg/AlphaFunc>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( AlphaFunc,
                         new osg::AlphaFunc,
                         osg::AlphaFunc,
                         "osg::Object osg::StateAttribute osg::AlphaFunc" )
{
    BEGIN_ENUM_SERIALIZER2( Function, osg::AlphaFunc::ComparisonFunction, ALWAYS );
        ADD_ENUM_VALUE( NEVER );
        ADD_ENUM_VALUE( LESS );
        ADD_ENUM_VALUE( EQUAL );
        ADD_ENUM_VALUE( LEQUAL );
        ADD_ENUM_VALUE( GREATER );
        ADD_ENUM_VALUE( NOTEQUAL );
        ADD_ENUM_VALUE( GEQUAL );
        ADD_ENUM_VALUE( ALWAYS );
    END_ENUM_SERIALIZER();  // _comparisonFunc

    ADD_FLOAT_SERIALIZER( ReferenceValue, 1.0f );  // _referenceValue
}
