
#include <osgParticle/SmokeEffect>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  SmokeEffect_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  SmokeEffect_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  SmokeEffect_Proxy
(
    new osgParticle::SmokeEffect(false),
    "SmokeEffect",
    "Object Node ParticleEffect SmokeEffect",
    SmokeEffect_readLocalData,
    SmokeEffect_writeLocalData
);

bool SmokeEffect_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool SmokeEffect_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
