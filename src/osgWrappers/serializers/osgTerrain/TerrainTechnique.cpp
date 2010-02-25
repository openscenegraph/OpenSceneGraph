#include <osgTerrain/TerrainTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_TerrainTechnique,
                         new osgTerrain::TerrainTechnique,
                         osgTerrain::TerrainTechnique,
                         "osg::Object osgTerrain::TerrainTechnique" )
{
}
