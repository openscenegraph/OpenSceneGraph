#include "osg/AlphaFunc"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool AlphaFunc_readLocalData(Object& obj, Input& fr);
bool AlphaFunc_writeLocalData(const Object& obj, Output& fw);
bool AlphaFunc_matchFuncStr(const char* str,AlphaFunc::ComparisonFunction& func);
const char* AlphaFunc_getFuncStr(AlphaFunc::ComparisonFunction func);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_AlphaFuncProxy
(
    new osg::AlphaFunc,
    "AlphaFunc",
    "Object StateAttribute AlphaFunc",
    &AlphaFunc_readLocalData,
    &AlphaFunc_writeLocalData
);


bool AlphaFunc_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    AlphaFunc& alphaFunc = static_cast<AlphaFunc&>(obj);

    AlphaFunc::ComparisonFunction func = alphaFunc.getFunction();
    if (fr[0].matchWord("comparisonFunc") && AlphaFunc_matchFuncStr(fr[1].getStr(),func))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    float ref = alphaFunc.getReferenceValue();
    if (fr[0].matchWord("referenceValue") && fr[1].getFloat(ref))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (iteratorAdvanced) alphaFunc.setFunction(func,ref);

    return iteratorAdvanced;
}


bool AlphaFunc_writeLocalData(const Object& obj,Output& fw)
{
    const AlphaFunc& alphaFunc = static_cast<const AlphaFunc&>(obj);

    fw.indent() << "comparisonFunc " << AlphaFunc_getFuncStr(alphaFunc.getFunction()) << std::endl;
    fw.indent() << "referenceValue " << alphaFunc.getReferenceValue() << std::endl;
    return true;
}


bool AlphaFunc_matchFuncStr(const char* str,AlphaFunc::ComparisonFunction& func)
{
    if (strcmp(str,"NEVER")==0) func = AlphaFunc::NEVER;
    else if (strcmp(str,"LESS")==0) func = AlphaFunc::LESS;
    else if (strcmp(str,"EQUAL")==0) func = AlphaFunc::EQUAL;
    else if (strcmp(str,"LEQUAL")==0) func = AlphaFunc::LEQUAL;
    else if (strcmp(str,"GREATER")==0) func = AlphaFunc::GREATER;
    else if (strcmp(str,"NOTEQUAL")==0) func = AlphaFunc::NOTEQUAL;
    else if (strcmp(str,"GEQUAL")==0) func = AlphaFunc::GEQUAL;
    else if (strcmp(str,"ALWAYS")==0) func = AlphaFunc::ALWAYS;
    else return false;
    return true;
}


const char* AlphaFunc_getFuncStr(AlphaFunc::ComparisonFunction func)
{
    switch(func)
    {
        case(AlphaFunc::NEVER): return "NEVER";
        case(AlphaFunc::LESS): return "LESS";
        case(AlphaFunc::EQUAL): return "EQUAL";
        case(AlphaFunc::LEQUAL): return "LEQUAL";
        case(AlphaFunc::GREATER): return "GREATER";
        case(AlphaFunc::NOTEQUAL): return "NOTEQUAL";
        case(AlphaFunc::GEQUAL): return "GEQUAL";
        case(AlphaFunc::ALWAYS): return "ALWAYS";
    }
    return "";
}
