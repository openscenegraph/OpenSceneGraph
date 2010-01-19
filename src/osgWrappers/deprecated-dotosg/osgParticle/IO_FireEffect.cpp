
#include <osgParticle/FireEffect>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  FireEffect_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  FireEffect_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  FireEffect_Proxy
(
    new osgParticle::FireEffect(false),
    "FireEffect",
    "Object Node ParticleEffect FireEffect",
    FireEffect_readLocalData,
    FireEffect_writeLocalData
);

bool FireEffect_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool FireEffect_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
