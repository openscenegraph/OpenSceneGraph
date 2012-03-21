
#include <osgParticle/ExplosionOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  ExplosionOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ExplosionOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ExplosionOperator_Proxy)
(
    new osgParticle::ExplosionOperator,
    "ExplosionOperator",
    "Object Operator ExplosionOperator",
    ExplosionOperator_readLocalData,
    ExplosionOperator_writeLocalData
);

bool ExplosionOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ExplosionOperator &ep = static_cast<osgParticle::ExplosionOperator &>(obj);
    bool itAdvanced = false;

    osg::Vec3 a;
    if (fr[0].matchWord("center")) {
        if (fr[1].getFloat(a.x()) && fr[2].getFloat(a.y()) && fr[3].getFloat(a.z())) {
            ep.setCenter(a);
            fr += 4;
            itAdvanced = true;
        }
    }

    float value = 0.0f;
    if (fr[0].matchWord("radius")) {
        if (fr[1].getFloat(value)) {
            ep.setRadius(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("magnitude")) {
        if (fr[1].getFloat(value)) {
            ep.setMagnitude(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("epsilon")) {
        if (fr[1].getFloat(value)) {
            ep.setEpsilon(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("sigma")) {
        if (fr[1].getFloat(value)) {
            ep.setSigma(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool ExplosionOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ExplosionOperator &ep = static_cast<const osgParticle::ExplosionOperator &>(obj);
    osg::Vec3 a = ep.getCenter();
    fw.indent() << "center " << a.x() << " " << a.y() << " " << a.z() << std::endl;

    fw.indent() << "radius " << ep.getRadius() << std::endl;
    fw.indent() << "magnitude " << ep.getMagnitude() << std::endl;
    fw.indent() << "epsilon " << ep.getEpsilon() << std::endl;
    fw.indent() << "sigma " << ep.getSigma() << std::endl;
    return true;
}
