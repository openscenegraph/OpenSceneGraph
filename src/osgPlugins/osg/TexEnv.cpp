#include "osg/TexEnv"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TexEnv_readLocalData(Object& obj, Input& fr);
bool TexEnv_writeLocalData(const Object& obj, Output& fw);
bool TexEnv_matchModeStr(const char* str,TexEnv::Mode& mode);
const char* TexEnv_getModeStr(TexEnv::Mode mode);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TexEnvProxy
(
    new osg::TexEnv,
    "TexEnv",
    "Object StateAttribute TexEnv",
    &TexEnv_readLocalData,
    &TexEnv_writeLocalData
);


bool TexEnv_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TexEnv& texenv = static_cast<TexEnv&>(obj);

    TexEnv::Mode mode;
    if (fr[0].matchWord("mode") && TexEnv_matchModeStr(fr[1].getStr(),mode))
    {
        texenv.setMode(mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("color %f %f %f %f"))
    {
        osg::Vec4 color;
        fr[1].getFloat(color[0]);
        fr[2].getFloat(color[1]);
        fr[3].getFloat(color[2]);
        fr[4].getFloat(color[3]);
    
        texenv.setColor(color);
        
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool TexEnv_writeLocalData(const Object& obj, Output& fw)
{
    const TexEnv& texenv = static_cast<const TexEnv&>(obj);

    fw.indent() << "mode " << TexEnv_getModeStr(texenv.getMode()) << std::endl;
    
    switch(texenv.getMode())
    {
        case(TexEnv::DECAL):
        case(TexEnv::MODULATE):
        case(TexEnv::REPLACE):
        case(TexEnv::ADD):
            break;
        case(TexEnv::BLEND):
        default:
            fw.indent() << "color " << texenv.getColor() << std::endl;
            break;
    }

    return true;
}

bool TexEnv_matchModeStr(const char* str,TexEnv::Mode& mode)
{
    if (strcmp(str,"DECAL")==0)         mode = TexEnv::DECAL;
    else if (strcmp(str,"MODULATE")==0) mode = TexEnv::MODULATE;
    else if (strcmp(str,"BLEND")==0)    mode = TexEnv::BLEND;
    else if (strcmp(str,"REPLACE")==0)  mode = TexEnv::REPLACE;
    else if (strcmp(str,"ADD")==0)      mode = TexEnv::ADD;
    else return false;
    return true;
}


const char* TexEnv_getModeStr(TexEnv::Mode mode)
{
    switch(mode)
    {
        case(TexEnv::DECAL):    return "DECAL";
        case(TexEnv::MODULATE): return "MODULATE";
        case(TexEnv::BLEND):    return "BLEND";
        case(TexEnv::REPLACE):  return "REPLACE";
        case(TexEnv::ADD):      return "ADD";
    }
    return "";
}

