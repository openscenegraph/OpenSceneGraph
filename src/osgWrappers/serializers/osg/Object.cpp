#include <osg/Object>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Object,
                         /*new osg::Object*/NULL,
                         osg::Object,
                         "osg::Object" )
{
    ADD_STRING_SERIALIZER( Name, "" );  // _name
    
    BEGIN_ENUM_SERIALIZER( DataVariance, UNSPECIFIED );
        ADD_ENUM_VALUE( STATIC );
        ADD_ENUM_VALUE( DYNAMIC );
        ADD_ENUM_VALUE( UNSPECIFIED );
    END_ENUM_SERIALIZER();  // _dataVariance
}
