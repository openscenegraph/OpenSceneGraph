#include "osg/Transparency"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Transparency_readLocalData(Object& obj, Input& fr);
bool Transparency_writeLocalData(const Object& obj, Output& fw);

bool Transparency_matchModeStr(const char* str,int& mode);
const char* Transparency_getModeStr(int value);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TransparencyProxy
(
    new osg::Transparency,
    "Transparency",
    "Object StateAttribute Transparency",
    &Transparency_readLocalData,
    &Transparency_writeLocalData
);


bool Transparency_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Transparency& transparency = static_cast<Transparency&>(obj);

    int mode;
    if (fr[0].matchWord("source") && Transparency_matchModeStr(fr[1].getStr(),mode))
    {
        transparency.setSource(mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("destination") && Transparency_matchModeStr(fr[1].getStr(),mode))
    {
        transparency.setDestination(mode);
        fr+=2;
        iteratorAdvanced = true;
    }
    
    return iteratorAdvanced;
}

bool Transparency_writeLocalData(const Object& obj, Output& fw)
{
    const Transparency& transparency = static_cast<const Transparency&>(obj);

    fw.indent() << "source " << Transparency_getModeStr(transparency.getSource()) << endl;
    fw.indent() << "destination " << Transparency_getModeStr(transparency.getDestination()) << endl;

    return true;
}



bool Transparency_matchModeStr(const char* str,int& mode)
{
    if (strcmp(str,"DST_ALPHA")==0)                 mode = Transparency::DST_ALPHA;
    else if (strcmp(str,"DST_COLOR")==0)            mode = Transparency::DST_COLOR;
    else if (strcmp(str,"ONE")==0)                  mode = Transparency::ONE;
    else if (strcmp(str,"ONE_MINUS_DST_ALPHA")==0)  mode = Transparency::ONE_MINUS_DST_ALPHA;
    else if (strcmp(str,"ONE_MINUS_DST_COLOR")==0)  mode = Transparency::ONE_MINUS_DST_COLOR;
    else if (strcmp(str,"ONE_MINUS_SRC_ALPHA")==0)  mode = Transparency::ONE_MINUS_SRC_ALPHA;
    else if (strcmp(str,"ONE_MINUS_SRC_COLOR")==0)  mode = Transparency::ONE_MINUS_SRC_COLOR;
    else if (strcmp(str,"SRC_ALPHA")==0)            mode = Transparency::SRC_ALPHA;
    else if (strcmp(str,"SRC_ALPHA_SATURATE")==0)   mode = Transparency::SRC_ALPHA_SATURATE;
    else if (strcmp(str,"SRC_COLOR")==0)            mode = Transparency::SRC_COLOR;
    else return false;
    return true;

}

const char* Transparency_getModeStr(int value)
{
    switch(value)
    {
        case(Transparency::DST_ALPHA) :             return "DST_ALPHA";
        case(Transparency::DST_COLOR) :             return "DST_COLOR";
        case(Transparency::ONE) :                   return "ONE";
        case(Transparency::ONE_MINUS_DST_ALPHA) :   return "ONE_MINUS_DST_ALPHA";
        case(Transparency::ONE_MINUS_DST_COLOR) :   return "ONE_MINUS_DST_COLOR";
        case(Transparency::ONE_MINUS_SRC_ALPHA) :   return "ONE_MINUS_SRC_ALPHA";
        case(Transparency::ONE_MINUS_SRC_COLOR) :   return "ONE_MINUS_SRC_COLOR";
        case(Transparency::SRC_ALPHA)  :            return "SRC_ALPHA";
        case(Transparency::SRC_ALPHA_SATURATE) :    return "SRC_ALPHA_SATURATE";
        case(Transparency::SRC_COLOR) :             return "SRC_COLOR";
        case(Transparency::ZERO) :                  return "ZERO";
    }

    return NULL;
}
