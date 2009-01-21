#include <osgVolume/RayTracedTechnique>

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

bool RayTracedTechnique_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool RayTracedTechnique_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy RayTracedTechnique_Proxy
(
    new osgVolume::RayTracedTechnique,
    "RayTracedTechnique",
    "RayTracedTechnique Object",
    RayTracedTechnique_readLocalData,
    RayTracedTechnique_writeLocalData
);


bool RayTracedTechnique_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    //osgVolume::RayTracedTechnique& st = static_cast<osgVolume::RayTracedTechnique&>(obj);
    bool itrAdvanced = false;
    return itrAdvanced;
}

bool RayTracedTechnique_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    //const osgVolume::RayTracedTechnique& st = static_cast<const osgVolume::RayTracedTechnique&>(obj);
    return true;
}
