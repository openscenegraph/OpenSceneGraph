
#include <osgParticle/VariableRateCounter>

#include <iostream>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  VariableRateCounter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  VariableRateCounter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(VariableRateCounter_Proxy)
(
    0,
    "VariableRateCounter",
    "Object Counter VariableRateCounter",
    VariableRateCounter_readLocalData,
    VariableRateCounter_writeLocalData
);

bool VariableRateCounter_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::VariableRateCounter &myobj = static_cast<osgParticle::VariableRateCounter &>(obj);
    bool itAdvanced = false;

    osgParticle::rangef r;
    if (fr[0].matchWord("rateRange")) {
        if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
            myobj.setRateRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool VariableRateCounter_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::VariableRateCounter &myobj = static_cast<const osgParticle::VariableRateCounter &>(obj);

    osgParticle::rangef r = myobj.getRateRange();
    fw.indent() << "rateRange " << r.minimum << " " << r.maximum << std::endl;

    return true;
}
