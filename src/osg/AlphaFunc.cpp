#include "osg/GL"
#include "osg/AlphaFunc"
#include "osg/Output"
#include "osg/Input"


using namespace osg;

AlphaFunc::AlphaFunc()
{
    _comparisonFunc = ALWAYS;
    _referenceValue = 1.0f;
}

AlphaFunc::~AlphaFunc()
{
}

AlphaFunc* AlphaFunc::instance()
{
    static ref_ptr<AlphaFunc> s_AlphaFunc(new AlphaFunc);
    return s_AlphaFunc.get();
}

void AlphaFunc::enable()
{
    glEnable(GL_ALPHA_TEST);
}

void AlphaFunc::disable()
{
    glDisable(GL_ALPHA_TEST);
}

void AlphaFunc::apply()
{
    glAlphaFunc((GLenum)_comparisonFunc,_referenceValue);
}


bool AlphaFunc::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    ComparisonFunction func;
    if (fr[0].matchWord("comparisonFunc") && matchFuncStr(fr[1].getStr(),func))
    {
        _comparisonFunc = func;
        fr+=2;
        iteratorAdvanced = true;
    }

    float ref;
    if (fr[0].matchWord("referenceValue") && fr[1].getFloat(ref))
    {
        _referenceValue = ref;
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool AlphaFunc::writeLocalData(Output& fw)
{
    fw.indent() << "comparisonFunc " << getFuncStr(_comparisonFunc) << endl;
    fw.indent() << "referenceValue " << _referenceValue << endl;
    return true;
}

bool AlphaFunc::matchFuncStr(const char* str,ComparisonFunction& func)
{
    if (strcmp(str,"NEVER")==0) func = NEVER;
    else if (strcmp(str,"LESS")==0) func = LESS;
    else if (strcmp(str,"EQUAL")==0) func = EQUAL;
    else if (strcmp(str,"LEQUAL")==0) func = LEQUAL;
    else if (strcmp(str,"GREATER")==0) func = GREATER;
    else if (strcmp(str,"NOTEQUAL")==0) func = NOTEQUAL;
    else if (strcmp(str,"GEQUAL")==0) func = GEQUAL;
    else if (strcmp(str,"ALWAYS")==0) func = ALWAYS;
    else return false;
    return true;
}


const char* AlphaFunc::getFuncStr(ComparisonFunction func)
{
    switch(func)
    {
        case(NEVER): return "NEVER";
        case(LESS): return "LESS";
        case(EQUAL): return "EQUAL";
        case(LEQUAL): return "LEQUAL";
        case(GREATER): return "GREATER";
        case(NOTEQUAL): return "NOTEQUAL";
        case(GEQUAL): return "GEQUAL";
        case(ALWAYS): return "ALWAYS";
    }
    return "";
}
