
#include <osgParticle/AngularAccelOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  AngularAccelOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  AngularAccelOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(AngularAccelOperator_Proxy)
(
    new osgParticle::AngularAccelOperator,
    "AngularAccelOperator",
    "Object Operator AngularAccelOperator",
    AngularAccelOperator_readLocalData,
    AngularAccelOperator_writeLocalData
);

bool AngularAccelOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::AngularAccelOperator &aop = static_cast<osgParticle::AngularAccelOperator &>(obj);
    bool itAdvanced = false;

    osg::Vec3 a;

    if (fr[0].matchWord("angularAcceleration")) {
        if (fr[1].getFloat(a.x()) && fr[2].getFloat(a.y()) && fr[3].getFloat(a.z())) {
            aop.setAngularAcceleration(a);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool AngularAccelOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::AngularAccelOperator &aop = static_cast<const osgParticle::AngularAccelOperator &>(obj);
    osg::Vec3 a = aop.getAngularAcceleration();
    fw.indent() << "angularAcceleration " << a.x() << " " << a.y() << " " << a.z() << std::endl;
    return true;
}
