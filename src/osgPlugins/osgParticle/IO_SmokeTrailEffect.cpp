
#include <osgParticle/SmokeTrailEffect>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  SmokeTrailEffect_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  SmokeTrailEffect_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  SmokeTrailEffect_Proxy
(
    new osgParticle::SmokeTrailEffect(false),
    "SmokeTrailEffect",
    "Object Node ParticleEffect SmokeTrailEffect",
    SmokeTrailEffect_readLocalData,
    SmokeTrailEffect_writeLocalData
);

bool SmokeTrailEffect_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool SmokeTrailEffect_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
