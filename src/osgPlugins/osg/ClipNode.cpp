#include "osg/ClipNode"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ClipNode_readLocalData(Object& obj, Input& fr);
bool ClipNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ClipNodeProxy
(
    new osg::ClipNode,
    "ClipNode",
    "Object Node ClipNode Group",
    &ClipNode_readLocalData,
    &ClipNode_writeLocalData
);

bool ClipNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ClipNode& clipnode = static_cast<ClipNode&>(obj);

    osg::ref_ptr<StateAttribute> sa=0;
    while((sa=fr.readStateAttribute())!=0)
    {
        ClipPlane* clipplane = dynamic_cast<ClipPlane*>(sa.get());
        if (clipplane) clipnode.addClipPlane(clipplane);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool ClipNode_writeLocalData(const Object& obj, Output& fw)
{
    const ClipNode& clipnode = static_cast<const ClipNode&>(obj);

    for(unsigned  int i=0;i<clipnode.getNumClipPlanes();++i)
    {
        fw.writeObject(*clipnode.getClipPlane(i));
    }

    return true;
}
