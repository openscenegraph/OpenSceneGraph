
#include <osgParticle/ForceOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  ForceOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ForceOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  ForceOperator_Proxy
(
    new osgParticle::ForceOperator,
    "ForceOperator",
    "Object Operator ForceOperator",
    ForceOperator_readLocalData,
    ForceOperator_writeLocalData
);

bool ForceOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ForceOperator &fop = static_cast<osgParticle::ForceOperator &>(obj);
    bool itAdvanced = false;

    osg::Vec3 f;

    if (fr[0].matchWord("force")) {
        if (fr[1].getFloat(f.x()) && fr[2].getFloat(f.y()) && fr[3].getFloat(f.z())) {
            fop.setForce(f);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool ForceOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ForceOperator &fop = static_cast<const osgParticle::ForceOperator &>(obj);
    osg::Vec3 f = fop.getForce();
    fw.indent() << "force " << f.x() << " " << f.y() << " " << f.z() << std::endl;
    return true;
}
