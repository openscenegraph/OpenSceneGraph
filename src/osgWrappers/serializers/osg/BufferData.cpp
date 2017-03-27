#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>



REGISTER_OBJECT_WRAPPER( BufferData,
                         0,
                         osg::BufferData,
                         "osg::Object osg::BufferData" )
{
    {
        UPDATE_TO_VERSION_SCOPED( 147 )
        ADD_OBJECT_SERIALIZER(BufferObject, osg::BufferObject, NULL);
    }
}
