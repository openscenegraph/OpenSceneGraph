#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include "osg/Light"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Light_readLocalData(Object& obj, Input& fr);
bool Light_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_LightProxy
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
        fr[3].getFloat(vec4[2])) \
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
        fr+=4; \
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

    // Vec4's
    fw.indent() << "ambient " << light.getAmbient() << endl;
    fw.indent() << "diffuse " << light.getDiffuse() << endl;
    fw.indent() << "specular " << light.getSpecular() << endl;
    fw.indent() << "position " << light.getPosition() << endl;

    // Vec3's
    fw.indent() << "direction " << light.getDirection() << endl;

    // float's
    fw.indent() << "constant_attenuation " << light.getConstantAttenuation() << endl;
    fw.indent() << "linear_attenuation " << light.getLinearAttenuation () << endl;
    fw.indent() << "quadratic_attenuation " << light.getQuadraticAttenuation() << endl;
    fw.indent() << "spot_exponent " << light.getSpotExponent() << endl;
    fw.indent() << "spot_cutoff " << light.getSpotCutoff() << endl;

    return true;
}
