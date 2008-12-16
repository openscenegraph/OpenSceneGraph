#include "osg/PolygonOffset"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool PolygonOffset_readLocalData(Object& obj, Input& fr);
bool PolygonOffset_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(PolygonOffset)
(
    new osg::PolygonOffset,
    "PolygonOffset",
    "Object StateAttribute PolygonOffset",
    &PolygonOffset_readLocalData,
    &PolygonOffset_writeLocalData
);


bool PolygonOffset_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    PolygonOffset& polygonoffset = static_cast<PolygonOffset&>(obj);

    float data;
    if (fr[0].matchWord("factor") && fr[1].getFloat(data))
    {

        polygonoffset.setFactor(data);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("units") && fr[1].getFloat(data))
    {

        polygonoffset.setUnits(data);
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool PolygonOffset_writeLocalData(const Object& obj, Output& fw)
{
    const PolygonOffset& polygonoffset = static_cast<const PolygonOffset&>(obj);

    fw.indent() << "factor " << polygonoffset.getFactor() << std::endl;
    fw.indent() << "units  " << polygonoffset.getUnits() << std::endl;
    return true;
}
