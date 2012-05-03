#include "osg/Stencil"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool Stencil_readLocalData(Object& obj, Input& fr);
bool Stencil_writeLocalData(const Object& obj, Output& fw);

bool Stencil_matchFuncStr(const char* str,Stencil::Function& func);
const char* Stencil_getFuncStr(Stencil::Function func);
bool Stencil_matchOperationStr(const char* str,Stencil::Operation& op);
const char* Stencil_getOperationStr(Stencil::Operation op);


// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Stencil)
(
    new osg::Stencil,
    "Stencil",
    "Object StateAttribute Stencil",
    &Stencil_readLocalData,
    &Stencil_writeLocalData
);


bool Stencil_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Stencil& stencil = static_cast<Stencil&>(obj);

    bool setFunction = false;
    Stencil::Function func = stencil.getFunction();
    if (fr[0].matchWord("function") && Stencil_matchFuncStr(fr[1].getStr(),func))
    {
        setFunction = true;
        fr+=2;
        iteratorAdvanced = true;
    }

    int ref = stencil.getFunctionRef();
    if (fr[0].matchWord("functionRef") && fr[1].getInt(ref))
    {
        setFunction = true;
        fr+=2;
        iteratorAdvanced = true;
    }

    unsigned int mask = stencil.getFunctionMask();
    if (fr[0].matchWord("functionMask") && fr[1].getUInt(mask))
    {
        setFunction = true;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (setFunction) stencil.setFunction(func,ref,mask);

    bool setOperation = false;
    osg::Stencil::Operation sfail = stencil.getStencilFailOperation();
    if (fr[0].matchWord("stencilFailOperation") && Stencil_matchOperationStr(fr[1].getStr(),sfail))
    {
        setOperation = true;
        fr+=2;
        iteratorAdvanced = true;
    }

    osg::Stencil::Operation zfail = stencil.getStencilPassAndDepthFailOperation();
    if (fr[0].matchWord("stencilPassAndDepthFailOperation") && Stencil_matchOperationStr(fr[1].getStr(),zfail))
    {
        setOperation = true;
        fr+=2;
        iteratorAdvanced = true;
    }

    osg::Stencil::Operation zpass = stencil.getStencilPassAndDepthPassOperation();
    if (fr[0].matchWord("stencilPassAndDepthPassOperation") && Stencil_matchOperationStr(fr[1].getStr(),zpass))
    {
        setOperation = true;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (setOperation) stencil.setOperation(sfail, zfail, zpass);


    if (fr[0].matchWord("writeMask") && fr[1].getUInt(mask))
    {
        stencil.setWriteMask(mask);
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Stencil_writeLocalData(const Object& obj,Output& fw)
{
    const Stencil& stencil = static_cast<const Stencil&>(obj);

    fw.indent() << "function " << Stencil_getFuncStr(stencil.getFunction()) << std::endl;
    fw.indent() << "functionRef " << stencil.getFunctionRef() << std::endl;
    fw.indent() << "functionMask 0x" << hex << stencil.getFunctionMask() << dec << std::endl;

    fw.indent() << "stencilFailOperation " << Stencil_getOperationStr(stencil.getStencilFailOperation()) << std::endl;
    fw.indent() << "stencilPassAndDepthFailOperation " << Stencil_getOperationStr(stencil.getStencilPassAndDepthFailOperation()) << std::endl;
    fw.indent() << "stencilPassAndDepthPassOperation " << Stencil_getOperationStr(stencil.getStencilPassAndDepthPassOperation()) << std::endl;

    fw.indent() << "writeMask 0x" << hex << stencil.getWriteMask() << dec << std::endl;

    return true;
}


bool Stencil_matchFuncStr(const char* str,Stencil::Function& func)
{
    if (strcmp(str,"NEVER")==0) func = Stencil::NEVER;
    else if (strcmp(str,"LESS")==0) func = Stencil::LESS;
    else if (strcmp(str,"EQUAL")==0) func = Stencil::EQUAL;
    else if (strcmp(str,"LEQUAL")==0) func = Stencil::LEQUAL;
    else if (strcmp(str,"GREATER")==0) func = Stencil::GREATER;
    else if (strcmp(str,"NOTEQUAL")==0) func = Stencil::NOTEQUAL;
    else if (strcmp(str,"GEQUAL")==0) func = Stencil::GEQUAL;
    else if (strcmp(str,"ALWAYS")==0) func = Stencil::ALWAYS;
    else return false;
    return true;
}


const char* Stencil_getFuncStr(Stencil::Function func)
{
    switch(func)
    {
        case(Stencil::NEVER): return "NEVER";
        case(Stencil::LESS): return "LESS";
        case(Stencil::EQUAL): return "EQUAL";
        case(Stencil::LEQUAL): return "LEQUAL";
        case(Stencil::GREATER): return "GREATER";
        case(Stencil::NOTEQUAL): return "NOTEQUAL";
        case(Stencil::GEQUAL): return "GEQUAL";
        case(Stencil::ALWAYS): return "ALWAYS";
    }
    return "";
}

bool Stencil_matchOperationStr(const char* str,Stencil::Operation& op)
{
    if (strcmp(str,"KEEP")==0) op = Stencil::KEEP;
    else if (strcmp(str,"ZERO")==0) op = Stencil::ZERO;
    else if (strcmp(str,"REPLACE")==0) op = Stencil::REPLACE;
    else if (strcmp(str,"INCR")==0) op = Stencil::INCR;
    else if (strcmp(str,"DECR")==0) op = Stencil::DECR;
    else if (strcmp(str,"INVERT")==0) op = Stencil::INVERT;
    else if (strcmp(str,"INCR_WRAP")==0) op = Stencil::INCR_WRAP;
    else if (strcmp(str,"DECR_WRAP")==0) op = Stencil::DECR_WRAP;
    else return false;
    return true;
}

const char* Stencil_getOperationStr(Stencil::Operation op)
{
    switch(op)
    {
        case(Stencil::KEEP): return "KEEP";
        case(Stencil::ZERO): return "ZERO";
        case(Stencil::REPLACE): return "REPLACE";
        case(Stencil::INCR): return "INCR";
        case(Stencil::DECR): return "DECR";
        case(Stencil::INVERT): return "INVERT";
        case(Stencil::INCR_WRAP): return "INCR_WRAP";
        case(Stencil::DECR_WRAP): return "DECR_WRAP";
    }
    return "";
}

