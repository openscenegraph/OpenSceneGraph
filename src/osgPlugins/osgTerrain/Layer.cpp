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

bool Layer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Layer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Layer_Proxy
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
    
    if (fr[0].matchWord("Filter"))
    {
        unsigned int layerNum = 0;
        if (fr.matchSequence("Filter %i"))
        {
            fr[1].getUInt(layerNum);
            fr += 2;
        }
        else
        {
            ++fr;
        }

        if (fr[0].matchWord("NEAREST")) layer.setFilter(osgTerrain::Layer::NEAREST);
        else if (fr[0].matchWord("LINEAR")) layer.setFilter(osgTerrain::Layer::LINEAR);

        ++fr;
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
    
    if (layer.getFilter()!=osgTerrain::Layer::LINEAR)
    {
        if (layer.getFilter()==osgTerrain::Layer::LINEAR)
        {
            fw.indent()<<"Filter LINEAER"<<std::endl;
        }
        else
        {
            fw.indent()<<"Filter NEAREST"<<std::endl;
        }
    }

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
