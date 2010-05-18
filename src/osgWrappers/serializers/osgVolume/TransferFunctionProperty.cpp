#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_TransferFunctionProperty,
                         new osgVolume::TransferFunctionProperty,
                         osgVolume::TransferFunctionProperty,
                         "osg::Object osgVolume::Property osgVolume::TransferFunctionProperty" )
{
    ADD_OBJECT_SERIALIZER( TransferFunction, osg::TransferFunction, NULL );  // _tf
}
