#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/Fog>
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Fog_readLocalData(Object& obj, Input& fr);
bool Fog_writeLocalData(const Object& obj, Output& fw);
bool Fog_matchModeStr(const char* str,Fog::Mode& mode);
const char* Fog_getModeStr(Fog::Mode mode);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Fog)
(
    new osg::Fog,
    "Fog",
    "Object StateAttribute Fog",
    &Fog_readLocalData,
    &Fog_writeLocalData
);


bool Fog_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Fog& fog = static_cast<Fog&>(obj);

    Fog::Mode mode;
    if (fr[0].matchWord("mode") && Fog_matchModeStr(fr[1].getStr(),mode))
    {
        fog.setMode(mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    float value;
    if (fr[0].matchWord("density") && fr[1].getFloat(value))
    {
        fog.setDensity(value);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("start") && fr[1].getFloat(value))
    {
        fog.setStart(value);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("end") && fr[1].getFloat(value))
    {
        fog.setEnd(value);
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
        fog.setColor(color);
        fr+=5;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("fogCoordinateSource"))
    {
        if (fr[1].matchWord("FOG_COORDINATE"))
        {
            fog.setFogCoordinateSource(osg::Fog::FOG_COORDINATE);
            fr+=2;
            iteratorAdvanced = true;
        } else if (fr[1].matchWord("FRAGMENT_DEPTH"))
        {
            fog.setFogCoordinateSource(osg::Fog::FRAGMENT_DEPTH);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool Fog_writeLocalData(const Object& obj,Output& fw)
{
    const Fog& fog = static_cast<const Fog&>(obj);

    fw.indent() << "mode " << Fog_getModeStr(fog.getMode()) << std::endl;
    fw.indent() << "density " << fog.getDensity() << std::endl;
    fw.indent() << "start " << fog.getStart() << std::endl;
    fw.indent() << "end " << fog.getEnd() << std::endl;
    fw.indent() << "color " << fog.getColor() << std::endl;
    switch(fog.getFogCoordinateSource())
    {
        case(Fog::FOG_COORDINATE):
            fw.indent() << "fogCoordinateSource FOG_COORDINATE" << std::endl;
            break;
        case(Fog::FRAGMENT_DEPTH):
            fw.indent() << "fogCoordinateSource FRAGMENT_DEPTH" << std::endl;
            break;
        default:
            // unrecognized source type.
            break;
    }
    return true;
}


bool Fog_matchModeStr(const char* str,Fog::Mode& mode)
{
    if (strcmp(str,"LINEAR")==0) mode = Fog::LINEAR;
    else if (strcmp(str,"EXP")==0) mode = Fog::EXP;
    else if (strcmp(str,"EXP2")==0) mode = Fog::EXP2;
    else return false;
    return true;
}


const char* Fog_getModeStr(Fog::Mode mode)
{
    switch(mode)
    {
        case(Fog::LINEAR): return "LINEAR";
        case(Fog::EXP): return "EXP";
        case(Fog::EXP2): return "EXP2";
    }
    return "";
}
