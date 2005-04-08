#include <osg/LightModel>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool LightModel_readLocalData(Object& obj, Input& fr);
bool LightModel_writeLocalData(const Object& obj, Output& fw);


// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_LightModelProxy
(
    new osg::LightModel,
    "LightModel",
    "Object StateAttribute LightModel",
    &LightModel_readLocalData,
    &LightModel_writeLocalData
);


bool LightModel_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    LightModel& lightmodel = static_cast<LightModel&>(obj);
    
    osg::Vec4 ambient;
    if (fr[0].matchWord("ambientIntensity") &&
        fr[1].getFloat(ambient[0]) &&
        fr[2].getFloat(ambient[1]) &&
        fr[3].getFloat(ambient[2]) &&
        fr[4].getFloat(ambient[3]))
    {
        lightmodel.setAmbientIntensity(ambient);
        fr+=5;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("colorControl"))
    {
        if (fr[1].matchWord("SEPARATE_SPECULAR_COLOR"))
        {
            lightmodel.setColorControl(osg::LightModel::SEPARATE_SPECULAR_COLOR);
        }
        else if (fr[1].matchWord("SINGLE_COLOR"))
        {
            lightmodel.setColorControl(osg::LightModel::SINGLE_COLOR);
        }
    }

    int result;
    if (fr[0].matchWord("localViewer") && fr[1].getInt(result))
    {
        if (fr[1].matchWord("TRUE"))
        {
            lightmodel.setLocalViewer(true);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            lightmodel.setLocalViewer(false);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("twoSided"))
    {
        if (fr[1].matchWord("TRUE"))
        {
            lightmodel.setTwoSided(true);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            lightmodel.setTwoSided(false);
            fr+=2;
            iteratorAdvanced = true;
        }
    }
    
    return iteratorAdvanced;
}

bool LightModel_writeLocalData(const Object& obj,Output& fw)
{
    const LightModel& lightmodel = static_cast<const LightModel&>(obj);

    fw.indent() << "ambientIntensity " << lightmodel.getAmbientIntensity() << std::endl;
    
    if (lightmodel.getColorControl()==osg::LightModel::SEPARATE_SPECULAR_COLOR)
        fw.indent() << "colorControl SEPARATE_SPECULAR_COLOR" << std::endl;
    else
        fw.indent() << "colorControl SINGLE_COLOR" << std::endl;

    if (lightmodel.getLocalViewer())
        fw.indent() << "localViewer TRUE"<< std::endl;
    else
        fw.indent() << "localViewer FALSE"<< std::endl;
        
    if (lightmodel.getTwoSided())
        fw.indent() << "twoSided TRUE"<< std::endl;
    else
        fw.indent() << "twoSided FALSE"<< std::endl;

    return true;
}

