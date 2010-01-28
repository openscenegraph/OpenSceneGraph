#include <osg/Transform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Transform,
                         new osg::Transform,
                         osg::Transform,
                         "osg::Object osg::Node osg::Group osg::Transform" )
{
    BEGIN_ENUM_SERIALIZER( ReferenceFrame, RELATIVE_RF );
        ADD_ENUM_VALUE( RELATIVE_RF );
        ADD_ENUM_VALUE( ABSOLUTE_RF );
        ADD_ENUM_VALUE( ABSOLUTE_RF_INHERIT_VIEWPOINT );
    END_ENUM_SERIALIZER();  // _referenceFrame
}
