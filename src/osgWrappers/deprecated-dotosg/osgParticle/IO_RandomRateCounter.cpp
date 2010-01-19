
#include <osgParticle/RandomRateCounter>

#include <iostream>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  RandomRateCounter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  RandomRateCounter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(RandomRateCounter_Proxy)
(
    new osgParticle::RandomRateCounter,
    "RandomRateCounter",
    "Object Counter VariableRateCounter RandomRateCounter",
    RandomRateCounter_readLocalData,
    RandomRateCounter_writeLocalData
);

bool RandomRateCounter_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool RandomRateCounter_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
