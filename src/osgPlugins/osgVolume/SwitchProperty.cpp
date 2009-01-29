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

bool SwitchProperty_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool SwitchProperty_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy SwitchProperty_Proxy
(
    new osgVolume::SwitchProperty,
    "SwitchProperty",
    "Object SwitchProperty CompositeProperty",
    SwitchProperty_readLocalData,
    SwitchProperty_writeLocalData
);


bool SwitchProperty_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::SwitchProperty& sp = static_cast<osgVolume::SwitchProperty&>(obj);

    bool itrAdvanced = false;

    int value=0; 
    if (fr.read("activeProperty",value))
    {
        itrAdvanced = true;
        sp.setActiveProperty(value);
    }

    return itrAdvanced;
}

bool SwitchProperty_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::SwitchProperty& sp = static_cast<const osgVolume::SwitchProperty&>(obj);

    fw.indent()<<"activeProperty "<<sp.getActiveProperty()<<std::endl;

    return true;
}
