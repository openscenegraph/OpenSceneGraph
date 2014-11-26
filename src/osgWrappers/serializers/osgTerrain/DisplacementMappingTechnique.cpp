#include <osgTerrain/DisplacementMappingTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_DisplacementMappingTechnique,
                         new osgTerrain::DisplacementMappingTechnique,
                         osgTerrain::DisplacementMappingTechnique,
                         "osg::Object osgTerrain::TerrainTechnique osgTerrain::DisplacementMappingTechnique" )
{
}
