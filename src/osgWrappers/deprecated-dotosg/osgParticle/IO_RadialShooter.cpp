
#include <osgParticle/RadialShooter>
#include <osgParticle/range>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  RadialShooter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  RadialShooter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(RadialShooter_Proxy)
(
    new osgParticle::RadialShooter,
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
        if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
            myobj.setThetaRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("phiRange")) {
        if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
            myobj.setPhiRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("initialSpeedRange")) {
        if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
            myobj.setInitialSpeedRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("initialRotationalSpeedRange")) {
        osg::Vec3 r1;
        osg::Vec3 r2;
        if (fr[1].getFloat(r1.x()) && fr[2].getFloat(r1.y()) && fr[3].getFloat(r1.z()) && \
            fr[4].getFloat(r2.x()) && fr[5].getFloat(r2.y()) && fr[6].getFloat(r2.z())) {
            myobj.setInitialRotationalSpeedRange(r1,r2);
            fr += 7;
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
    fw.indent() << "thetaRange " << r.minimum << " " << r.maximum << std::endl;
    r = myobj.getPhiRange();
    fw.indent() << "phiRange " << r.minimum << " " << r.maximum << std::endl;
    r = myobj.getInitialSpeedRange();
    fw.indent() << "initialSpeedRange " << r.minimum << " " << r.maximum << std::endl;

    osgParticle::rangev3 rv = myobj.getInitialRotationalSpeedRange();
    osg::Vec3 v1 = rv.minimum;
    osg::Vec3 v2 = rv.maximum;
    fw.indent() << "initialRotationalSpeedRange ";
    fw << v1.x() << " " << v1.y() << " " << v1.z() << " ";
    fw << v2.x() << " " << v2.y() << " " << v2.z() << std::endl;

    return true;
}
