#include "osg/EarthSky"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool EarthSky_readLocalData(Object& obj, Input& fr);
bool EarthSky_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_EarthSkyProxy
(
    new osg::EarthSky,
    "EarthSky",
    "Object Node EarthSky Group",
    &EarthSky_readLocalData,
    &EarthSky_writeLocalData
);

bool EarthSky_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    EarthSky& es = static_cast<EarthSky&>(obj);

    if (fr.matchSequence("requiresClear"))
    {
        if (fr[1].matchWord("TRUE"))
        {
            es.setRequiresClear(true);
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            es.setRequiresClear(false);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    osg::Vec4 vec4(0.0f,0.0f,0.0f,1.0f);
    
    if (fr[0].matchWord("clearColor") && 
        fr[1].getFloat(vec4[0]) &&
        fr[2].getFloat(vec4[1]) &&
        fr[3].getFloat(vec4[2]) &&
        fr[4].getFloat(vec4[3]))
    {
        es.setClearColor(vec4);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool EarthSky_writeLocalData(const Object& obj, Output& fw)
{
    const EarthSky& es = static_cast<const EarthSky&>(obj);

    fw.indent() << "requiresClear ";
    if (es.getRequiresClear())
    {
        fw<<"TRUE"<< std::endl;
    }
    else
    {
        fw<<"FALSE"<< std::endl;
    }
    
    fw.indent() << "clearColor "<<es.getClearColor()<< std::endl;

    return true;
}
