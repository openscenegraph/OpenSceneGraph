
#include <osgParticle/ExplosionDebriEffect>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  ExplosionDebriEffect_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ExplosionDebriEffect_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  ExplosionDebriEffect_Proxy
(
    new osgParticle::ExplosionDebriEffect,
    "ExplosionDebriEffect",
    "Object Node ParticleEffect ExplosionDebriEffect",
    ExplosionDebriEffect_readLocalData,
    ExplosionDebriEffect_writeLocalData
);

bool ExplosionDebriEffect_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool ExplosionDebriEffect_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
