#include "osg/TexEnvCombine"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TexEnvCombine_readLocalData(Object& obj, Input& fr);
bool TexEnvCombine_writeLocalData(const Object& obj, Output& fw);

bool TexEnvCombine_matchCombineParamStr(const char* str,GLint& value);
const char* TexEnvCombine_getCombineParamStr(GLint value);

bool TexEnvCombine_matchSourceParamStr(const char* str,GLint& value);
const char* TexEnvCombine_getSourceParamStr(GLint value);

bool TexEnvCombine_matchOperandParamStr(const char* str,GLint& value);
const char* TexEnvCombine_getOperandParamStr(GLint value);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(TexEnvCombine)
(
    new osg::TexEnvCombine,
    "TexEnvCombine",
    "Object StateAttribute TexEnvCombine",
    &TexEnvCombine_readLocalData,
    &TexEnvCombine_writeLocalData
);


bool TexEnvCombine_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TexEnvCombine& texenv = static_cast<TexEnvCombine&>(obj);

    GLint value;
    if (fr[0].matchWord("combine_RGB") && TexEnvCombine_matchCombineParamStr(fr[1].getStr(),value))
    {
        texenv.setCombine_RGB(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("combine_Alpha") && TexEnvCombine_matchCombineParamStr(fr[1].getStr(),value))
    {
        texenv.setCombine_Alpha(value);
        fr+=2;
        iteratorAdvanced = true;
    }


    if (fr[0].matchWord("source0_RGB") && TexEnvCombine_matchSourceParamStr(fr[1].getStr(),value))
    {
        texenv.setSource0_RGB(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("source1_RGB") && TexEnvCombine_matchSourceParamStr(fr[1].getStr(),value))
    {
        texenv.setSource1_RGB(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("source2_RGB") && TexEnvCombine_matchSourceParamStr(fr[1].getStr(),value))
    {
        texenv.setSource2_RGB(value);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("source0_Alpha") && TexEnvCombine_matchSourceParamStr(fr[1].getStr(),value))
    {
        texenv.setSource0_Alpha(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("source1_Alpha") && TexEnvCombine_matchSourceParamStr(fr[1].getStr(),value))
    {
        texenv.setSource1_Alpha(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("source2_Alpha") && TexEnvCombine_matchSourceParamStr(fr[1].getStr(),value))
    {
        texenv.setSource2_Alpha(value);
        fr+=2;
        iteratorAdvanced = true;
    }



    if (fr[0].matchWord("operand0_RGB") && TexEnvCombine_matchOperandParamStr(fr[1].getStr(),value))
    {
        texenv.setOperand0_RGB(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("operand1_RGB") && TexEnvCombine_matchOperandParamStr(fr[1].getStr(),value))
    {
        texenv.setOperand1_RGB(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("operand2_RGB") && TexEnvCombine_matchOperandParamStr(fr[1].getStr(),value))
    {
        texenv.setOperand2_RGB(value);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("operand0_Alpha") && TexEnvCombine_matchOperandParamStr(fr[1].getStr(),value))
    {
        texenv.setOperand0_Alpha(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("operand1_Alpha") && TexEnvCombine_matchOperandParamStr(fr[1].getStr(),value))
    {
        texenv.setOperand1_Alpha(value);
        fr+=2;
        iteratorAdvanced = true;
    }
    if (fr[0].matchWord("operand2_Alpha") && TexEnvCombine_matchOperandParamStr(fr[1].getStr(),value))
    {
        texenv.setOperand2_Alpha(value);
        fr+=2;
        iteratorAdvanced = true;
    }

    float scale;
    if (fr[0].matchWord("scale_RGB") && fr[1].getFloat(scale))
    {
        texenv.setScale_RGB(scale);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("scale_Alpha") && fr[1].getFloat(scale))
    {
        texenv.setScale_Alpha(scale);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("constantColor %f %f %f %f"))
    {
        osg::Vec4 color;
        fr[1].getFloat(color[0]);
        fr[2].getFloat(color[1]);
        fr[3].getFloat(color[2]);
        fr[4].getFloat(color[3]);

        texenv.setConstantColor(color);

        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool TexEnvCombine_writeLocalData(const Object& obj, Output& fw)
{
    const TexEnvCombine& texenv = static_cast<const TexEnvCombine&>(obj);

    fw.indent() << "combine_RGB " << TexEnvCombine_getCombineParamStr(texenv.getCombine_RGB()) << std::endl;
    fw.indent() << "combine_Alpha " << TexEnvCombine_getCombineParamStr(texenv.getCombine_Alpha()) << std::endl;

    fw.indent() << "source0_RGB " << TexEnvCombine_getSourceParamStr(texenv.getSource0_RGB()) << std::endl;
    fw.indent() << "source1_RGB " << TexEnvCombine_getSourceParamStr(texenv.getSource1_RGB()) << std::endl;
    fw.indent() << "source2_RGB " << TexEnvCombine_getSourceParamStr(texenv.getSource2_RGB()) << std::endl;

    fw.indent() << "source0_Alpha " << TexEnvCombine_getSourceParamStr(texenv.getSource0_Alpha()) << std::endl;
    fw.indent() << "source1_Alpha " << TexEnvCombine_getSourceParamStr(texenv.getSource1_Alpha()) << std::endl;
    fw.indent() << "source2_Alpha " << TexEnvCombine_getSourceParamStr(texenv.getSource2_Alpha()) << std::endl;

    fw.indent() << "operand0_RGB " << TexEnvCombine_getOperandParamStr(texenv.getOperand0_RGB()) << std::endl;
    fw.indent() << "operand1_RGB " << TexEnvCombine_getOperandParamStr(texenv.getOperand1_RGB()) << std::endl;
    fw.indent() << "operand2_RGB " << TexEnvCombine_getOperandParamStr(texenv.getOperand2_RGB()) << std::endl;

    fw.indent() << "operand0_Alpha " << TexEnvCombine_getOperandParamStr(texenv.getOperand0_Alpha()) << std::endl;
    fw.indent() << "operand1_Alpha " << TexEnvCombine_getOperandParamStr(texenv.getOperand1_Alpha()) << std::endl;
    fw.indent() << "operand2_Alpha " << TexEnvCombine_getOperandParamStr(texenv.getOperand2_Alpha()) << std::endl;

    fw.indent() << "scale_RGB " << texenv.getScale_RGB() << std::endl;
    fw.indent() << "scale_Alpha " << texenv.getScale_Alpha() << std::endl;

    fw.indent() << "constantColor " << texenv.getConstantColor() << std::endl;

    return true;
}


bool TexEnvCombine_matchCombineParamStr(const char* str,GLint& value)
{
    if      (strcmp(str,"REPLACE")==0)      value = TexEnvCombine::REPLACE;
    else if (strcmp(str,"MODULATE")==0)     value = TexEnvCombine::MODULATE;
    else if (strcmp(str,"ADD")==0)          value = TexEnvCombine::ADD;
    else if (strcmp(str,"ADD_SIGNED")==0)   value = TexEnvCombine::ADD_SIGNED;
    else if (strcmp(str,"INTERPOLATE")==0)  value = TexEnvCombine::INTERPOLATE;
    else if (strcmp(str,"SUBTRACT")==0)     value = TexEnvCombine::SUBTRACT;
    else if (strcmp(str,"DOT3_RGB")==0)     value = TexEnvCombine::DOT3_RGB;
    else if (strcmp(str,"DOT3_RGBA")==0)    value = TexEnvCombine::DOT3_RGBA;
    else return false;
    return true;
}

const char* TexEnvCombine_getCombineParamStr(GLint value)
{
    switch(value)
    {
        case(TexEnvCombine::REPLACE):       return "REPLACE";
        case(TexEnvCombine::MODULATE):      return "MODULATE";
        case(TexEnvCombine::ADD):           return "ADD";
        case(TexEnvCombine::ADD_SIGNED):    return "ADD_SIGNED";
        case(TexEnvCombine::INTERPOLATE):   return "INTERPOLATE";
        case(TexEnvCombine::SUBTRACT):      return "SUBTRACT";
        case(TexEnvCombine::DOT3_RGB):      return "DOT3_RGB";
        case(TexEnvCombine::DOT3_RGBA):     return "DOT3_RGBA";
    }
    return "";
}

bool TexEnvCombine_matchSourceParamStr(const char* str,GLint& value)
{
    if      (strcmp(str,"CONSTANT")==0)         value = TexEnvCombine::CONSTANT;
    else if (strcmp(str,"PRIMARY_COLOR")==0)    value = TexEnvCombine::PRIMARY_COLOR;
    else if (strcmp(str,"PREVIOUS")==0)         value = TexEnvCombine::PREVIOUS;
    else if (strcmp(str,"TEXTURE")==0)          value = TexEnvCombine::TEXTURE;
    else if (strcmp(str,"TEXTURE0")==0)         value = TexEnvCombine::TEXTURE0;
    else if (strcmp(str,"TEXTURE1")==0)         value = TexEnvCombine::TEXTURE1;
    else if (strcmp(str,"TEXTURE2")==0)         value = TexEnvCombine::TEXTURE2;
    else if (strcmp(str,"TEXTURE3")==0)         value = TexEnvCombine::TEXTURE3;
    else if (strcmp(str,"TEXTURE4")==0)         value = TexEnvCombine::TEXTURE4;
    else if (strcmp(str,"TEXTURE5")==0)         value = TexEnvCombine::TEXTURE5;
    else if (strcmp(str,"TEXTURE6")==0)         value = TexEnvCombine::TEXTURE6;
    else if (strcmp(str,"TEXTURE7")==0)         value = TexEnvCombine::TEXTURE7;
    else return false;
    return true;
}

const char* TexEnvCombine_getSourceParamStr(GLint value)
{
    switch(value)
    {
        case(TexEnvCombine::CONSTANT):      return "CONSTANT";
        case(TexEnvCombine::PRIMARY_COLOR): return "PRIMARY_COLOR";
        case(TexEnvCombine::PREVIOUS):      return "PREVIOUS";
        case(TexEnvCombine::TEXTURE):       return "TEXTURE";
        case(TexEnvCombine::TEXTURE0):      return "TEXTURE0";
        case(TexEnvCombine::TEXTURE1):      return "TEXTURE1";
        case(TexEnvCombine::TEXTURE2):      return "TEXTURE2";
        case(TexEnvCombine::TEXTURE3):      return "TEXTURE3";
        case(TexEnvCombine::TEXTURE4):      return "TEXTURE4";
        case(TexEnvCombine::TEXTURE5):      return "TEXTURE5";
        case(TexEnvCombine::TEXTURE6):      return "TEXTURE6";
        case(TexEnvCombine::TEXTURE7):      return "TEXTURE7";
    }
    return "";
}

bool TexEnvCombine_matchOperandParamStr(const char* str,GLint& value)
{
    if      (strcmp(str,"SRC_COLOR")==0)            value = TexEnvCombine::SRC_COLOR;
    else if (strcmp(str,"ONE_MINUS_SRC_COLOR")==0)  value = TexEnvCombine::ONE_MINUS_SRC_COLOR;
    else if (strcmp(str,"SRC_ALPHA")==0)            value = TexEnvCombine::SRC_ALPHA;
    else if (strcmp(str,"ONE_MINUS_SRC_ALPHA")==0)  value = TexEnvCombine::ONE_MINUS_SRC_ALPHA;
    else return false;
    return true;
}

const char* TexEnvCombine_getOperandParamStr(GLint value)
{
    switch(value)
    {
        case(TexEnvCombine::SRC_COLOR):             return "SRC_COLOR";
        case(TexEnvCombine::ONE_MINUS_SRC_COLOR):   return "ONE_MINUS_SRC_COLOR";
        case(TexEnvCombine::SRC_ALPHA):             return "SRC_ALPHA";
        case(TexEnvCombine::ONE_MINUS_SRC_ALPHA):   return "ONE_MINUS_SRC_ALPHA";
    }
    return "";
}

