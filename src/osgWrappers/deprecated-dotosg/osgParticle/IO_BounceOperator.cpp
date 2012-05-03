
#include <osgParticle/BounceOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  BounceOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  BounceOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(BounceOperator_Proxy)
(
    new osgParticle::BounceOperator,
    "BounceOperator",
    "Object Operator DomainOperator BounceOperator",
    BounceOperator_readLocalData,
    BounceOperator_writeLocalData
);

bool BounceOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::BounceOperator &bp = static_cast<osgParticle::BounceOperator &>(obj);
    bool itAdvanced = false;

    float value = 0.0f;
    if (fr[0].matchWord("friction")) {
        if (fr[1].getFloat(value)) {
            bp.setFriction(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("resilience")) {
        if (fr[1].getFloat(value)) {
            bp.setResilience(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("cutoff")) {
        if (fr[1].getFloat(value)) {
            bp.setCutoff(value);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool BounceOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::BounceOperator &bp = static_cast<const osgParticle::BounceOperator &>(obj);
    fw.indent() << "friction " << bp.getFriction() << std::endl;
    fw.indent() << "resilience " << bp.getResilience() << std::endl;
    fw.indent() << "cutoff " << bp.getCutoff() << std::endl;
    return true;
}
