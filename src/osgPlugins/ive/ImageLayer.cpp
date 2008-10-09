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
#include "ImageLayer.h"
#include "Layer.h"

#include <osgTerrain/TerrainTile>

#include <osgDB/ReadFile>

using namespace ive;

void ImageLayer::write(DataOutputStream* out)
{
    // Write Layer's identification.
    out->writeInt(IVEIMAGELAYER);

    // If the osg class is inherited by any other class we should also write this to file.
    osgTerrain::Layer*  layer = dynamic_cast<osgTerrain::Layer*>(this);
    if  (layer)
        ((ive::Layer*)(layer))->write(out);
    else
        throw Exception("ImageLayer::write(): Could not cast this osgLayer::ImageLayer to an osgTerrain::Layer.");


    IncludeImageMode imMode = out->getIncludeImageMode(getImage());

    if (getFileName().empty() && imMode==IMAGE_REFERENCE_FILE) imMode = IMAGE_INCLUDE_DATA;

    out->writeChar(imMode);
    out->writeImage(imMode,getImage());

}

void ImageLayer::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVEIMAGELAYER)
        throw Exception("ImageLayer::read(): Expected ImageLayer identification.");
    
    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osgTerrain::Layer*  layer = dynamic_cast<osgTerrain::Layer*>(this);
    if (layer)
        ((ive::Layer*)(layer))->read(in);
    else
        throw Exception("ImageLayer::read(): Could not cast this osgLayer::Layer to an osg::Group.");


    bool deferExternalLayerLoading = osgTerrain::TerrainTile::getTileLoadedCallback().valid() ? 
        osgTerrain::TerrainTile::getTileLoadedCallback()->deferExternalLayerLoading() : false;

    // Should we read image data from stream
    IncludeImageMode includeImg = (IncludeImageMode)in->readChar();
    
    if (includeImg==IMAGE_REFERENCE_FILE && deferExternalLayerLoading)
    {
        setFileName(in->readString());
    }
    else
    {
        setImage(in->readImage(includeImg));
    }
}
