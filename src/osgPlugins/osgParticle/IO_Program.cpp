
#include <osgParticle/Program>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  Program_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  Program_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  Program_Proxy
(
    0,
    "Program",
    "Object Node ParticleProcessor Program",
    Program_readLocalData,
    Program_writeLocalData
);

bool Program_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool Program_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
