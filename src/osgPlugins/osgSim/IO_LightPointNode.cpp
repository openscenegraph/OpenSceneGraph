#include <osgSim/LightPointNode>
#include <osgSim/LightPoint>
#include <osg/ref_ptr>

#include "IO_LightPoint.h"

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

using namespace osgSim;

bool LightPointNode_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool LightPointNode_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy LightPointNode_Proxy
(
    new LightPointNode,
    "LightPointNode",
    "Object Node LightPointNode",
    &LightPointNode_readLocalData,
    &LightPointNode_writeLocalData
);

bool LightPointNode_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    LightPointNode &lightpointnode = static_cast<LightPointNode &>(obj);
    bool itAdvanced = false;

    if (fr.matchSequence("num_lightpoints %d")) {
        // Could allocate space for lightpoints here
        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("minPixelSize %f")) {
        float size = 0.0f;
        fr[1].getFloat(size);
        lightpointnode.setMinPixelSize(size);

        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("maxPixelSize %f")) {
        float size = 30.0f;
        fr[1].getFloat(size);
        lightpointnode.setMaxPixelSize(size);

        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("maxVisibleDistance2 %f")) {
        float distance = FLT_MAX;
        fr[1].getFloat(distance);
        lightpointnode.setMaxVisibleDistance2(distance);

        fr += 2;
        itAdvanced = true;
    }

    if (fr[0].matchWord("lightPoint")) {
        LightPoint lp;
        if (readLightPoint(lp, fr)) {
            lightpointnode.addLightPoint(lp);
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool LightPointNode_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const LightPointNode &lightpointnode = static_cast<const LightPointNode &>(obj);

    fw.indent() << "num_lightpoints " << lightpointnode.getNumLightPoints() << std::endl;

    fw.indent() << "minPixelSize " << lightpointnode.getMinPixelSize() << std::endl;

    fw.indent() << "maxPixelSize " << lightpointnode.getMaxPixelSize() << std::endl;

    fw.indent() << "maxVisibleDistance2 " << lightpointnode.getMaxVisibleDistance2() << std::endl;

    LightPointNode::LightPointList const lightpointlist = lightpointnode.getLightPointList();
    LightPointNode::LightPointList::const_iterator itr;

    for (itr = lightpointlist.begin(); itr != lightpointlist.end(); itr++) {
        writeLightPoint((*itr), fw);
    }

    return true;
}
