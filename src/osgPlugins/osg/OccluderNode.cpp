#include "osg/OccluderNode"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool OccluderNode_readLocalData(Object& obj, Input& fr);
bool OccluderNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_OccluderNodeProxy
(
    osgNew osg::OccluderNode,
    "OccluderNode",
    "Object Node Group OccluderNode",
    &OccluderNode_readLocalData,
    &OccluderNode_writeLocalData
);

bool OccluderNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    OccluderNode& occludernode = static_cast<OccluderNode&>(obj);

    return iteratorAdvanced;
}


bool OccluderNode_writeLocalData(const Object& obj, Output& fw)
{
    const OccluderNode& occludenode = static_cast<const OccluderNode&>(obj);

    return true;
}
