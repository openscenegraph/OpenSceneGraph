
#include <osgParticle/ConnectedParticleSystem>

#include <osg/BoundingBox>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <iostream>
#include <string>

extern bool  read_particle(osgDB::Input &fr, osgParticle::Particle &P);
extern void  write_particle(const osgParticle::Particle &P, osgDB::Output &fw);

bool  ConnectedParticleSystem_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ConnectedParticleSystem_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  ConnectedParticleSystem_Proxy
(
    new osgParticle::ConnectedParticleSystem,
    "ConnectedParticleSystem",
    "Object Drawable ParticleSystem ConnectedParticleSystem",
    ConnectedParticleSystem_readLocalData,
    ConnectedParticleSystem_writeLocalData
);

bool ConnectedParticleSystem_readLocalData(osg::Object&, osgDB::Input&)
{
    return false;
}

bool ConnectedParticleSystem_writeLocalData(const osg::Object&, osgDB::Output&)
{
    return false;
}
