
#include <osgParticle/ParticleProcessor>
#include <osgParticle/ParticleSystem>

#include <iostream>

#include <osg/ref_ptr>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

extern bool  read_particle(osgDB::Input &fr, osgParticle::Particle &P);
extern void  write_particle(const osgParticle::Particle &P, osgDB::Output &fw);

bool  ParticleProcessor_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ParticleProcessor_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  ParticleProcessor_Proxy
(
    0,
    "ParticleProcessor",
    "Object Node ParticleProcessor",
    ParticleProcessor_readLocalData,
    ParticleProcessor_writeLocalData
);

bool ParticleProcessor_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ParticleProcessor &myobj = static_cast<osgParticle::ParticleProcessor &>(obj);
    bool itAdvanced = false;

    osg::ref_ptr<osgParticle::ParticleSystem> ps_proto = osgNew osgParticle::ParticleSystem;
    
    osgParticle::ParticleSystem *ps = static_cast<osgParticle::ParticleSystem *>(fr.readObjectOfType(*ps_proto));
    if (ps) {
        myobj.setParticleSystem(ps);
        itAdvanced = true;
    } 

    if (fr[0].matchWord("enabled")) {
        if (fr[1].matchWord("TRUE")) {
            myobj.setEnabled(true);
            fr += 2;
            itAdvanced = true;
        } else if (fr[1].matchWord("FALSE")) {
            myobj.setEnabled(false);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("referenceFrame")) {
        if (fr[1].matchWord("RELATIVE_TO_ABSOLUTE")) {
            myobj.setReferenceFrame(osgParticle::ParticleProcessor::RELATIVE_TO_ABSOLUTE);
            fr += 2;
            itAdvanced = true;
        }
        if (fr[1].matchWord("RELATIVE_TO_PARENTS")) {
            myobj.setReferenceFrame(osgParticle::ParticleProcessor::RELATIVE_TO_PARENTS);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool ParticleProcessor_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ParticleProcessor &myobj = static_cast<const osgParticle::ParticleProcessor &>(obj);

    if (myobj.getParticleSystem()) fw.writeObject(*myobj.getParticleSystem());

    fw.indent() << "enabled ";
    if (myobj.isEnabled()) 
        fw << "TRUE" << std::endl;
    else
        fw << "FALSE" << std::endl;

    fw.indent() << "referenceFrame ";
    switch (myobj.getReferenceFrame())
    {
    case osgParticle::ParticleProcessor::RELATIVE_TO_ABSOLUTE:
        fw << "RELATIVE_TO_ABSOLUTE" << std::endl;
        break;
    case osgParticle::ParticleProcessor::RELATIVE_TO_PARENTS:
    default:
        fw << "RELATIVE_TO_PARENTS" << std::endl;
    }

    return true;
}
