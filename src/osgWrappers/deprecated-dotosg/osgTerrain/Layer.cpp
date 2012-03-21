#include <osgTerrain/Layer>

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
    new osgTerrain::Layer,
    "Layer",
    "Object Layer",
    Layer_readLocalData,
    Layer_writeLocalData
);

bool Layer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::Layer& layer = static_cast<osgTerrain::Layer&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
    osgTerrain::Locator* locator = dynamic_cast<osgTerrain::Locator*>(readObject.get());
    if (locator) layer.setLocator(locator);

    osg::Texture::FilterMode filter;
    if (fr[0].matchWord("MinFilter") && Layer_matchFilterStr(fr[1].getStr(),filter))
    {
        layer.setMinFilter(filter);
        fr+=2;
        itrAdvanced = true;
    }

    if ((fr[0].matchWord("Filter") || fr[0].matchWord("MagFilter")) &&
        Layer_matchFilterStr(fr[1].getStr(),filter))
    {
        layer.setMagFilter(filter);
        fr+=2;
        itrAdvanced = true;
    }


    unsigned int minLevel=0;
    if (fr.read("MinLevel",minLevel))
    {
        itrAdvanced = true;
        layer.setMinLevel(minLevel);
    }

    unsigned int maxLevel = MAXIMUM_NUMBER_OF_LEVELS;
    if (fr.read("MaxLevel",maxLevel))
    {
        itrAdvanced = true;
        layer.setMaxLevel(maxLevel);
    }

    return itrAdvanced;
}

bool Layer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::Layer& layer = static_cast<const osgTerrain::Layer&>(obj);

    if (layer.getLocator() && !(layer.getLocator()->getDefinedInFile()))
    {
        fw.writeObject(*layer.getLocator());
    }

    fw.indent()<<"MinFilter "<<Layer_getFilterStr(layer.getMinFilter())<<std::endl;
    fw.indent()<<"MagFilter "<<Layer_getFilterStr(layer.getMagFilter())<<std::endl;

    if (layer.getMinLevel()!=0)
    {
        fw.indent()<<"MinLevel "<<layer.getMinLevel()<<std::endl;
    }

    if (layer.getMaxLevel()!=MAXIMUM_NUMBER_OF_LEVELS)
    {
        fw.indent()<<"MaxLevel "<<layer.getMaxLevel()<<std::endl;
    }

    return true;
}

bool Layer_matchFilterStr(const char* str, osg::Texture::FilterMode& filter)
{
    if (strcmp(str,"NEAREST")==0) filter = osg::Texture::NEAREST;
    else if (strcmp(str,"LINEAR")==0) filter = osg::Texture::LINEAR;
    else if (strcmp(str,"NEAREST_MIPMAP_NEAREST")==0) filter = osg::Texture::NEAREST_MIPMAP_NEAREST;
    else if (strcmp(str,"LINEAR_MIPMAP_NEAREST")==0) filter = osg::Texture::LINEAR_MIPMAP_NEAREST;
    else if (strcmp(str,"NEAREST_MIPMAP_LINEAR")==0) filter = osg::Texture::NEAREST_MIPMAP_LINEAR;
    else if (strcmp(str,"LINEAR_MIPMAP_LINEAR")==0) filter = osg::Texture::LINEAR_MIPMAP_LINEAR;
    else if (strcmp(str,"ANISOTROPIC")==0) filter = osg::Texture::LINEAR;
    else return false;
    return true;
}


const char* Layer_getFilterStr(osg::Texture::FilterMode filter)
{
    switch(filter)
    {
        case(osg::Texture::NEAREST): return "NEAREST";
        case(osg::Texture::LINEAR): return "LINEAR";
        case(osg::Texture::NEAREST_MIPMAP_NEAREST): return "NEAREST_MIPMAP_NEAREST";
        case(osg::Texture::LINEAR_MIPMAP_NEAREST): return "LINEAR_MIPMAP_NEAREST";
        case(osg::Texture::NEAREST_MIPMAP_LINEAR): return "NEAREST_MIPMAP_LINEAR";
        case(osg::Texture::LINEAR_MIPMAP_LINEAR): return "LINEAR_MIPMAP_LINEAR";
    }
    return "";
}

