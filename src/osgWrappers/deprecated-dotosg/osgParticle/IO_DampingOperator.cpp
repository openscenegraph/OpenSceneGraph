
#include <osgParticle/DampingOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  DampingOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  DampingOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(DampingOperator_Proxy)
(
    new osgParticle::DampingOperator,
    "DampingOperator",
    "Object Operator DampingOperator",
    DampingOperator_readLocalData,
    DampingOperator_writeLocalData
);

bool DampingOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::DampingOperator &dp = static_cast<osgParticle::DampingOperator &>(obj);
    bool itAdvanced = false;

    osg::Vec3 a;
    if (fr[0].matchWord("damping")) {
        if (fr[1].getFloat(a.x()) && fr[2].getFloat(a.y()) && fr[3].getFloat(a.z())) {
            dp.setDamping(a);
            fr += 4;
            itAdvanced = true;
        }
    }

    float low = 0.0f, high = FLT_MAX;
    if (fr[0].matchWord("cutoff")) {
        if (fr[1].getFloat(low) && fr[2].getFloat(high)) {
            dp.setCutoff(low, high);
            fr += 3;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool DampingOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::DampingOperator &dp = static_cast<const osgParticle::DampingOperator &>(obj);
    osg::Vec3 a = dp.getDamping();
    fw.indent() << "damping " << a.x() << " " << a.y() << " " << a.z() << std::endl;

    float low = dp.getCutoffLow(), high = dp.getCutoffHigh();
    fw.indent() << "cutoff " << low << " " << high << std::endl;
    return true;
}
