#include "osg/CullFace"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool CullFace_readLocalData(Object& obj, Input& fr);
bool CullFace_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_CullFaceFuncProxy
(
    new osg::CullFace,
    "CullFace",
    "Object StateAttribute CullFace",
    &CullFace_readLocalData,
    &CullFace_writeLocalData
);


bool CullFace_readLocalData(Object& obj,Input& fr)
{
    bool iteratorAdvanced = false;

    CullFace& cullface = static_cast<CullFace&>(obj);

    if (fr[0].matchWord("mode"))
    {
        if (fr[1].matchWord("FRONT"))
        {
            cullface.setMode(CullFace::FRONT);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("BACK"))
        {
            cullface.setMode(CullFace::BACK);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FRONT_AND_BACK"))
        {
            cullface.setMode(CullFace::FRONT_AND_BACK);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool CullFace_writeLocalData(const Object& obj, Output& fw)
{

    const CullFace& cullface = static_cast<const CullFace&>(obj);

    switch(cullface.getMode())
    {
        case(CullFace::FRONT):          fw.indent() << "mode FRONT" <<std::endl; break;
        case(CullFace::BACK):           fw.indent() << "mode BACK" <<std::endl; break;
        case(CullFace::FRONT_AND_BACK): fw.indent() << "mode FRONT_AND_BACK" <<std::endl; break;
    }
    return true;
}
