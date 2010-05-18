#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_SampleDensityProperty,
                         new osgVolume::SampleDensityProperty,
                         osgVolume::SampleDensityProperty,
                         "osg::Object osgVolume::Property osgVolume::ScalarProperty osgVolume::SampleDensityProperty" )
{
}
