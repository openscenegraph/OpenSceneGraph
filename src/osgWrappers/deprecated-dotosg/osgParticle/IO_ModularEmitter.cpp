
#include <osgParticle/ModularEmitter>
#include <osgParticle/Counter>
#include <osgParticle/Placer>
#include <osgParticle/Shooter>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  ModularEmitter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ModularEmitter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ModularEmitter_Proxy)
(
    new osgParticle::ModularEmitter,
    "ModularEmitter",
    "Object Node ParticleProcessor Emitter ModularEmitter",
    ModularEmitter_readLocalData,
    ModularEmitter_writeLocalData
);

bool ModularEmitter_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ModularEmitter &myobj = static_cast<osgParticle::ModularEmitter &>(obj);
    bool itAdvanced = false;

    osgParticle::Counter *counter = static_cast<osgParticle::Counter *>(fr.readObjectOfType(osgDB::type_wrapper<osgParticle::Counter>()));
    if (counter) {
        myobj.setCounter(counter);
        itAdvanced = true;
    }

    osgParticle::Placer *placer = static_cast<osgParticle::Placer *>(fr.readObjectOfType(osgDB::type_wrapper<osgParticle::Placer>()));
    if (placer) {
        myobj.setPlacer(placer);
        itAdvanced = true;
    }

    osgParticle::Shooter *shooter = static_cast<osgParticle::Shooter *>(fr.readObjectOfType(osgDB::type_wrapper<osgParticle::Shooter>()));
    if (shooter) {
        myobj.setShooter(shooter);
        itAdvanced = true;
    }

    return itAdvanced;
}

bool ModularEmitter_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ModularEmitter &myobj = static_cast<const osgParticle::ModularEmitter &>(obj);

    if (myobj.getCounter()) fw.writeObject(*myobj.getCounter());
    if (myobj.getPlacer()) fw.writeObject(*myobj.getPlacer());
    if (myobj.getShooter()) fw.writeObject(*myobj.getShooter());

    return true;
}
