#include "osg/Switch"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Switch_readLocalData(Object& obj, Input& fr);
bool Switch_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_SwitchProxy
(
    new osg::Switch,
    "Switch",
    "Object Node Group Switch",
    &Switch_readLocalData,
    &Switch_writeLocalData
);

bool Switch_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Switch& sw = static_cast<Switch&>(obj);

    if (fr.matchSequence("value"))
    {
        if (fr[1].matchWord("ALL_CHILDREN_ON"))
        {
            sw.setValue(Switch::ALL_CHILDREN_ON);
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("ALL_CHILDREN_OFF"))
        {
            sw.setValue(Switch::ALL_CHILDREN_OFF);
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].isInt())
        {
            int value;
            fr[1].getInt(value);
            sw.setValue(value);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    return iteratorAdvanced;
}


bool Switch_writeLocalData(const Object& obj, Output& fw)
{
    const Switch& sw = static_cast<const Switch&>(obj);

    fw.indent() << "value ";
    switch(sw.getValue())
    {
        case(Switch::ALL_CHILDREN_ON): fw<<"ALL_CHILDREN_ON"<< std::endl;break;
        case(Switch::ALL_CHILDREN_OFF): fw<<"ALL_CHILDREN_OFF"<< std::endl;break;
        default: fw<<sw.getValue()<< std::endl;break;
    }

    return true;
}
