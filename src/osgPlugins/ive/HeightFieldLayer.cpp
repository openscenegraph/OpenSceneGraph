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
#include "HeightFieldLayer.h"
#include "Layer.h"

#include <osgDB/ReadFile>
#include <osg/io_utils>

using namespace ive;

void HeightFieldLayer::write(DataOutputStream* out)
{
    // Write Layer's identification.
    out->writeInt(IVEHEIGHTFIELDLAYER);

    // If the osg class is inherited by any other class we should also write this to file.
    osgTerrain::Layer*  layer = dynamic_cast<osgTerrain::Layer*>(this);
    if  (layer)
        ((ive::Layer*)(layer))->write(out);
    else
        throw Exception("HeightFieldLayer::write(): Could not cast this osgLayer::HeightFieldLayer to an osgTerrain::Layer.");


    if (getFileName().empty() && getHeightField())
    {
        osg::HeightField* hf = getHeightField();
    
        // using inline heightfield
        out->writeBool(true);        
        if (out->getVersion()>=VERSION_0035)
        {
            // Write HeightField's properties.
            out->writeUInt(hf->getNumColumns());
            out->writeUInt(hf->getNumRows());
            out->writeVec3(hf->getOrigin());
            out->writeFloat(hf->getXInterval());
            out->writeFloat(hf->getYInterval());
            out->writeQuat(hf->getRotation());
            out->writeFloat(hf->getSkirtHeight());
            out->writeUInt(hf->getBorderWidth());

            float maxError = 0.0f;
            
            if (getLocator())
            {
                osg::Vec3d world_origin, world_corner;
                
                getLocator()->convertLocalToModel(osg::Vec3d(0.0,0.0,0.0), world_origin);
                getLocator()->convertLocalToModel(osg::Vec3d(1.0,1.0,0.0), world_corner);
                
                double distance = (world_origin-world_corner).length();
                
                maxError = distance * out->getTerrainMaximumErrorToSizeRatio();
            }

            out->writePackedFloatArray(hf->getFloatArray(), maxError);
        }
        else
        {
            out->writeShape(getHeightField());
        }    

    }
    else
    {
        // using external heightfield file
        out->writeBool(false);        
        out->writeString(getFileName());        
    }

}

void HeightFieldLayer::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVEHEIGHTFIELDLAYER)
        throw Exception("HeightFieldLayer::read(): Expected HeightFieldLayer identification.");
    
    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osgTerrain::Layer*  layer = dynamic_cast<osgTerrain::Layer*>(this);
    if (layer)
        ((ive::Layer*)(layer))->read(in);
    else
        throw Exception("HeightFieldLayer::read(): Could not cast this osgLayer::Layer to an osg::Group.");


    bool useInlineHeightField = in->readBool();
    
    if (useInlineHeightField)
    {
    
        if (in->getVersion()>=VERSION_0035)
        {
            osg::HeightField* hf = new osg::HeightField;
            
            // Read HeightField's properties
            //setColor(in->readVec4());
            unsigned int col = in->readUInt();
            unsigned int row = in->readUInt();        
            hf->allocate(col,row);

            hf->setOrigin(in->readVec3());
            hf->setXInterval(in->readFloat());
            hf->setYInterval(in->readFloat());
            hf->setRotation(in->readQuat());

            hf->setSkirtHeight(in->readFloat());
            hf->setBorderWidth(in->readUInt());

            if (in->getVersion()>=VERSION_0035)
            {
                in->readPackedFloatArray(hf->getFloatArray());
            }
            
            setHeightField(hf);

        }
        else
        {
            osg::Shape* shape = in->readShape();
            setHeightField(dynamic_cast<osg::HeightField*>(shape));
        }    

    }
    else
    {
        std::string filename = in->readString();
        setFileName(filename);
        
        setHeightField(osgDB::readHeightFieldFile(filename,in->getOptions()));
    }

}
