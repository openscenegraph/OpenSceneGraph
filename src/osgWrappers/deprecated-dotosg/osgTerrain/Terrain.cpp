#include "osgTerrain/Terrain"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

// forward declare functions to use later.
bool Terrain_readLocalData(osg::Object& obj, osgDB::Input& fr);
bool Terrain_writeLocalData(const osg::Object& obj, osgDB::Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Terrain)
(
    new osgTerrain::Terrain,
    "Terrain",
    "Object Node Terrain CoordinateSystemNode Group",
    &Terrain_readLocalData,
    &Terrain_writeLocalData
);

bool Terrain_readLocalData(osg::Object& obj, osgDB::Input& fr)
{
    bool iteratorAdvanced = false;

    osgTerrain::Terrain& terrain = static_cast<osgTerrain::Terrain&>(obj);

    float value;
    if (fr.read("SampleRatio",value)) terrain.setSampleRatio(value);
    if (fr.read("VerticalScale",value)) terrain.setVerticalScale(value);

    std::string blendingPolicy;
    if (fr.read("BlendingPolicy",blendingPolicy))
    {
        if (blendingPolicy == "INHERIT") terrain.setBlendingPolicy(osgTerrain::TerrainTile::INHERIT);
        else if (blendingPolicy == "DO_NOT_SET_BLENDING") terrain.setBlendingPolicy(osgTerrain::TerrainTile::DO_NOT_SET_BLENDING);
        else if (blendingPolicy == "ENABLE_BLENDING") terrain.setBlendingPolicy(osgTerrain::TerrainTile::ENABLE_BLENDING);
        else if (blendingPolicy == "ENABLE_BLENDING_WHEN_ALPHA_PRESENT") terrain.setBlendingPolicy(osgTerrain::TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT);
    }

    return iteratorAdvanced;
}


bool Terrain_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::Terrain& terrain = static_cast<const osgTerrain::Terrain&>(obj);
    fw.indent()<<"SampleRatio "<<terrain.getSampleRatio()<<std::endl;
    fw.indent()<<"VerticalScale "<<terrain.getVerticalScale()<<std::endl;

    switch(terrain.getBlendingPolicy())
    {
        case(osgTerrain::TerrainTile::INHERIT): fw.indent()<<"BlendingPolicy INHERIT"<<std::endl; break;
        case(osgTerrain::TerrainTile::DO_NOT_SET_BLENDING): fw.indent()<<"BlendingPolicy DO_NOT_SET_BLENDING"<<std::endl; break;
        case(osgTerrain::TerrainTile::ENABLE_BLENDING): fw.indent()<<"BlendingPolicy ENABLE_BLENDING"<<std::endl; break;
        case(osgTerrain::TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT): fw.indent()<<"BlendingPolicy ENABLE_BLENDING_WHEN_ALPHA_PRESENT"<<std::endl; break;
    }

    return true;
}
