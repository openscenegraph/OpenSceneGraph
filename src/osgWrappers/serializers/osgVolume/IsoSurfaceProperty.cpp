#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_IsoSurfaceProperty,
                         new osgVolume::IsoSurfaceProperty,
                         osgVolume::IsoSurfaceProperty,
                         "osg::Object osgVolume::Property osgVolume::ScalarProperty osgVolume::IsoSurfaceProperty" )
{
}
