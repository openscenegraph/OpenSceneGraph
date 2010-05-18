#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_LightingProperty,
                         new osgVolume::LightingProperty,
                         osgVolume::LightingProperty,
                         "osg::Object osgVolume::Property osgVolume::LightingProperty" )
{
}
