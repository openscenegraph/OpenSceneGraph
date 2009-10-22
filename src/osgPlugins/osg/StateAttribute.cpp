#include "osg/StateAttribute"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool StateAttribute_readLocalData(Object& obj, Input& fr);
bool StateAttribute_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
osg::StateAttribute* g_stateAttribute = 0; 
REGISTER_DOTOSGWRAPPER(StateAttribute)
(
    g_stateAttribute, // no instance, osg::StateAttribute is an abstract class.
    "StateAttribute",
    "Object StateAttribute",
    &StateAttribute_readLocalData,
    &StateAttribute_writeLocalData
);


bool StateAttribute_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;
    StateAttribute& stateAttribute = static_cast<StateAttribute&>(obj);

    static ref_ptr<StateAttributeCallback> s_callback = new osg::StateAttributeCallback;
    while (fr.matchSequence("UpdateCallback {"))
    {
        //int entry = fr[0].getNoNestedBrackets();
        fr += 2;
        StateAttributeCallback* callback = dynamic_cast<StateAttributeCallback*>(fr.readObjectOfType(*s_callback));
        if (callback) {
            stateAttribute.setUpdateCallback(callback);
        }
        iteratorAdvanced = true;
    }

    while (fr.matchSequence("EventCallback {"))
    {
        //int entry = fr[0].getNoNestedBrackets();
        fr += 2;
        StateAttributeCallback* callback = dynamic_cast<StateAttributeCallback*>(fr.readObjectOfType(*s_callback));
        if (callback) {
            stateAttribute.setEventCallback(callback);
        }
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool StateAttribute_writeLocalData(const Object& obj,Output& fw)
{
    const StateAttribute& stateAttribute = static_cast<const StateAttribute&>(obj);

    if (stateAttribute.getUpdateCallback())
    {
        fw.indent() << "UpdateCallback {" << std::endl;
        fw.moveIn();
        fw.writeObject(*stateAttribute.getUpdateCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    if (stateAttribute.getEventCallback())
    {
        fw.indent() << "EventCallback {" << std::endl;
        fw.moveIn();
        fw.writeObject(*stateAttribute.getEventCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    return true;
}
