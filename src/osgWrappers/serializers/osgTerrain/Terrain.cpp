#include <osgTerrain/Terrain>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_Terrain,
                         new osgTerrain::Terrain,
                         osgTerrain::Terrain,
                         "osg::Object osg::Node osg::Group osgTerrain::Terrain" )
{
    ADD_FLOAT_SERIALIZER( SampleRatio, 1.0f );  // _sampleRatio
    ADD_FLOAT_SERIALIZER( VerticalScale, 1.0f );  // _verticalScale
}
