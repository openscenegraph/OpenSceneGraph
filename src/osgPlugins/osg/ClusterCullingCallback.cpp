#include <osg/Notify>

#include <osg/ClusterCullingCallback>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

bool  ClusterCullingCallback_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ClusterCullingCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw); // register the read and write functions with the osgDB::Registry.

osgDB::RegisterDotOsgWrapperProxy  ClusterCullingCallback_Proxy
(
    new ClusterCullingCallback,
    "ClusterCullingCallback",
    "Object ClusterCullingCallback",
    &ClusterCullingCallback_readLocalData,
    &ClusterCullingCallback_writeLocalData
);

bool ClusterCullingCallback_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    ClusterCullingCallback& nc = dynamic_cast<ClusterCullingCallback&>(obj);
    if (!(&nc)) return false;

    bool itrAdvanced = false;


    return itrAdvanced;
}

bool ClusterCullingCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const ClusterCullingCallback* nc = dynamic_cast<const ClusterCullingCallback*>(&obj);
    if (!nc) return false;

    ClusterCullingCallback* nnc = (ClusterCullingCallback*) nc;

    return true;
}
