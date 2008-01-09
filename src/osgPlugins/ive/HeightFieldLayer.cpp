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
        // using inline heightfield
        out->writeBool(true);        
        out->writeShape(getHeightField());
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
        osg::Shape* shape = in->readShape();
        setHeightField(dynamic_cast<osg::HeightField*>(shape));
    }
    else
    {
        std::string filename = in->readString();
        setFileName(filename);
        
        setHeightField(osgDB::readHeightFieldFile(filename,in->getOptions()));
    }

}
