
#include <osgParticle/AccelOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  AccelOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  AccelOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  AccelOperator_Proxy
(
    new osgParticle::AccelOperator,
    "AccelOperator",
    "Object Operator AccelOperator",
    AccelOperator_readLocalData,
    AccelOperator_writeLocalData
);

bool AccelOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::AccelOperator &aop = static_cast<osgParticle::AccelOperator &>(obj);
    bool itAdvanced = false;

    osg::Vec3 a;

    if (fr[0].matchWord("acceleration")) {
        if (fr[1].getFloat(a.x()) && fr[2].getFloat(a.y()) && fr[3].getFloat(a.z())) {
            aop.setAcceleration(a);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool AccelOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::AccelOperator &aop = static_cast<const osgParticle::AccelOperator &>(obj);
    osg::Vec3 a = aop.getAcceleration();
    fw.indent() << "acceleration " << a.x() << " " << a.y() << " " << a.z() << std::endl;
    return true;
}
