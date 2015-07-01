#include "osg/ClipControl"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

using namespace osg;
using namespace osgDB;

static bool ClipControl_matchOriginStr(const char* str,ClipControl::Origin& origin)
{
    if (strcmp(str,"LOWER_LEFT")==0) origin = ClipControl::LOWER_LEFT;
    else if (strcmp(str,"UPPER_LEFT")==0) origin = ClipControl::UPPER_LEFT;
    else return false;
    return true;
}

static const char* ClipControl_getOriginStr(ClipControl::Origin origin)
{
    switch(origin)
    {
        case(ClipControl::LOWER_LEFT): return "LOWER_LEFT";
        case(ClipControl::UPPER_LEFT): return "UPPER_LEFT";
    }
    return "";
}

static bool ClipControl_matchDepthModeStr(const char* str,ClipControl::DepthMode& depthMode)
{
    if (strcmp(str,"NEGATIVE_ONE_TO_ONE")==0) depthMode = ClipControl::NEGATIVE_ONE_TO_ONE;
    else if (strcmp(str,"ZERO_TO_ONE")==0) depthMode = ClipControl::ZERO_TO_ONE;
    else return false;
    return true;
}

static const char* ClipControl_getDepthModeStr(ClipControl::DepthMode depthMode)
{
    switch(depthMode)
    {
        case(ClipControl::NEGATIVE_ONE_TO_ONE): return "NEGATIVE_ONE_TO_ONE";
        case(ClipControl::ZERO_TO_ONE): return "ZERO_TO_ONE";
    }
    return "";
}

static bool ClipControl_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ClipControl& clipControl = static_cast<ClipControl&>(obj);

    ClipControl::Origin origin;
    if (fr[0].matchWord("origin") && ClipControl_matchOriginStr(fr[1].getStr(),origin))
    {
        clipControl.setOrigin(origin);
        fr+=2;
        iteratorAdvanced = true;
    }

    ClipControl::DepthMode depthMode;
    if (fr[0].matchWord("depthMode") && ClipControl_matchDepthModeStr(fr[1].getStr(),depthMode))
    {
        clipControl.setDepthMode(depthMode);
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


static bool ClipControl_writeLocalData(const Object& obj,Output& fw)
{
    const ClipControl& clipControl = static_cast<const ClipControl&>(obj);

    fw.indent() << "origin " << ClipControl_getOriginStr(clipControl.getOrigin()) << std::endl;
    fw.indent() << "depthMode " << ClipControl_getDepthModeStr(clipControl.getDepthMode()) << std::endl;

    return true;
}

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(ClipControl)
(
    new osg::ClipControl,
    "ClipControl",
    "Object StateAttribute ClipControl",
    &ClipControl_readLocalData,
    &ClipControl_writeLocalData
);
