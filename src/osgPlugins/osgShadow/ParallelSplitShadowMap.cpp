#include <osgShadow/ParallelSplitShadowMap>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool ParallelSplitShadowMap_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ParallelSplitShadowMap_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy ParallelSplitShadowMap_Proxy
(
    new osgShadow::ParallelSplitShadowMap,
    "ParallelSplitShadowMap",
    "Object ShadowTechnique ParallelSplitShadowMap",
    ParallelSplitShadowMap_readLocalData,
    ParallelSplitShadowMap_writeLocalData
);

bool ParallelSplitShadowMap_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgShadow::ParallelSplitShadowMap& ss = static_cast<osgShadow::ParallelSplitShadowMap&>(obj);
    bool itAdvanced = false;

    return itAdvanced;
}

bool ParallelSplitShadowMap_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgShadow::ParallelSplitShadowMap& ss = static_cast<const osgShadow::ParallelSplitShadowMap &>(obj);

    return true;
}
