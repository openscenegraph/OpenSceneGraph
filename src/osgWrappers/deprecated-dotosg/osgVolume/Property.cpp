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

bool Property_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Property_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(Property_Proxy)
(
    new osgVolume::Property,
    "Property",
    "Object Property",
    Property_readLocalData,
    Property_writeLocalData
);

REGISTER_DOTOSGWRAPPER(MaximumImageProjectionProperty_Proxy)
(
    new osgVolume::MaximumIntensityProjectionProperty,
    "MaximumIntensityProjectionProperty",
    "Object MaximumIntensityProjectionProperty",
    Property_readLocalData,
    Property_writeLocalData
);

REGISTER_DOTOSGWRAPPER(LightingProperty_Proxy)
(
    new osgVolume::LightingProperty,
    "LightingProperty",
    "Object LightingProperty",
    Property_readLocalData,
    Property_writeLocalData
);

bool Property_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    return false;
}

bool Property_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    return true;
}
