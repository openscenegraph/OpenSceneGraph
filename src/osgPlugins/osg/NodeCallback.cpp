#include <osg/Notify>

#include <osg/NodeCallback>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

bool  NodeCallback_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  NodeCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw); // register the read and write functions with the osgDB::Registry.

osgDB::RegisterDotOsgWrapperProxy  NodeCallback_Proxy
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

    static osg::ref_ptr<NodeCallback> s_nc = new NodeCallback;
    osg::ref_ptr<osg::Object> object = fr.readObjectOfType(*s_nc);
    if (object.valid())
    {
        NodeCallback* ncc = dynamic_cast<NodeCallback*>(object.get());
        if (ncc) nc.setNestedCallback(ncc);
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
