#include <osgTerrain/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_ContourLayer,
                         new osgTerrain::ContourLayer,
                         osgTerrain::ContourLayer,
                         "osg::Object osgTerrain::Layer osgTerrain::ContourLayer" )
{
    ADD_OBJECT_SERIALIZER( TransferFunction, osg::TransferFunction1D, NULL );  // _tf
}
