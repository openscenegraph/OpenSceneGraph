
#include <osgParticle/SegmentPlacer>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  SegmentPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  SegmentPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  SegmentPlacer_Proxy
(
    new osgParticle::SegmentPlacer,
    "SegmentPlacer",
    "Object Placer SegmentPlacer",
    SegmentPlacer_readLocalData,
    SegmentPlacer_writeLocalData
);

bool SegmentPlacer_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::SegmentPlacer &myobj = static_cast<osgParticle::SegmentPlacer &>(obj);
    bool itAdvanced = false;

    osg::Vec3 v;

    if (fr[0].matchWord("vertex_A")) {
        if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
            myobj.setVertexA(v);
            fr += 4;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("vertex_B")) {
        if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
            myobj.setVertexB(v);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool SegmentPlacer_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::SegmentPlacer &myobj = static_cast<const osgParticle::SegmentPlacer &>(obj);

    osg::Vec3 v = myobj.getVertexA();
    fw.indent() << "vertex_A " << v.x() << " " << v.y() << " " << v.z() << std::endl;
    v = myobj.getVertexB();
    fw.indent() << "vertex_B " << v.x() << " " << v.y() << " " << v.z() << std::endl;

    return true;
}
