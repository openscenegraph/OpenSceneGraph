#include <osg/ClusterCullingCallback>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

bool  ClusterCullingCallback_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ClusterCullingCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw); // register the read and write functions with the osgDB::Registry.

REGISTER_DOTOSGWRAPPER(ClusterCullingCallback)
(
    new ClusterCullingCallback,
    "ClusterCullingCallback",
    "Object ClusterCullingCallback",
    &ClusterCullingCallback_readLocalData,
    &ClusterCullingCallback_writeLocalData
);

bool ClusterCullingCallback_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    ClusterCullingCallback* ccc = dynamic_cast<ClusterCullingCallback*>(&obj);
    if (!ccc) return false; 

    bool iteratorAdvanced = false;

    osg::Vec3 vec;
    if (fr[0].matchWord("controlPoint") &&
        fr[1].getFloat(vec[0]) && fr[2].getFloat(vec[1]) && fr[3].getFloat(vec[2])) 
    {
        ccc->setControlPoint(vec);
        fr += 4;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("normal") &&
        fr[1].getFloat(vec[0]) && fr[2].getFloat(vec[1]) && fr[3].getFloat(vec[2])) 
    {
        ccc->setNormal(vec);
        fr += 4;
        iteratorAdvanced = true;
    }

    float value;
    if (fr[0].matchWord("radius") && fr[1].getFloat(value)) 
    {
        ccc->setRadius(value);
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("deviation") && fr[1].getFloat(value)) 
    {
        ccc->setDeviation(value);
        fr += 2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool ClusterCullingCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const ClusterCullingCallback* ccc = dynamic_cast<const ClusterCullingCallback*>(&obj);
    if (!ccc) return false; 

    int prec = fw.precision();
    fw.precision(15);

    fw.indent() << "controlPoint " << ccc->getControlPoint() << std::endl;
    fw.indent() << "normal " << ccc->getNormal() << std::endl;
    fw.indent() << "radius " << ccc->getRadius() << std::endl;
    fw.indent() << "deviation " << ccc->getDeviation() << std::endl;
 
    fw.precision(prec);

    return true;
}
