#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace osgVolume_SampleRatioProperty
{
    REGISTER_OBJECT_WRAPPER( osgVolume_SampleRatioProperty,
                            new osgVolume::SampleRatioProperty,
                            osgVolume::SampleRatioProperty,
                            "osg::Object osgVolume::Property osgVolume::ScalarProperty osgVolume::SampleRatioProperty" )
    {
    }
}

namespace osgVolume_SampleRatioWhenMovingProperty
{
    REGISTER_OBJECT_WRAPPER( osgVolume_SampleRatioWhenMovingProperty,
                            new osgVolume::SampleRatioWhenMovingProperty,
                            osgVolume::SampleRatioWhenMovingProperty,
                            "osg::Object osgVolume::Property osgVolume::ScalarProperty osgVolume::SampleRatioWhenMovingProperty" )
    {
    }
}
