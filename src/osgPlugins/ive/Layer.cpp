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

#include "Exception.h"
#include "Layer.h"
#include "Locator.h"
#include "Object.h"
#include "Locator.h"
#include "ImageLayer.h"
#include "HeightFieldLayer.h"
#include "CompositeLayer.h"
#include "SwitchLayer.h"

#include <osgDB/ReadFile>

using namespace ive;

void Layer::write(DataOutputStream* out)
{
    // Write Layer's identification.
    out->writeInt(IVELAYER);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object*  object = dynamic_cast<osg::Object*>(this);
    if (object)
        ((ive::Object*)(object))->write(out);
    else
        throw Exception("Layer::write(): Could not cast this osgLayer::Layer to an osg::Object.");

 
    if (out->getVersion() >= VERSION_0023)
    {
        out->writeLocator(getLocator());

        if (out->getVersion() >= VERSION_0034)
        {
            out->writeUInt(getMinFilter());
            out->writeUInt(getMagFilter());
        }
        else
        {
            out->writeUInt((getMagFilter()==osg::Texture::LINEAR) ? 1 : 0);
        }
    }
    else
    {
        LayerHelper helper;
        helper.writeLocator(out, getLocator());
    }
    
    

    out->writeUInt(getMinLevel());
    out->writeUInt(getMaxLevel());

    if (out->getVersion() >= VERSION_0027)
    {
        writeValidDataOperator(out,getValidDataOperator());
    }
}

void Layer::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVELAYER)
        throw Exception("Layer::read(): Expected Layer identification.");
    
    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osg::Object*  object = dynamic_cast<osg::Object*>(this);
    if(object)
        ((ive::Object*)(object))->read(in);
    else
        throw Exception("Layer::read(): Could not cast this osgLayer::Layer to an osg::Group.");

    if (in->getVersion() >= VERSION_0023)
    {
        setLocator(in->readLocator());

        if (in->getVersion() >= VERSION_0034)
        {
            setMinFilter(osg::Texture::FilterMode(in->readUInt()));
            setMagFilter(osg::Texture::FilterMode(in->readUInt()));
        }
        else
        {
            setMagFilter(in->readUInt()==0 ? osg::Texture::NEAREST : osg::Texture::LINEAR);
        }
    }
    else
    {
        LayerHelper helper;
        setLocator(helper.readLocator(in));
    }

    setMinLevel(in->readUInt());
    setMaxLevel(in->readUInt());

    if (in->getVersion() >= VERSION_0027)
    {
        setValidDataOperator(readValidDataOperator(in));
    }
}

void LayerHelper::writeLayer(DataOutputStream* out, osgTerrain::Layer* layer)
{
    if (layer) 
    {
        out->writeBool(true);

        if (dynamic_cast<osgTerrain::HeightFieldLayer*>(layer))
        {
            ((ive::HeightFieldLayer*)(layer))->write(out);
        }
        else if (dynamic_cast<osgTerrain::ImageLayer*>(layer))
        {
            ((ive::ImageLayer*)(layer))->write(out);
        }
        else if (dynamic_cast<osgTerrain::SwitchLayer*>(layer))
        {
            ((ive::SwitchLayer*)(layer))->write(out);
        }
        else if (dynamic_cast<osgTerrain::CompositeLayer*>(layer))
        {
            ((ive::CompositeLayer*)(layer))->write(out);
        }
        else if (dynamic_cast<osgTerrain::ProxyLayer*>(layer))
        {
            out->writeInt(IVEPROXYLAYER);
            out->writeString(layer->getFileName());

            osgTerrain::Locator* locator = layer->getLocator();
            bool writeOutLocator = locator && !locator->getDefinedInFile();
            writeLocator(out, writeOutLocator ? locator : 0 );

            out->writeUInt(layer->getMinLevel());
            out->writeUInt(layer->getMaxLevel());
        }

    }
    else
    {
        out->writeBool(false);
    }
}

osgTerrain::Layer* LayerHelper::readLayer(DataInputStream* in)
{
    bool layerExist = in->readBool();
    if (!layerExist) return 0;

    int id = in->peekInt();
    if (id==IVEHEIGHTFIELDLAYER)
    {
        osgTerrain::HeightFieldLayer* layer = new osgTerrain::HeightFieldLayer;
        ((ive::HeightFieldLayer*)(layer))->read(in);
        return layer;
    }
    else if (id==IVEIMAGELAYER)
    {
        osgTerrain::ImageLayer* layer = new osgTerrain::ImageLayer;
        ((ive::ImageLayer*)(layer))->read(in);
        return layer;
    }
    else if (id==IVESWITCHLAYER)
    {
        osgTerrain::SwitchLayer* layer = new osgTerrain::SwitchLayer;
        ((ive::SwitchLayer*)(layer))->read(in);
        return layer;
    }
    else if (id==IVECOMPOSITELAYER)
    {
        osgTerrain::CompositeLayer* layer = new osgTerrain::CompositeLayer;
        ((ive::CompositeLayer*)(layer))->read(in);
        return layer;
    }
    else if (id==IVEPROXYLAYER)
    {
        std::string filename = in->readString();
        osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(filename+".gdal");
        osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<osgTerrain::ProxyLayer*>(object.get());
        
        osg::ref_ptr<osgTerrain::Locator> locator = readLocator(in);
        unsigned int minLevel = in->readUInt();
        unsigned int maxLevel = in->readUInt();
        
        if (proxyLayer)
        {
            if (locator.valid()) proxyLayer->setLocator(locator.get());
            
            proxyLayer->setMinLevel(minLevel);
            proxyLayer->setMaxLevel(maxLevel);
        }
        
        return proxyLayer;
    }
    
    return new osgTerrain::ImageLayer;
}

void LayerHelper::writeLocator(DataOutputStream* out, osgTerrain::Locator* locator)
{
    if (locator) 
    {
        out->writeBool(true);
        
        ((ive::Locator*)(locator))->write(out);
    }
    else
    {
        out->writeBool(false);
    }
}

osgTerrain::Locator* LayerHelper::readLocator(DataInputStream* in)
{
    bool locatorExist = in->readBool();
    if (!locatorExist) return 0;
    
    osgTerrain::Locator* locator = new osgTerrain::Locator;
    
    ((ive::Locator*)(locator))->read(in);
    
    return locator;
}

void Layer::writeValidDataOperator(DataOutputStream* out, osgTerrain::ValidDataOperator* validDataOperator)
{
    if (validDataOperator)
    {
        out->writeBool(true);
        osgTerrain::ValidRange* validRange = dynamic_cast<osgTerrain::ValidRange*>(validDataOperator);
        if (validRange)
        {
            out->writeInt(IVEVALIDRANGE);
            out->writeFloat(validRange->getMinValue());
            out->writeFloat(validRange->getMaxValue());
        }
        else 
        {    
            osgTerrain::NoDataValue* noDataValue  = dynamic_cast<osgTerrain::NoDataValue*>(validDataOperator);
            if (noDataValue)
            {
                out->writeInt(IVENODATAVALUE);
                out->writeFloat(noDataValue->getValue());
            }
        }
    }
    else
    {
        out->writeBool(false);
    }
}

osgTerrain::ValidDataOperator* Layer::readValidDataOperator(DataInputStream* in)
{
    bool hasOperator = in->readBool();
    if (!hasOperator) return 0;
    
    int id = in->peekInt();
    if (id==IVEVALIDRANGE)
    {
        id = in->readInt();
        float minValue = in->readFloat();
        float maxValue = in->readFloat();
        return new osgTerrain::ValidRange(minValue,maxValue);        
    }
    else if (id==IVENODATAVALUE)
    {
        id = in->readInt();
        float value = in->readFloat();
        return new osgTerrain::NoDataValue(value);
    }
    else
    {
        return 0;
    }
}
