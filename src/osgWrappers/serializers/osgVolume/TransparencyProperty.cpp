#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_TransparencyProperty,
                         new osgVolume::TransparencyProperty,
                         osgVolume::TransparencyProperty,
                         "osg::Object osgVolume::Property osgVolume::ScalarProperty osgVolume::TransparencyProperty" )
{
}
