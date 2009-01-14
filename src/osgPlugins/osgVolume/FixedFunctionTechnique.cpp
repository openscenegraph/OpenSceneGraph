#include <osgVolume/FixedFunctionTechnique>

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

bool FixedFunctionTechnique_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool FixedFunctionTechnique_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy FixedFunctionTechnique_Proxy
(
    new osgVolume::FixedFunctionTechnique,
    "FixedFunctionTechnique",
    "FixedFunctionTechnique Object",
    FixedFunctionTechnique_readLocalData,
    FixedFunctionTechnique_writeLocalData
);


bool FixedFunctionTechnique_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    //osgVolume::FixedFunctionTechnique& fft = static_cast<osgVolume::FixedFunctionTechnique&>(obj);
    bool itrAdvanced = false;
    return itrAdvanced;
}

bool FixedFunctionTechnique_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    //const osgVolume::FixedFunctionTechnique& fft = static_cast<const osgVolume::FixedFunctionTechnique&>(obj);
    return true;
}
