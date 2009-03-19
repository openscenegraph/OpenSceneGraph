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
    "Object Node Terrain Group",
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

    return iteratorAdvanced;
}


bool Terrain_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::Terrain& terrain = static_cast<const osgTerrain::Terrain&>(obj);
    fw.indent()<<"SampleRatio "<<terrain.getSampleRatio()<<std::endl;
    fw.indent()<<"VerticalScale "<<terrain.getVerticalScale()<<std::endl;

    return true;
}
