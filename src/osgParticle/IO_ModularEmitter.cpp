
#include <osgParticle/ModularEmitter>
#include <osgParticle/Counter>
#include <osgParticle/Placer>
#include <osgParticle/Shooter>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  ModularEmitter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ModularEmitter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  ModularEmitter_Proxy
(
    osgNew osgParticle::ModularEmitter,
    "ModularEmitter",
    "Object Node ParticleProcessor Emitter ModularEmitter",
    ModularEmitter_readLocalData,
    ModularEmitter_writeLocalData
);

bool ModularEmitter_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ModularEmitter &myobj = static_cast<osgParticle::ModularEmitter &>(obj);
    bool itAdvanced = false;

    // we cannot use readObjectOfType() because the Coutner, Placer and Shooter classes are 
    // abstract and we can't create instances to use as prototypes.
    // So, we call readObject() and then dynamic cast to the desired class.

    osgParticle::Counter *counter = dynamic_cast<osgParticle::Counter *>(fr.readObject());
    if (counter) {
        myobj.setCounter(counter);
        itAdvanced = true;
    }
    
    osgParticle::Placer *placer = dynamic_cast<osgParticle::Placer *>(fr.readObject());
    if (placer) {
        myobj.setPlacer(placer);
        itAdvanced = true;
    }

    osgParticle::Shooter *shooter = dynamic_cast<osgParticle::Shooter *>(fr.readObject());
    if (shooter) {
        myobj.setShooter(shooter);
        itAdvanced = true;
    }

    return itAdvanced;
}

bool ModularEmitter_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ModularEmitter &myobj = static_cast<const osgParticle::ModularEmitter &>(obj);

    fw.writeObject(*myobj.getCounter());
    fw.writeObject(*myobj.getPlacer());
    fw.writeObject(*myobj.getShooter());

    return true;
}
