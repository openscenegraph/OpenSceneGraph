
#include <osgParticle/ParticleSystem>

#include <osg/BoundingBox>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <iostream>
#include <string>

extern bool  read_particle(osgDB::Input &fr, osgParticle::Particle &P);
extern void  write_particle(const osgParticle::Particle &P, osgDB::Output &fw);

bool  ParticleSystem_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ParticleSystem_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  ParticleSystem_Proxy
(
    new osgParticle::ParticleSystem,
    "ParticleSystem",
    "Object Drawable ParticleSystem",
    ParticleSystem_readLocalData,
    ParticleSystem_writeLocalData
);

bool ParticleSystem_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::ParticleSystem &myobj = static_cast<osgParticle::ParticleSystem &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("particleAlignment")) {
        if (fr[1].matchWord("BILLBOARD")) {
            myobj.setParticleAlignment(osgParticle::ParticleSystem::BILLBOARD);
            fr += 2;
            itAdvanced = true;
        }
        if (fr[1].matchWord("FIXED")) {
            myobj.setParticleAlignment(osgParticle::ParticleSystem::FIXED);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("particleScaleReferenceFrame")) {
        if (fr[1].matchWord("LOCAL_COORDINATES")) {
            myobj.setParticleScaleReferenceFrame(osgParticle::ParticleSystem::LOCAL_COORDINATES);
            fr += 2;
            itAdvanced = true;
        }
        if (fr[1].matchWord("")) {
            myobj.setParticleScaleReferenceFrame(osgParticle::ParticleSystem::WORLD_COORDINATES);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("alignVectorX")) {
        osg::Vec3 v;
        if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
            myobj.setAlignVectorX(v);
            fr += 4;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("alignVectorY")) {
        osg::Vec3 v;
        if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
            myobj.setAlignVectorY(v);
            fr += 4;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("doublePassRendering")) {
        if (fr[1].matchWord("TRUE")) {
            myobj.setDoublePassRendering(true);
            fr += 2;
            itAdvanced = true;
        } else if (fr[1].matchWord("FALSE")) {
            myobj.setDoublePassRendering(false);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("frozen")) {
        if (fr[1].matchWord("TRUE")) {
            myobj.setFrozen(true);
            fr += 2;
            itAdvanced = true;
        } else if (fr[1].matchWord("FALSE")) {
            myobj.setFrozen(false);    // this might not be necessary
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("freezeOnCull")) {
        if (fr[1].matchWord("TRUE")) {
            myobj.setFreezeOnCull(true);
            fr += 2;
            itAdvanced = true;
        } else if (fr[1].matchWord("FALSE")) {
            myobj.setFreezeOnCull(false);
            fr += 2;
            itAdvanced = true;
        }
    }
    
    if (fr[0].matchWord("defaultBoundingBox")) {
        osg::BoundingBox bbox;
        if (    fr[1].getFloat(bbox.xMin()) &&
            fr[2].getFloat(bbox.yMin()) &&
            fr[3].getFloat(bbox.zMin()) &&
            fr[4].getFloat(bbox.xMax()) &&
            fr[5].getFloat(bbox.yMax()) &&
            fr[6].getFloat(bbox.zMax()) ) {
            myobj.setDefaultBoundingBox(bbox);
            fr += 7;
            itAdvanced = true;
        }
    }
    
    if (fr[0].matchWord("particleTemplate")) {        
        ++fr;
        itAdvanced = true;
        osgParticle::Particle P;
        if (read_particle(fr, P)) {
            myobj.setDefaultParticleTemplate(P);
        }
    }    

    return itAdvanced;
}

bool ParticleSystem_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::ParticleSystem &myobj = static_cast<const osgParticle::ParticleSystem &>(obj);

    fw.indent() << "particleAlignment ";
    switch (myobj.getParticleAlignment()) {
        default:
        case osgParticle::ParticleSystem::BILLBOARD:
            fw << "BILLBOARD" << std::endl;
            break;
        case osgParticle::ParticleSystem::FIXED:
            fw << "FIXED" << std::endl;
            break;
    }

    fw.indent() << "particleScaleReferenceFrame ";
    switch (myobj.getParticleScaleReferenceFrame()) {
        default:
        case osgParticle::ParticleSystem::LOCAL_COORDINATES:
            fw << "LOCAL_COORDINATES" << std::endl;
            break;
        case osgParticle::ParticleSystem::WORLD_COORDINATES:
            fw << "WORLD_COORDINATES" << std::endl;
            break;
    }

    osg::Vec3 v = myobj.getAlignVectorX();
    fw.indent() << "alignVectorX " << v.x() << " " << v.y() << " " << v.z() << std::endl;
    v = myobj.getAlignVectorY();
    fw.indent() << "alignVectorY " << v.x() << " " << v.y() << " " << v.z() << std::endl;

    fw.indent() << "doublePassRendering ";
    if (myobj.getDoublePassRendering())
        fw << "TRUE" << std::endl;
    else
        fw << "FALSE" << std::endl;
        
    fw.indent() << "frozen ";
    if (myobj.isFrozen()) 
        fw << "TRUE" << std::endl;
    else
        fw << "FALSE" << std::endl;

    fw.indent() << "freezeOnCull ";
    if (myobj.getFreezeOnCull())
        fw << "TRUE" << std::endl;
    else
        fw << "FALSE" << std::endl;

    osg::BoundingBox bbox = myobj.getDefaultBoundingBox();    
    fw.indent() << "defaultBoundingBox ";
    fw << bbox.xMin() << " " << bbox.yMin() << " " << bbox.zMin() << " ";
    fw << bbox.xMax() << " " << bbox.yMax() << " " << bbox.zMax() << std::endl;

    fw.indent() << "particleTemplate ";
    write_particle(myobj.getDefaultParticleTemplate(), fw);

    return true;
}
