
#include <osgParticle/MultiSegmentPlacer>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  MultiSegmentPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  MultiSegmentPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(MultiSegmentPlacer_Proxy)
(
    new osgParticle::MultiSegmentPlacer,
    "MultiSegmentPlacer",
    "Object Placer MultiSegmentPlacer",
    MultiSegmentPlacer_readLocalData,
    MultiSegmentPlacer_writeLocalData
);

bool MultiSegmentPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::MultiSegmentPlacer &myobj = static_cast<osgParticle::MultiSegmentPlacer &>(obj);
    bool itAdvanced = false;

    osg::Vec3 v;

    if (fr[0].matchWord("vertex")) {
        if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
            myobj.addVertex(v);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool MultiSegmentPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::MultiSegmentPlacer &myobj = static_cast<const osgParticle::MultiSegmentPlacer &>(obj);

    int n = myobj.numVertices();

    for (int i=0; i<n; ++i) {
        const osg::Vec3 &v = myobj.getVertex(i);
        fw.indent() << "vertex " << v.x() << " " << v.y() << " " << v.z() << std::endl;
    }

    return true;
}
