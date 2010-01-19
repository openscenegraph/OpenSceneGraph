#include <osgVolume/Property>

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

bool ScalarProperty_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ScalarProperty_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy ScalarProperty_Proxy
(
    0,
    "ScalarProperty",
    "Object ScalarProperty",
    ScalarProperty_readLocalData,
    ScalarProperty_writeLocalData
);

osgDB::RegisterDotOsgWrapperProxy IsoSurfaceProperty_Proxy
(
    new osgVolume::IsoSurfaceProperty,
    "IsoSurfaceProperty",
    "Object ScalarProperty",
    ScalarProperty_readLocalData,
    ScalarProperty_writeLocalData
);

osgDB::RegisterDotOsgWrapperProxy AlphaFuncProperty_Proxy
(
    new osgVolume::AlphaFuncProperty,
    "AlphaFuncProperty",
    "Object AlphaFuncProperty",
    ScalarProperty_readLocalData,
    ScalarProperty_writeLocalData
);

osgDB::RegisterDotOsgWrapperProxy SampleDensityProperty_Proxy
(
    new osgVolume::SampleDensityProperty,
    "SampleDensityProperty",
    "Object SampleDensityProperty",
    ScalarProperty_readLocalData,
    ScalarProperty_writeLocalData
);

osgDB::RegisterDotOsgWrapperProxy TransparencyProperty_Proxy
(
    new osgVolume::TransparencyProperty,
    "TransparencyProperty",
    "Object TransparencyProperty",
    ScalarProperty_readLocalData,
    ScalarProperty_writeLocalData
);



bool ScalarProperty_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::ScalarProperty& sp = static_cast<osgVolume::ScalarProperty&>(obj);

    bool itrAdvanced = false;

    float value=0; 
    if (fr.read("value",value))
    {
        itrAdvanced = true;
        sp.setValue(value);
    }

    return itrAdvanced;
}

bool ScalarProperty_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::ScalarProperty& sp = static_cast<const osgVolume::ScalarProperty&>(obj);

    fw.indent()<<"value "<<sp.getValue()<<std::endl;

    return true;
}
