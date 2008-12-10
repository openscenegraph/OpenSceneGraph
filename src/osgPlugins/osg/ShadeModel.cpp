#include "osg/ShadeModel"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ShadeModel_readLocalData(Object& obj, Input& fr);
bool ShadeModel_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(ShadeModel)
(
    new osg::ShadeModel,
    "ShadeModel",
    "Object StateAttribute ShadeModel",
    &ShadeModel_readLocalData,
    &ShadeModel_writeLocalData
);


bool ShadeModel_readLocalData(Object& obj,Input& fr)
{
    bool iteratorAdvanced = false;

    ShadeModel& shademodel = static_cast<ShadeModel&>(obj);

    if (fr[0].matchWord("mode"))
    {
        if (fr[1].matchWord("FLAT"))
        {
            shademodel.setMode(ShadeModel::FLAT);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("SMOOTH"))
        {
            shademodel.setMode(ShadeModel::SMOOTH);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool ShadeModel_writeLocalData(const Object& obj, Output& fw)
{

    const ShadeModel& shademodel = static_cast<const ShadeModel&>(obj);

    switch(shademodel.getMode())
    {
        case(ShadeModel::FLAT):     fw.indent() << "mode FLAT" <<std::endl; break;
        case(ShadeModel::SMOOTH):   fw.indent() << "mode SMOOTH" <<std::endl; break;
    }
    return true;
}
