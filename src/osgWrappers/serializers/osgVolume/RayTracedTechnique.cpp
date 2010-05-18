#include <osgVolume/RayTracedTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_RayTracedTechnique,
                         new osgVolume::RayTracedTechnique,
                         osgVolume::RayTracedTechnique,
                         "osg::Object osgVolume::VolumeTechnique osgVolume::RayTracedTechnique" )
{
}
