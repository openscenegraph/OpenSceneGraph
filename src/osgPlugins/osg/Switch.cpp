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
REGISTER_DOTOSGWRAPPER(Switch)
(
    new osg::Switch,
    "Switch",
    "Object Node Switch Group",
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
            sw.setAllChildrenOn();
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("ALL_CHILDREN_OFF"))
        {
            sw.setAllChildrenOff();
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].isInt())
        {
            unsigned int value;
            fr[1].getUInt(value);
            sw.setSingleChildOn(value);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    if (fr[0].matchWord("NewChildDefaultValue"))
    {
        if (fr[1].matchWord("TRUE")) 
        {
            sw.setNewChildDefaultValue(true);
            iteratorAdvanced = true;
            fr += 2;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            sw.setNewChildDefaultValue(false);
            iteratorAdvanced = true;
            fr += 2;
        }
        else if (fr[1].isInt())
        {
            int value;
            fr[1].getInt(value);
            sw.setNewChildDefaultValue(value!=0);
            iteratorAdvanced = true;
            fr += 2;
        }
    }

    if (fr.matchSequence("ValueList {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        // move inside the brakets.
        fr += 2;

        unsigned int pos=0;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            int value;
            if (fr[0].getInt(value))
            {
                sw.setValue(pos,value!=0);
                ++pos;
            }
            ++fr;
        }

        ++fr;
        
        iteratorAdvanced = true;
        
    }

    return iteratorAdvanced;
}


bool Switch_writeLocalData(const Object& obj, Output& fw)
{
    const Switch& sw = static_cast<const Switch&>(obj);


    fw.indent()<<"NewChildDefaultValue "<<sw.getNewChildDefaultValue()<<std::endl;

    fw.indent()<<"ValueList {"<< std::endl;
    fw.moveIn();
    const Switch::ValueList& values = sw.getValueList();
    for(Switch::ValueList::const_iterator itr=values.begin();
        itr!=values.end();
        ++itr)
    {
        fw.indent()<<*itr<<std::endl;
    }
    fw.moveOut();
    fw.indent()<<"}"<< std::endl;

    return true;
}
