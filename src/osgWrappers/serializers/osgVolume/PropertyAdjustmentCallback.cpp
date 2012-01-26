#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_PropertyAdjustmentCallback,
                         new osgVolume::PropertyAdjustmentCallback,
                         osgVolume::PropertyAdjustmentCallback,
                         "osg::Object osg::NodeCallback osgVolume::PropertyAdjustmentCallback" )
{
    ADD_INT_SERIALIZER( KeyEventCycleForward, 'v' );  // _cyleForwardKey
    ADD_INT_SERIALIZER( KeyEventCycleBackward, 'V' );  // _cyleBackwardKey
    ADD_INT_SERIALIZER( KeyEventActivatesTransparencyAdjustment, 't' );  // _transparencyKey
    ADD_INT_SERIALIZER( KeyEventActivatesAlphaFuncAdjustment, 'a' );  // _alphaFuncKey
    ADD_INT_SERIALIZER( KeyEventActivatesSampleDensityAdjustment, 'd' );  // _sampleDensityKey
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
