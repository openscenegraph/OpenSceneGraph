#include <osgVolume/VolumeTile>

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

bool PropertyAdjustmentCallback_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool PropertyAdjustmentCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy PropertyAdjustmentCallback_Proxy
(
    new osgVolume::PropertyAdjustmentCallback,
    "PropertyAdjustmentCallback",
    "Object NodeCallback PropertyAdjustmentCallback",
    PropertyAdjustmentCallback_readLocalData,
    PropertyAdjustmentCallback_writeLocalData
);


bool PropertyAdjustmentCallback_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    return false;
}

bool PropertyAdjustmentCallback_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    return true;
}
