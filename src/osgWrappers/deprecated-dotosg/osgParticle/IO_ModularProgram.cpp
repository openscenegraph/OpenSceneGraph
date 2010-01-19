
#include <osgParticle/ModularProgram>
#include <osgParticle/Operator>

#include <iostream>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  ModularProgram_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ModularProgram_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ModularProgram_Proxy)
(
    new osgParticle::ModularProgram,
    "ModularProgram",
    "Object Node ParticleProcessor osgParticle::Program ModularProgram",
    ModularProgram_readLocalData,
    ModularProgram_writeLocalData
);

bool ModularProgram_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ModularProgram &myobj = static_cast<osgParticle::ModularProgram &>(obj);
    bool itAdvanced = false;

    osgParticle::Operator *op = static_cast<osgParticle::Operator *>(fr.readObjectOfType(osgDB::type_wrapper<osgParticle::Operator>()));
    if (op) {
        myobj.addOperator(op);
        itAdvanced = true;
    }

    return itAdvanced;
}

bool ModularProgram_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ModularProgram &myobj = static_cast<const osgParticle::ModularProgram &>(obj);

    for (int i=0; i<myobj.numOperators(); ++i) {
        fw.writeObject(*myobj.getOperator(i));
    }

    return true;
}
