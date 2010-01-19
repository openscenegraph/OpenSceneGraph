#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "osg/Light"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Light_readLocalData(Object& obj, Input& fr);
bool Light_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Light)
(
    new osg::Light,
    "Light",
    "Object StateAttribute Light",
    &Light_readLocalData,
    &Light_writeLocalData
);


bool Light_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Light& light = static_cast<Light&>(obj);

    if (fr[0].matchWord("light_num"))
    {
        int lightnum=0;
        if (fr[1].getInt(lightnum))
        {
            light.setLightNum(lightnum);
            fr += 2;
            iteratorAdvanced = true;
        }
    }

#define ReadVec4(A,B) {  \
    if (fr[0].matchWord(B) && \
        fr[1].getFloat(vec4[0]) && \
        fr[2].getFloat(vec4[1]) && \
        fr[3].getFloat(vec4[2]) && \
        fr[4].getFloat(vec4[3])) \
    { \
        light.A(vec4); \
        fr+=5; \
        iteratorAdvanced = true; \
    } \
} 

#define ReadVec3(A,B) {  \
    if (fr[0].matchWord(B) && \
        fr[1].getFloat(vec3[0]) && \
        fr[2].getFloat(vec3[1]) && \
        fr[3].getFloat(vec3[2])) \
    { \
        light.A(vec3); \
        fr+=4; \
        iteratorAdvanced = true; \
    } \
} 

#define ReadFloat(A,B) {  \
    if (fr[0].matchWord(B) && \
        fr[1].getFloat(value)) \
    { \
        light.A(value); \
        fr+=2; \
        iteratorAdvanced = true; \
    } \
} 

    Vec4 vec4;
    ReadVec4(setAmbient,"ambient")
    ReadVec4(setDiffuse,"diffuse")
    ReadVec4(setSpecular,"specular")
    ReadVec4(setPosition,"position")
    
    Vec3 vec3;
    ReadVec3(setDirection,"direction")
    
    float value;
    ReadFloat(setConstantAttenuation,"constant_attenuation")
    ReadFloat(setLinearAttenuation,"linear_attenuation")
    ReadFloat(setQuadraticAttenuation,"quadratic_attenuation")
    ReadFloat(setSpotExponent,"spot_exponent")
    ReadFloat(setSpotCutoff,"spot_cutoff")

#undef ReadVec4
#undef ReadVec3
#undef ReadFloat

    return iteratorAdvanced;
}


bool Light_writeLocalData(const Object& obj,Output& fw)
{
    const Light& light = static_cast<const Light&>(obj);

    fw.indent() << "light_num " << light.getLightNum() << std::endl;

    // Vec4's
    fw.indent() << "ambient " << light.getAmbient() << std::endl;
    fw.indent() << "diffuse " << light.getDiffuse() << std::endl;
    fw.indent() << "specular " << light.getSpecular() << std::endl;
    fw.indent() << "position " << light.getPosition() << std::endl;

    // Vec3's
    fw.indent() << "direction " << light.getDirection() << std::endl;

    // float's
    fw.indent() << "constant_attenuation " << light.getConstantAttenuation() << std::endl;
    fw.indent() << "linear_attenuation " << light.getLinearAttenuation () << std::endl;
    fw.indent() << "quadratic_attenuation " << light.getQuadraticAttenuation() << std::endl;
    fw.indent() << "spot_exponent " << light.getSpotExponent() << std::endl;
    fw.indent() << "spot_cutoff " << light.getSpotCutoff() << std::endl;

    return true;
}
