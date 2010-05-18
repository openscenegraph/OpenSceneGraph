#include <osgVolume/FixedFunctionTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_FixedFunctionTechnique,
                         new osgVolume::FixedFunctionTechnique,
                         osgVolume::FixedFunctionTechnique,
                         "osg::Object osgVolume::VolumeTechnique osgVolume::FixedFunctionTechnique" )
{
    ADD_UINT_SERIALIZER( NumSlices, 500 );  // _numSlices
}
