#include "osg/Depth"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Depth_readLocalData(Object& obj, Input& fr);
bool Depth_writeLocalData(const Object& obj, Output& fw);

bool Depth_matchFuncStr(const char* str,Depth::Function& func);
const char* Depth_getFuncStr(Depth::Function func);


// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Depth)
(
    new osg::Depth,
    "Depth",
    "Object StateAttribute Depth",
    &Depth_readLocalData,
    &Depth_writeLocalData
);


bool Depth_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Depth& depth = static_cast<Depth&>(obj);

    Depth::Function func;
    if (fr[0].matchWord("function") && Depth_matchFuncStr(fr[1].getStr(),func))
    {
        depth.setFunction(func);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("writeMask"))
    {
        if (fr[1].matchWord("TRUE") || fr[1].matchWord("ON"))
        {
            depth.setWriteMask(true);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE") || fr[1].matchWord("OFF"))
        {
            depth.setWriteMask(false);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    double znear,zfar;
    if (fr[0].matchWord("range") && fr[1].getFloat(znear) && fr[2].getFloat(zfar))
    {
        depth.setRange(znear,zfar);
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Depth_writeLocalData(const Object& obj,Output& fw)
{
    const Depth& depth = static_cast<const Depth&>(obj);

    fw.indent() << "function " << Depth_getFuncStr(depth.getFunction()) << std::endl;

    fw.indent() << "writeMask ";
    if (depth.getWriteMask()) fw << "TRUE" << std::endl;
    else fw << "FALSE" << std::endl;

    fw.indent() << "range " << depth.getZNear() << " " << depth.getZFar() << std::endl;

    return true;
}


bool Depth_matchFuncStr(const char* str,Depth::Function& func)
{
    if (strcmp(str,"NEVER")==0) func = Depth::NEVER;
    else if (strcmp(str,"LESS")==0) func = Depth::LESS;
    else if (strcmp(str,"EQUAL")==0) func = Depth::EQUAL;
    else if (strcmp(str,"LEQUAL")==0) func = Depth::LEQUAL;
    else if (strcmp(str,"GREATER")==0) func = Depth::GREATER;
    else if (strcmp(str,"NOTEQUAL")==0) func = Depth::NOTEQUAL;
    else if (strcmp(str,"GEQUAL")==0) func = Depth::GEQUAL;
    else if (strcmp(str,"ALWAYS")==0) func = Depth::ALWAYS;
    else return false;
    return true;
}


const char* Depth_getFuncStr(Depth::Function func)
{
    switch(func)
    {
        case(Depth::NEVER): return "NEVER";
        case(Depth::LESS): return "LESS";
        case(Depth::EQUAL): return "EQUAL";
        case(Depth::LEQUAL): return "LEQUAL";
        case(Depth::GREATER): return "GREATER";
        case(Depth::NOTEQUAL): return "NOTEQUAL";
        case(Depth::GEQUAL): return "GEQUAL";
        case(Depth::ALWAYS): return "ALWAYS";
    }
    return "";
}
