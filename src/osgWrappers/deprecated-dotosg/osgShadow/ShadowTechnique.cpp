#include <osgShadow/ShadowTechnique>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool ShadowTechnique_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ShadowTechnique_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ShadowTechnique_Proxy)
(
    0,
    "ShadowTechnique",
    "Object ShadowTechnique ",
    ShadowTechnique_readLocalData,
    ShadowTechnique_writeLocalData
);

bool ShadowTechnique_readLocalData(osg::Object& /*obj*/, osgDB::Input& /*fr*/)
{
    //osgShadow::ShadowTechnique& ss = static_cast<osgShadow::ShadowTechnique&>(obj);
    bool itAdvanced = false;

    return itAdvanced;
}

bool ShadowTechnique_writeLocalData(const osg::Object& /*obj*/, osgDB::Output& /*fw*/)
{
    //const osgShadow::ShadowTechnique& ss = static_cast<const osgShadow::ShadowTechnique &>(obj);

    return true;
}
