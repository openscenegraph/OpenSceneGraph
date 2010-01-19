#include <osgTerrain/GeometryTechnique>

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

bool GeometryTechnique_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool GeometryTechnique_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy GeometryTechnique_Proxy
(
    new osgTerrain::GeometryTechnique,
    "GeometryTechnique",
    "GeometryTechnique Object",
    GeometryTechnique_readLocalData,
    GeometryTechnique_writeLocalData
);


bool GeometryTechnique_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    //osgTerrain::GeometryTechnique& gt = static_cast<osgTerrain::GeometryTechnique&>(obj);
    bool itrAdvanced = false;
    return itrAdvanced;
}

bool GeometryTechnique_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    //const osgTerrain::GeometryTechnique& gt = static_cast<const osgTerrain::GeometryTechnique&>(obj);
    return true;
}
