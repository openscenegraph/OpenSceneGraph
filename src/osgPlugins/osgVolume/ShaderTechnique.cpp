#include <osgVolume/ShaderTechnique>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool ShaderTechnique_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ShaderTechnique_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy ShaderTechnique_Proxy
(
    new osgVolume::ShaderTechnique,
    "ShaderTechnique",
    "ShaderTechnique Object",
    ShaderTechnique_readLocalData,
    ShaderTechnique_writeLocalData
);


bool ShaderTechnique_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    //osgVolume::ShaderTechnique& st = static_cast<osgVolume::ShaderTechnique&>(obj);
    bool itrAdvanced = false;
    return itrAdvanced;
}

bool ShaderTechnique_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    //const osgVolume::ShaderTechnique& st = static_cast<const osgVolume::ShaderTechnique&>(obj);
    return true;
}
