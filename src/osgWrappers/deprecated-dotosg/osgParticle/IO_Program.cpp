
#include <osgParticle/Program>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  IOProgram_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  IOProgram_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(g_IOProgramProxy)
(
    0,
    "osgParticle::Program",
    "Object Node ParticleProcessor osgParticle::Program",
    IOProgram_readLocalData,
    IOProgram_writeLocalData
);

bool IOProgram_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool IOProgram_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
