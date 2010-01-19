
#include <osgParticle/ExplosionDebrisEffect>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  ExplosionDebrisEffect_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  ExplosionDebrisEffect_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ExplosionDebrisEffect_Proxy)
(
    new osgParticle::ExplosionDebrisEffect(false),
    "ExplosionDebrisEffect",
    "Object Node ParticleEffect ExplosionDebrisEffect",
    ExplosionDebrisEffect_readLocalData,
    ExplosionDebrisEffect_writeLocalData
);

bool ExplosionDebrisEffect_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool ExplosionDebrisEffect_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
