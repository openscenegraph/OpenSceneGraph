
#include <osgParticle/OrbitOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  OrbitOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  OrbitOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(OrbitOperator_Proxy)
(
    new osgParticle::OrbitOperator,
    "OrbitOperator",
    "Object Operator OrbitOperator",
    OrbitOperator_readLocalData,
    OrbitOperator_writeLocalData
);

bool OrbitOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::OrbitOperator &op = static_cast<osgParticle::OrbitOperator &>(obj);
    bool itAdvanced = false;

    osg::Vec3 a;
    if (fr[0].matchWord("center")) {
        if (fr[1].getFloat(a.x()) && fr[2].getFloat(a.y()) && fr[3].getFloat(a.z())) {
            op.setCenter(a);
            fr += 4;
            itAdvanced = true;
        }
    }

    float value = 0.0f;
    if (fr[0].matchWord("magnitude")) {
        if (fr[1].getFloat(value)) {
            op.setMagnitude(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("epsilon")) {
        if (fr[1].getFloat(value)) {
            op.setEpsilon(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("maxRadius")) {
        if (fr[1].getFloat(value)) {
            op.setMaxRadius(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool OrbitOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::OrbitOperator &op = static_cast<const osgParticle::OrbitOperator &>(obj);
    osg::Vec3 a = op.getCenter();
    fw.indent() << "center " << a.x() << " " << a.y() << " " << a.z() << std::endl;

    fw.indent() << "magnitude " << op.getMagnitude() << std::endl;
    fw.indent() << "epsilon " << op.getEpsilon() << std::endl;
    fw.indent() << "maxRadius " << op.getMaxRadius() << std::endl;
    return true;
}
