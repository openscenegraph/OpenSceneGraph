
#include <osgParticle/CenteredPlacer>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  CenteredPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  CenteredPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  CenteredPlacer_Proxy
(
    0,
    "CenteredPlacer",
    "Object Placer CenteredPlacer",
    CenteredPlacer_readLocalData,
    CenteredPlacer_writeLocalData
);

bool CenteredPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::CenteredPlacer &myobj = static_cast<osgParticle::CenteredPlacer &>(obj);
    bool itAdvanced = false;

    osg::Vec3 v;
    if (fr[0].matchWord("center")) {
        if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
            myobj.setCenter(v);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool CenteredPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::CenteredPlacer &myobj = static_cast<const osgParticle::CenteredPlacer &>(obj);

    osg::Vec3 v = myobj.getCenter();
    fw.indent() << "center " << v.x() << " " << v.y() << " " << v.z() << std::endl;

    return true;
}
