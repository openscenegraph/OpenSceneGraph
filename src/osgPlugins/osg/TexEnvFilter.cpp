#include "osg/TexEnvFilter"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TexEnvFilter_readLocalData(Object& obj, Input& fr);
bool TexEnvFilter_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TexEnvFilterProxy
(
    new TexEnvFilter,
    "TexEnvFilter",
    "Object StateAttribute TexEnvFilter",
    &TexEnvFilter_readLocalData,
    &TexEnvFilter_writeLocalData
);


bool TexEnvFilter_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TexEnvFilter& texenvfilter = static_cast<TexEnvFilter&>(obj);

    float lodBias = 0.0f;
    if (fr[0].matchWord("lodBias") && fr[1].getFloat(lodBias))
    {
        fr += 2;
        texenvfilter.setLodBias(lodBias);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool TexEnvFilter_writeLocalData(const Object& obj, Output& fw)
{
    const TexEnvFilter& texenvfilter = static_cast<const TexEnvFilter&>(obj);

    fw.indent() << "lodBias " << texenvfilter.getLodBias() << std::endl;
    
    return true;
}
