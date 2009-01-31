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

bool TransferFunctionProperty_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool TransferFunctionProperty_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy TransferFunctionProperty_Proxy
(
    new osgVolume::TransferFunctionProperty,
    "TransferFunctionProperty",
    "Object TransferFunctionProperty",
    TransferFunctionProperty_readLocalData,
    TransferFunctionProperty_writeLocalData
);


bool TransferFunctionProperty_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::TransferFunctionProperty& tfp = static_cast<osgVolume::TransferFunctionProperty&>(obj);

    bool itrAdvanced = false;

    return itrAdvanced;
}

bool TransferFunctionProperty_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::TransferFunctionProperty& tfp = static_cast<const osgVolume::TransferFunctionProperty&>(obj);

    return true;
}
