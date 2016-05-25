/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

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

bool CompositeLayer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool CompositeLayer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(CompositeLayer_Proxy)
(
    new osgTerrain::CompositeLayer,
    "CompositeLayer",
    "Object CompositeLayer Layer",
    CompositeLayer_readLocalData,
    CompositeLayer_writeLocalData
);

bool CompositeLayer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::CompositeLayer& layer = static_cast<osgTerrain::CompositeLayer&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osgTerrain::Locator> locator = 0;

    do
    {
        itrAdvanced = false;

        osg::ref_ptr<osg::Object> readLocatorObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
        locator = dynamic_cast<osgTerrain::Locator*>(readLocatorObject.get());
        if (readLocatorObject.valid()) itrAdvanced = true;

        unsigned int minLevel=0;
        if (fr.read("MinLevel",minLevel))
        {
            itrAdvanced = true;
        }

        unsigned int maxLevel = MAXIMUM_NUMBER_OF_LEVELS;
        if (fr.read("MaxLevel",maxLevel))
        {
            itrAdvanced = true;
        }

        if (fr.matchSequence("file %s") || fr.matchSequence("file %w") )
        {
            layer.addLayer(fr[1].getStr());
            fr += 2;

            itrAdvanced = true;
        }
        else if (fr.matchSequence("ProxyLayer %s") || fr.matchSequence("ProxyLayer %w"))
        {
            std::string setname;
            std::string filename;
            osgTerrain::extractSetNameAndFileName(fr[1].getStr(),setname, filename);
            if (!filename.empty())
            {
                osgTerrain::ProxyLayer* proxyLayer = new osgTerrain::ProxyLayer;
                proxyLayer->setFileName(filename);
                proxyLayer->setName(setname);

                if (locator.valid()) proxyLayer->setLocator(locator.get());
                if (minLevel!=0) proxyLayer->setMinLevel(minLevel);
                if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) proxyLayer->setMaxLevel(maxLevel);


                layer.addLayer(proxyLayer);
            }

            fr += 2;

            itrAdvanced = true;
        }
        else
        {
            osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Layer>());
            osgTerrain::Layer* readLayer = dynamic_cast<osgTerrain::Layer*>(readObject.get());
            if (readLayer)
            {
                if (locator.valid())
                {
                    readLayer->setLocator(locator.get());
                    locator = 0;
                }

                if (minLevel!=0) readLayer->setMinLevel(minLevel);
                if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) readLayer->setMaxLevel(maxLevel);

                layer.addLayer(readLayer);
            }

            if (readObject.valid()) itrAdvanced = true;
        }

    } while (itrAdvanced);

    if (locator.valid()) layer.setLocator(locator.get());

    return itrAdvanced;
}

bool CompositeLayer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::CompositeLayer& layer = static_cast<const osgTerrain::CompositeLayer&>(obj);

    for(unsigned int i=0; i<layer.getNumLayers();++i)
    {
        if (layer.getLayer(i))
        {
            const osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<const osgTerrain::ProxyLayer*>(layer.getLayer(i));
            if (proxyLayer)
            {
                if (!proxyLayer->getFileName().empty())
                {
                    const osgTerrain::Locator* locator = proxyLayer->getLocator();
                    if (locator && !locator->getDefinedInFile())
                    {
                        fw.writeObject(*locator);
                    }

                    if (proxyLayer->getMinLevel()!=0)
                    {
                        fw.indent()<<"MinLevel "<<proxyLayer->getMinLevel()<<std::endl;
                    }

                    if (proxyLayer->getMaxLevel()!=MAXIMUM_NUMBER_OF_LEVELS)
                    {
                        fw.indent()<<"MaxLevel "<<proxyLayer->getMaxLevel()<<std::endl;
                    }

                    fw.indent()<<"ProxyLayer "<<proxyLayer->getCompoundName()<<std::endl;
                }
            }
            else
            {
                fw.writeObject(*(layer.getLayer(i)));
            }
        }
        else if (!layer.getFileName(i).empty())
        {
            fw.indent()<<"file "<<layer.getCompoundName(i)<<std::endl;
        }
    }



    return true;
}
