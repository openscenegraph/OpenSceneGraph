
#include <osgParticle/ParticleSystemUpdater>

#include <osg/ref_ptr>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  PSU_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  PSU_writeLocalData(const osg::Object &obj, osgDB::Output &fr);

osgDB::RegisterDotOsgWrapperProxy PSU_Proxy
(
    new osgParticle::ParticleSystemUpdater,
    "ParticleSystemUpdater",
    "Object Node ParticleSystemUpdater",
    PSU_readLocalData,
    PSU_writeLocalData
);

bool PSU_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ParticleSystemUpdater &myobj = static_cast<osgParticle::ParticleSystemUpdater &>(obj);    
    bool itAdvanced = false;

    osg::ref_ptr<osgParticle::ParticleSystem> proto = new osgParticle::ParticleSystem;
    osgParticle::ParticleSystem *ps = static_cast<osgParticle::ParticleSystem *>(fr.readObjectOfType(*proto));
    if (ps) {
        myobj.addParticleSystem(ps);
        itAdvanced = true;
    }

    return itAdvanced;
}

bool PSU_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ParticleSystemUpdater &myobj = static_cast<const osgParticle::ParticleSystemUpdater &>(obj);    

    for (unsigned int i=0; i<myobj.getNumParticleSystems(); ++i) {
        fw.writeObject(*myobj.getParticleSystem(i));
    }

    return true;
}
