#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "osg/Material"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Material_readLocalData(Object& obj, Input& fr);
bool Material_writeLocalData(const Object& obj, Output& fw);
bool Material_matchFaceAndColor(Input& fr,const char* name,Material::Face& mf,Vec4& color);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Material)
(
    new osg::Material,
    "Material",
    "Object StateAttribute Material",
    &Material_readLocalData,
    &Material_writeLocalData
);


bool Material_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Material& material = static_cast<Material&>(obj);

    Vec4 data(0.0f,  0.0f, 0.0f, 1.0f);
    Material::Face mf = Material::FRONT_AND_BACK;

    if (fr[0].matchWord("ColorMode"))
    {
        if (fr[1].matchWord("AMBIENT"))
        {
            material.setColorMode(Material::AMBIENT);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("DIFFUSE"))
        {
            material.setColorMode(Material::DIFFUSE);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("SPECULAR"))
        {
            material.setColorMode(Material::SPECULAR);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("EMISSION"))
        {
            material.setColorMode(Material::EMISSION);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("AMBIENT_AND_DIFFUSE"))
        {
            material.setColorMode(Material::AMBIENT_AND_DIFFUSE);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("OFF"))
        {
            material.setColorMode(Material::OFF);
            fr+=2;
            iteratorAdvanced = true;
        }

    }

    if (Material_matchFaceAndColor(fr,"ambientColor",mf,data))
    {
        material.setAmbient(mf,data);
        iteratorAdvanced = true;
    }

    if (Material_matchFaceAndColor(fr,"diffuseColor",mf,data))
    {
        material.setDiffuse(mf,data);
        iteratorAdvanced = true;
    }

    if (Material_matchFaceAndColor(fr,"specularColor",mf,data))
    {
        material.setSpecular(mf,data);
        iteratorAdvanced = true;
    }

    if (Material_matchFaceAndColor(fr,"emissionColor",mf,data) ||
        Material_matchFaceAndColor(fr,"emissiveColor",mf,data))
    {
        material.setEmission(mf,data);
        iteratorAdvanced = true;
    }

    if (Material_matchFaceAndColor(fr,"ambientColor",mf,data))
    {
        material.setAmbient(mf,data);
        iteratorAdvanced = true;
    }

    float shininess = 0.0f;
    if (fr[0].matchWord("shininess"))
    {

        mf = Material::FRONT_AND_BACK;
        int fr_inc = 1;

        if (fr[1].matchWord("FRONT"))
        {
            mf = Material::FRONT;
            ++fr_inc;
        }
        else if (fr[1].matchWord("BACK"))
        {
            mf = Material::BACK;
            ++fr_inc;
        }

        if (fr[fr_inc].getFloat(shininess))
        {
            fr+=(fr_inc+1);
            material.setShininess(mf,shininess);
            iteratorAdvanced = true;
        }

    }

    float transparency = 0.0f;
    if (fr[0].matchWord("transparency") && fr[1].getFloat(transparency))
    {

        material.setTransparency(Material::FRONT_AND_BACK,transparency);

        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;

}

bool Material_matchFaceAndColor(Input& fr,const char* name,Material::Face& mf,Vec4& color)
{
    bool iteratorAdvanced = false;

    if (fr[0].matchWord(name))
    {
        int fr_inc = 1;
        if (fr[1].matchWord("FRONT"))
        {
            mf = Material::FRONT;
            ++fr_inc;
        }
        else if (fr[1].matchWord("BACK"))
        {
            mf = Material::BACK;
            ++fr_inc;
        }

        if (fr[fr_inc].getFloat(color[0]) && fr[fr_inc+1].getFloat(color[1]) && fr[fr_inc+2].getFloat(color[2]))
        {
            fr_inc += 3;

            if (fr[fr_inc].getFloat(color[3])) ++fr_inc;
            else color[3] = 1.0f;

            fr+=fr_inc;

            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool Material_writeLocalData(const Object& obj, Output& fw)
{

    const Material& material = static_cast<const Material&>(obj);

    switch(material.getColorMode())
    {
        case(Material::AMBIENT): fw.indent() << "ColorMode AMBIENT" << std::endl; break;
        case(Material::DIFFUSE): fw.indent() << "ColorMode DIFFUSE" << std::endl; break;
        case(Material::SPECULAR): fw.indent() << "ColorMode SPECULAR" << std::endl; break;
        case(Material::EMISSION): fw.indent() << "ColorMode EMISSION" << std::endl; break;
        case(Material::AMBIENT_AND_DIFFUSE): fw.indent() << "ColorMode AMBIENT_AND_DIFFUSE" << std::endl; break;
        case(Material::OFF): fw.indent() << "ColorMode OFF" << std::endl; break;
    }

    if (material.getAmbientFrontAndBack())
    {
        fw.indent() << "ambientColor " << material.getAmbient(Material::FRONT) << std::endl;
    }
    else
    {
        fw.indent() << "ambientColor FRONT " << material.getAmbient(Material::FRONT) << std::endl;
        fw.indent() << "ambientColor BACK  " << material.getAmbient(Material::BACK)  << std::endl;
    }

    if (material.getDiffuseFrontAndBack())
    {
        fw.indent() << "diffuseColor " << material.getDiffuse(Material::FRONT) << std::endl;
    }
    else
    {
        fw.indent() << "diffuseColor FRONT " << material.getDiffuse(Material::FRONT) << std::endl;
        fw.indent() << "diffuseColor BACK  " << material.getDiffuse(Material::BACK) << std::endl;
    }

    if (material.getSpecularFrontAndBack())
    {
        fw.indent() << "specularColor " << material.getSpecular(Material::FRONT) << std::endl;
    }
    else
    {
        fw.indent() << "specularColor FRONT " << material.getSpecular(Material::FRONT) << std::endl;
        fw.indent() << "specularColor BACK  " << material.getSpecular(Material::BACK) << std::endl;
    }

    if (material.getEmissionFrontAndBack())
    {
        fw.indent() << "emissionColor " << material.getEmission(Material::FRONT) << std::endl;
    }
    else
    {
        fw.indent() << "emissionColor FRONT " << material.getEmission(Material::FRONT) << std::endl;
        fw.indent() << "emissionColor BACK  " << material.getEmission(Material::BACK)  << std::endl;
    }

    if (material.getShininessFrontAndBack())
    {
        fw.indent() << "shininess " << material.getShininess(Material::FRONT) << std::endl;
    }
    else
    {
        fw.indent() << "shininess FRONT " << material.getShininess(Material::FRONT) << std::endl;
        fw.indent() << "shininess BACK  " << material.getShininess(Material::BACK) << std::endl;
    }

    return true;
}
