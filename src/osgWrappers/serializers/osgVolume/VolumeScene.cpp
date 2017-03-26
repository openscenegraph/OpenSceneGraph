#include <osg/Geometry>
#include <osgVolume/VolumeScene>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_VolumeScene,
                         new osgVolume::VolumeScene,
                         osgVolume::VolumeScene,
                         "osg::Object osg::Node osg::Group osgVolume::VolumeScene" )
{

}
