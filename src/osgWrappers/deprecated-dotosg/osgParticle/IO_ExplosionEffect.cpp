
#include <osgParticle/ExplosionEffect>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  ExplosionEffect_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ExplosionEffect_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ExplosionEffect_Proxy)
(
    new osgParticle::ExplosionEffect(false),
    "ExplosionEffect",
    "Object Node ParticleEffect ExplosionEffect",
    ExplosionEffect_readLocalData,
    ExplosionEffect_writeLocalData
);

bool ExplosionEffect_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool ExplosionEffect_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
