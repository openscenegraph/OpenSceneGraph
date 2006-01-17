
#include <osgParticle/BoxPlacer>

#include <iostream>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  BoxPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  BoxPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  BoxPlacer_Proxy
(
    new osgParticle::BoxPlacer,
    "BoxPlacer",
    "Object Placer CenteredPlacer BoxPlacer",
    BoxPlacer_readLocalData,
    BoxPlacer_writeLocalData
);

bool BoxPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::BoxPlacer &myobj = static_cast<osgParticle::BoxPlacer &>(obj);
    bool itAdvanced = false;

    osgParticle::rangef r;
    if (fr[0].matchWord("xRange")) {
        if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
            myobj.setXRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("yRange")) {
        if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
            myobj.setYRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("zRange")) {
        if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
            myobj.setZRange(r);
            fr += 3;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool BoxPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::BoxPlacer &myobj = static_cast<const osgParticle::BoxPlacer &>(obj);

    osgParticle::rangef r;
    
    r = myobj.getXRange();
    fw.indent() << "xRange " << r.minimum << " " << r.maximum << std::endl;
    r = myobj.getYRange();
    fw.indent() << "yRange " << r.minimum << " " << r.maximum << std::endl;
    r = myobj.getZRange();
    fw.indent() << "zRange " << r.minimum << " " << r.maximum << std::endl;

    return true;
}
