
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

    osg::ref_ptr<osgParticle::ParticleSystem> ps_proto = new osgParticle::ParticleSystem;
    
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
        if (fr[1].matchWord("RELATIVE_TO_ABSOLUTE") || fr[1].matchWord("ABSOLUTE")) {
            myobj.setReferenceFrame(osgParticle::ParticleProcessor::ABSOLUTE_RF);
            fr += 2;
            itAdvanced = true;
        }
        if (fr[1].matchWord("RELATIVE_TO_PARENTS") || fr[1].matchWord("RELATIVE")) {
            myobj.setReferenceFrame(osgParticle::ParticleProcessor::RELATIVE_RF);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("endless")) {
        if (fr[1].matchWord("TRUE")) {
            myobj.setEndless(true);
            fr += 2;
            itAdvanced = true;
        } else if (fr[1].matchWord("FALSE")) {
            myobj.setEndless(false);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("lifeTime")) {
        float lt;
        if (fr[1].getFloat(lt)) {
            myobj.setLifeTime(lt);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("startTime")) {
        float st;
        if (fr[1].getFloat(st)) {
            myobj.setStartTime(st);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("currentTime")) {
        float ct;
        if (fr[1].getFloat(ct)) {
            myobj.setCurrentTime(ct);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("resetTime")) {
        float ct;
        if (fr[1].getFloat(ct)) {
            myobj.setResetTime(ct);
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
    case osgParticle::ParticleProcessor::ABSOLUTE_RF:
        fw << "ABSOLUTE" << std::endl;
        break;
    case osgParticle::ParticleProcessor::RELATIVE_RF:
    default:
        fw << "RELATIVE" << std::endl;
    }

    fw.indent() << "endless ";
    if (myobj.isEndless())
        fw << "TRUE" << std::endl;
    else
        fw << "FALSE" << std::endl;

    fw.indent() << "lifeTime " << myobj.getLifeTime() << std::endl;
    fw.indent() << "startTime " << myobj.getStartTime() << std::endl;
    fw.indent() << "currentTime " << myobj.getCurrentTime() << std::endl;
    fw.indent() << "resetTime " << myobj.getResetTime() << std::endl;

    return true;
}
