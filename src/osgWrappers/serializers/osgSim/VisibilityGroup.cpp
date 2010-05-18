#include <osgSim/VisibilityGroup>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgSim_VisibilityGroup,
                         new osgSim::VisibilityGroup,
                         osgSim::VisibilityGroup,
                         "osg::Object osg::Node osg::Group osgSim::VisibilityGroup" )
{
    ADD_OBJECT_SERIALIZER( VisibilityVolume, osg::Node, NULL );  // _visibilityVolume
    ADD_UINT_SERIALIZER( VolumeIntersectionMask, 0xffffffff );  // _volumeIntersectionMask
    ADD_FLOAT_SERIALIZER( SegmentLength, 0.0f );  // _segmentLength
}
