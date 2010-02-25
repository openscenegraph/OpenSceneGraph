#include <osgTerrain/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_ImageLayer,
                         new osgTerrain::ImageLayer,
                         osgTerrain::ImageLayer,
                         "osg::Object osgTerrain::Layer osgTerrain::ImageLayer" )
{
    ADD_IMAGE_SERIALIZER( Image, osg::Image, NULL );  // _image
}
