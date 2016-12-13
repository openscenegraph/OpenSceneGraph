#include <osgVolume/VolumeScene>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_VolumeScene,
                         new osgVolume::VolumeScene,
                         osgVolume::VolumeScene,
                         "osg::Object osg::Node osg::Group osgVolume::VolumeScene" )
{
    //ADD_HEXINT_SERIALIZER( ReceivesShadowTraversalMask, 0xffffffff );  // _receivesShadowTraversalMask
    //ADD_HEXINT_SERIALIZER( CastsShadowTraversalMask, 0xffffffff );  // _castsShadowTraversalMask
    //ADD_OBJECT_SERIALIZER( VolumeTechnique, osgVolume::VolumeTechnique, NULL );  // _volumeTechnique
}
