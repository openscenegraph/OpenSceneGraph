
#include <osgParticle/AngularDampingOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  AngularDampingOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  AngularDampingOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(AngularDampingOperator_Proxy)
(
    new osgParticle::AngularDampingOperator,
    "AngularDampingOperator",
    "Object Operator AngularDampingOperator",
    AngularDampingOperator_readLocalData,
    AngularDampingOperator_writeLocalData
);

bool AngularDampingOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::AngularDampingOperator &adp = static_cast<osgParticle::AngularDampingOperator &>(obj);
    bool itAdvanced = false;

    osg::Vec3 a;
    if (fr[0].matchWord("damping")) {
        if (fr[1].getFloat(a.x()) && fr[2].getFloat(a.y()) && fr[3].getFloat(a.z())) {
            adp.setDamping(a);
            fr += 4;
            itAdvanced = true;
        }
    }

    float low = 0.0f, high = FLT_MAX;
    if (fr[0].matchWord("cutoff")) {
        if (fr[1].getFloat(low) && fr[2].getFloat(high)) {
            adp.setCutoff(low, high);
            fr += 3;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool AngularDampingOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::AngularDampingOperator &adp = static_cast<const osgParticle::AngularDampingOperator &>(obj);
    osg::Vec3 a = adp.getDamping();
    fw.indent() << "damping " << a.x() << " " << a.y() << " " << a.z() << std::endl;

    float low = adp.getCutoffLow(), high = adp.getCutoffHigh();
    fw.indent() << "cutoff " << low << " " << high << std::endl;
    return true;
}
