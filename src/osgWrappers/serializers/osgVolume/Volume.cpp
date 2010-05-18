#include <osgVolume/Volume>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_Volume,
                         new osgVolume::Volume,
                         osgVolume::Volume,
                         "osg::Object osg::Node osg::Group osgVolume::Volume" )
{
    ADD_OBJECT_SERIALIZER( VolumeTechniquePrototype, osgVolume::VolumeTechnique, NULL );  // _volumeTechnique
}
