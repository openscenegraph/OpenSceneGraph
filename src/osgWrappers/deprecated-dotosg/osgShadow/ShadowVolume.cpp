#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool ShadowVolume_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ShadowVolume_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ShadowVolume_Proxy)
(
    0,
    "ShadowVolume",
    "Object ShadowTechnique ShadowVolume",
    ShadowVolume_readLocalData,
    ShadowVolume_writeLocalData
);

bool ShadowVolume_readLocalData(osg::Object& /*obj*/, osgDB::Input& /*fr*/)
{
    // osgShadow::ShadowVolume& ss = static_cast<osgShadow::ShadowVolume&>(obj);
    bool itAdvanced = false;

    return itAdvanced;
}

bool ShadowVolume_writeLocalData(const osg::Object& /*obj*/, osgDB::Output& /*fw*/)
{
    // const osgShadow::ShadowVolume& ss = static_cast<const osgShadow::ShadowVolume &>(obj);

    return true;
}
