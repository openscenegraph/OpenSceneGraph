#include <osgShadow/ShadowedScene>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool ShadowedScene_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ShadowedScene_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ShadowedScene_Proxy)
(
    new osgShadow::ShadowedScene,
    "ShadowedScene",
    "Object ShadowedScene Group ",
    ShadowedScene_readLocalData,
    ShadowedScene_writeLocalData
);

bool ShadowedScene_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgShadow::ShadowedScene& ss = static_cast<osgShadow::ShadowedScene&>(obj);
    bool iteratorAdvanced = false;

    osg::ref_ptr<osg::Object> object=0;
    while((object=fr.readObject())!=0)
    {
        osgShadow::ShadowTechnique* st = dynamic_cast<osgShadow::ShadowTechnique*>(object.get());
        if (st) ss.setShadowTechnique(st);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool ShadowedScene_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgShadow::ShadowedScene& ss = static_cast<const osgShadow::ShadowedScene &>(obj);

    if (ss.getShadowTechnique())
    {
        fw.writeObject(*ss.getShadowTechnique());
    }

    return true;
}
