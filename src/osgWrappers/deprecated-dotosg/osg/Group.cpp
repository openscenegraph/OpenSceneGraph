#include "osg/Group"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Group_readLocalData(Object& obj, Input& fr);
bool Group_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Group)
(
    new osg::Group,
    "Group",
    "Object Node Group",
    &Group_readLocalData,
    &Group_writeLocalData
);

bool Group_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Group& group = static_cast<Group&>(obj);

    int num_children;
    if (fr[0].matchWord("num_children") &&
        fr[1].getInt(num_children))
    {
        // could allocate space for children here...
        fr+=2;
        iteratorAdvanced = true;
    }

    Node* node = NULL;
    while((node=fr.readNode())!=NULL)
    {
        group.addChild(node);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Group_writeLocalData(const Object& obj, Output& fw)
{
    const Group& group = static_cast<const Group&>(obj);

    if (group.getNumChildren()!=0) fw.indent() << "num_children " << group.getNumChildren() << std::endl;

    for(unsigned int i=0;i<group.getNumChildren();++i)
    {
        fw.writeObject(*group.getChild(i));
    }
    return true;
}
