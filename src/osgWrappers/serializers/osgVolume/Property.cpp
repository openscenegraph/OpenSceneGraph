#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_Property,
                         new osgVolume::Property,
                         osgVolume::Property,
                         "osg::Object osgVolume::Property" )
{
    ADD_UINT_SERIALIZER_NO_SET( ModifiedCount, 0 );
}
