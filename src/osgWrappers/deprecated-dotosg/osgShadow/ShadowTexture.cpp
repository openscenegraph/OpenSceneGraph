#include <osgShadow/ShadowTexture>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool ShadowTexture_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ShadowTexture_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ShadowTexture_Proxy)
(
    new osgShadow::ShadowTexture,
    "ShadowTexture",
    "Object ShadowTechnique ShadowTexture",
    ShadowTexture_readLocalData,
    ShadowTexture_writeLocalData
);

bool ShadowTexture_readLocalData(osg::Object& /*obj*/, osgDB::Input& /*fr*/)
{
    // osgShadow::ShadowTexture& ss = static_cast<osgShadow::ShadowTexture&>(obj);
    bool itAdvanced = false;

    return itAdvanced;
}

bool ShadowTexture_writeLocalData(const osg::Object& /*obj*/, osgDB::Output& /*fw*/)
{
    // const osgShadow::ShadowTexture& ss = static_cast<const osgShadow::ShadowTexture &>(obj);

    return true;
}
