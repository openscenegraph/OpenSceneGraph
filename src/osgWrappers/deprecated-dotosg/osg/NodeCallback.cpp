#include <osg/Notify>

#include <osg/NodeCallback>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

bool  NodeCallback_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  NodeCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw); // register the read and write functions with the osgDB::Registry.

REGISTER_DOTOSGWRAPPER(NodeCallback)
(
    new NodeCallback,
    "NodeCallback",
    "Object NodeCallback",
    &NodeCallback_readLocalData,
    &NodeCallback_writeLocalData
);

bool NodeCallback_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    NodeCallback& nc = dynamic_cast<NodeCallback&>(obj);
    if (!(&nc)) return false;

    bool itrAdvanced = false;

    NodeCallback* ncc = fr.readObjectOfType<NodeCallback>();
    if (ncc)
    {
        nc.setNestedCallback(ncc);
        itrAdvanced = true;
    }

    return itrAdvanced;
}

bool NodeCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const NodeCallback* nc = dynamic_cast<const NodeCallback*>(&obj);
    if (!nc) return false;

    NodeCallback* nnc = (NodeCallback*) nc;

    if (nnc->getNestedCallback())
    {
        fw.writeObject(*(nnc->getNestedCallback()));
    }

    return true;
}
