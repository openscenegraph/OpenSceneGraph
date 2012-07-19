
#include <osgParticle/Emitter>

#include <iostream>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

extern bool  read_particle(osgDB::Input &fr, osgParticle::Particle &P);
extern void  write_particle(const osgParticle::Particle &P, osgDB::Output &fw);

bool  Emitter_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  Emitter_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(Emitter_Proxy)
(
    0,
    "Emitter",
    "Object Node ParticleProcessor Emitter",
    Emitter_readLocalData,
    Emitter_writeLocalData
);

bool Emitter_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::Emitter &myobj = static_cast<osgParticle::Emitter &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("useDefaultTemplate")) {
        if (fr[1].matchWord("TRUE")) {
            myobj.setUseDefaultTemplate(true);
            fr += 2;
            itAdvanced = true;
        }
        if (fr[1].matchWord("FALSE")) {
            myobj.setUseDefaultTemplate(false);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("particleTemplate")) {
        ++fr;
        itAdvanced = true;
        osgParticle::Particle P;
        if (read_particle(fr, P)) {
            myobj.setParticleTemplate(P);
        }
    }

    return itAdvanced;
}

bool Emitter_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::Emitter &myobj = static_cast<const osgParticle::Emitter &>(obj);

    fw.indent() << "useDefaultTemplate ";
    if (!myobj.getUseDefaultTemplate()) {
        fw << "FALSE" << std::endl;
        fw.indent() << "particleTemplate ";
        write_particle(myobj.getParticleTemplate(), fw);
        fw << std::endl;
    } else {
        fw << "TRUE" << std::endl;
    }

    return true;
}
