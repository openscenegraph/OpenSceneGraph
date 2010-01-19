
#include <osgParticle/PointPlacer>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  PointPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  PointPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(PointPlacer_Proxy)
(
    new osgParticle::PointPlacer,
    "PointPlacer",
    "Object Placer CenteredPlacer PointPlacer",
    PointPlacer_readLocalData,
    PointPlacer_writeLocalData
);

bool PointPlacer_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool PointPlacer_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
