#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_MaximumIntensityProjectionProperty,
                         new osgVolume::MaximumIntensityProjectionProperty,
                         osgVolume::MaximumIntensityProjectionProperty,
                         "osg::Object osgVolume::Property osgVolume::MaximumIntensityProjectionProperty" )
{
}
