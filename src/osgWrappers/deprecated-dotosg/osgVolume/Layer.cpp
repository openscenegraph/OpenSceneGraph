#include <osgVolume/Layer>

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

#include <string.h>

bool Layer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Layer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);
bool Layer_matchFilterStr(const char* str, osg::Texture::FilterMode& filter);
const char* Layer_getFilterStr(osg::Texture::FilterMode filter);

REGISTER_DOTOSGWRAPPER(Layer_Proxy)
(
    new osgVolume::Layer,
    "Layer",
    "Object Layer",
    Layer_readLocalData,
    Layer_writeLocalData
);

bool Layer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::Layer& layer = static_cast<osgVolume::Layer&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::Locator>());
    osgVolume::Locator* locator = dynamic_cast<osgVolume::Locator*>(readObject.get());
    if (locator) layer.setLocator(locator);

    readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::Property>());
    if (readObject.valid()) itrAdvanced = true;

    osgVolume::Property* property = dynamic_cast<osgVolume::Property*>(readObject.get());
    if (property) layer.addProperty(property);

    return itrAdvanced;
}

bool Layer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::Layer& layer = static_cast<const osgVolume::Layer&>(obj);

    if (layer.getLocator())
    {
        fw.writeObject(*layer.getLocator());
    }

    if (layer.getProperty())
    {
        fw.writeObject(*layer.getProperty());
    }

    return true;
}

