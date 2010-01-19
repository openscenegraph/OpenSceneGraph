#include "osg/ColorMask"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ColorMask_readLocalData(Object& obj, Input& fr);
bool ColorMask_writeLocalData(const Object& obj, Output& fw);
bool ColorMask_matchModeStr(const char* str,bool& mode);
const char* ColorMask_getModeStr(bool mode);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(ColorMask)
(
    new osg::ColorMask,
    "ColorMask",
    "Object StateAttribute ColorMask",
    &ColorMask_readLocalData,
    &ColorMask_writeLocalData
);


bool ColorMask_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ColorMask& colormask = static_cast<ColorMask&>(obj);

    bool redMask=colormask.getRedMask();
    if (fr[0].matchWord("redMask") && ColorMask_matchModeStr(fr[1].getStr(),redMask))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    bool greenMask=colormask.getGreenMask();
    if (fr[0].matchWord("greenMask") && ColorMask_matchModeStr(fr[1].getStr(),greenMask))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    bool blueMask=colormask.getBlueMask();
    if (fr[0].matchWord("blueMask") && ColorMask_matchModeStr(fr[1].getStr(),blueMask))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    bool alphaMask=colormask.getAlphaMask();
    if (fr[0].matchWord("alphaMask") && ColorMask_matchModeStr(fr[1].getStr(),alphaMask))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (iteratorAdvanced)
    {
        colormask.setMask(redMask,greenMask,blueMask,alphaMask);
    }

    return iteratorAdvanced;
}


bool ColorMask_writeLocalData(const Object& obj,Output& fw)
{
    const ColorMask& colormask = static_cast<const ColorMask&>(obj);

    fw.indent() << "redMask " << ColorMask_getModeStr(colormask.getRedMask()) <<std::endl;
    fw.indent() << "greenMask " << ColorMask_getModeStr(colormask.getGreenMask()) <<std::endl;
    fw.indent() << "blueMask " << ColorMask_getModeStr(colormask.getBlueMask()) <<std::endl;
    fw.indent() << "alphaMask " << ColorMask_getModeStr(colormask.getAlphaMask()) <<std::endl;
    return true;
}


bool ColorMask_matchModeStr(const char* str,bool& mode)
{
    if (strcmp(str,"TRUE")==0) mode = true;
    else if (strcmp(str,"FALSE")==0) mode = false;
    else if (strcmp(str,"ON")==0) mode = true;
    else if (strcmp(str,"OFF")==0) mode = false;
    else return false;
    return true;
}


const char* ColorMask_getModeStr(bool mode)
{
    if (mode) return "ON";
    else return "OFF";
}
