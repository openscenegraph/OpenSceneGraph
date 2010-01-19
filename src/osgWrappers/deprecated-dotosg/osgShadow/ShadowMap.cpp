#include <osgShadow/ShadowMap>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool ShadowMap_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ShadowMap_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ShadowMap_Proxy)
(
    new osgShadow::ShadowMap,
    "ShadowMap",
    "Object ShadowTechnique ShadowMap",
    ShadowMap_readLocalData,
    ShadowMap_writeLocalData
);

bool ShadowMap_readLocalData(osg::Object& /*obj*/, osgDB::Input &/*fr*/)
{
    // osgShadow::ShadowMap& ss = static_cast<osgShadow::ShadowMap&>(obj);
    bool itAdvanced = false;

    return itAdvanced;
}

bool ShadowMap_writeLocalData(const osg::Object& /*obj*/, osgDB::Output& /*fw*/)
{
    // const osgShadow::ShadowMap& ss = static_cast<const osgShadow::ShadowMap &>(obj);

    return true;
}
