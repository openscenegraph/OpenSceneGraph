
#include <osgParticle/SectorPlacer>

#include <iostream>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  SectorPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  SectorPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  SectorPlacer_Proxy
(
    osgNew osgParticle::SectorPlacer,
    "SectorPlacer",
    "Object Placer CenteredPlacer SectorPlacer",
    SectorPlacer_readLocalData,
    SectorPlacer_writeLocalData
);

bool SectorPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::SectorPlacer &myobj = static_cast<osgParticle::SectorPlacer &>(obj);
    bool itAdvanced = false;

    osgParticle::rangef r;
    if (fr[0].matchWord("radiusRange")) {
        if (fr[1].getFloat(r.min) && fr[2].getFloat(r.max)) {
            myobj.setRadiusRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("phiRange")) {
        if (fr[1].getFloat(r.min) && fr[2].getFloat(r.max)) {
            myobj.setPhiRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool SectorPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::SectorPlacer &myobj = static_cast<const osgParticle::SectorPlacer &>(obj);

    osgParticle::rangef r;
    
    r = myobj.getRadiusRange();
    fw.indent() << "radiusRange " << r.min << " " << r.max << std::endl;
    r = myobj.getPhiRange();
    fw.indent() << "phiRange " << r.min << " " << r.max << std::endl;

    return true;
}
