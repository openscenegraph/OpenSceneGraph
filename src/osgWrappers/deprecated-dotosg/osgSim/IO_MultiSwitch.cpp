#include "osgSim/MultiSwitch"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgSim;
using namespace osgDB;

// forward declare functions to use later.
bool MultiSwitch_readLocalData(Object& obj, Input& fr);
bool MultiSwitch_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_simSwitchProxy
(
    new osgSim::MultiSwitch,
    "MultiSwitch",
    "Object Node MultiSwitch Group",
    &MultiSwitch_readLocalData,
    &MultiSwitch_writeLocalData
);

bool MultiSwitch_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    MultiSwitch& sw = static_cast<MultiSwitch&>(obj);

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

    if (fr.matchSequence("ActiveSwitchSet %i"))
    {
        unsigned int switchSet;
        fr[1].getUInt(switchSet);
        fr+=2;
        
        sw.setActiveSwitchSet(switchSet);
    }


    if (fr.matchSequence("ValueList %i {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        unsigned int switchSet;
        fr[1].getUInt(switchSet);

        // move inside the brakets.
        fr += 3;

        unsigned int pos=0;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            int value;
            if (fr[0].getInt(value))
            {
                sw.setValue(switchSet, pos,value!=0);
                ++pos;
            }
            ++fr;
        }

        ++fr;
        
        iteratorAdvanced = true;
        
    }

    return iteratorAdvanced;
}


bool MultiSwitch_writeLocalData(const Object& obj, Output& fw)
{
    const MultiSwitch& sw = static_cast<const MultiSwitch&>(obj);


    fw.indent()<<"NewChildDefaultValue "<<sw.getNewChildDefaultValue()<<std::endl;

    fw.indent()<<"ActiveSwitchSet "<<sw.getActiveSwitchSet()<<std::endl;

    unsigned int pos = 0;
    const osgSim::MultiSwitch::SwitchSetList& switchset = sw.getSwitchSetList();
    for(osgSim::MultiSwitch::SwitchSetList::const_iterator sitr=switchset.begin();
        sitr!=switchset.end();
        ++sitr,++pos)
    {
        fw.indent()<<"ValueList "<<pos<<" {"<< std::endl;
        fw.moveIn();
        const MultiSwitch::ValueList& values = *sitr;
        for(MultiSwitch::ValueList::const_iterator itr=values.begin();
            itr!=values.end();
            ++itr)
        {
            fw.indent()<<*itr<<std::endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }
    return true;
}
