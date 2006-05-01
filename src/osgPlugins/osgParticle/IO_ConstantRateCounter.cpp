#include <osgParticle/ConstantRateCounter>

#include <iostream>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  ConstantRateCounter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ConstantRateCounter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  ConstantRateCounter_Proxy
(
    new osgParticle::ConstantRateCounter,
    "ConstantRateCounter",
    "Object Counter ConstantRateCounter",
    ConstantRateCounter_readLocalData,
    ConstantRateCounter_writeLocalData
);

bool ConstantRateCounter_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ConstantRateCounter &myobj = static_cast<osgParticle::ConstantRateCounter &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("minimumNumberOfParticlesToCreate")) {
        int v;
        if (fr[1].getInt(v)) {
            myobj.setMinimumNumberOfParticlesToCreate(v);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("numberOfParticlesPerSecondToCreate")) {
        float v;
        if (fr[1].getFloat(v)) {
            myobj.setNumberOfParticlesPerSecondToCreate(v);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool ConstantRateCounter_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ConstantRateCounter &myobj = static_cast<const osgParticle::ConstantRateCounter &>(obj);

    fw.indent() << "minimumNumberOfParticlesToCreate " << myobj.getMinimumNumberOfParticlesToCreate() << std::endl;
    fw.indent() << "numberOfParticlesPerSecondToCreate " << myobj.getNumberOfParticlesPerSecondToCreate() << std::endl;
    
    return true;
}
