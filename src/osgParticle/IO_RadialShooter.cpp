
#include <osgParticle/RadialShooter>
#include <osgParticle/range>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  RadialShooter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  RadialShooter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  RadialShooter_Proxy
(
    osgNew osgParticle::RadialShooter,
    "RadialShooter",
    "Object Shooter RadialShooter",
    RadialShooter_readLocalData,
    RadialShooter_writeLocalData
);

bool RadialShooter_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::RadialShooter &myobj = static_cast<osgParticle::RadialShooter &>(obj);
    bool itAdvanced = false;

    osgParticle::rangef r;

    if (fr[0].matchWord("thetaRange")) {
        if (fr[1].getFloat(r.min) && fr[2].getFloat(r.max)) {
            myobj.setThetaRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("phiRange")) {
        if (fr[1].getFloat(r.min) && fr[2].getFloat(r.max)) {
            myobj.setPhiRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("initialSpeedRange")) {
        if (fr[1].getFloat(r.min) && fr[2].getFloat(r.max)) {
            myobj.setInitialSpeedRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool RadialShooter_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::RadialShooter &myobj = static_cast<const osgParticle::RadialShooter &>(obj);
    osgParticle::rangef r;

    r = myobj.getThetaRange();
    fw.indent() << "thetaRange " << r.min << " " << r.max << std::endl;
    r = myobj.getPhiRange();
    fw.indent() << "phiRange " << r.min << " " << r.max << std::endl;
    r = myobj.getInitialSpeedRange();
    fw.indent() << "initialSpeedRange " << r.min << " " << r.max << std::endl;

    return true;
}
