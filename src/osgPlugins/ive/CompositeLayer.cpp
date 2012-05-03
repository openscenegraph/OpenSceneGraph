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
#include "CompositeLayer.h"
#include "Layer.h"

using namespace ive;

void CompositeLayer::write(DataOutputStream* out)
{
    // Write Layer's identification.
    out->writeInt(IVECOMPOSITELAYER);

    // If the osg class is inherited by any other class we should also write this to file.
    osgTerrain::Layer*  layer = dynamic_cast<osgTerrain::Layer*>(this);
    if  (layer)
        ((ive::Layer*)(layer))->write(out);
    else
        out_THROW_EXCEPTION("CompositeLayer::write(): Could not cast this osgLayer::CompositeLayer to an osgTerrain::Layer.");

    LayerHelper helper;

    out->writeUInt(getNumLayers());
    for(unsigned int i=0; i<getNumLayers(); ++i)
    {
        if(getLayer(i))
        {
            out->writeBool(true);
            helper.writeLayer(out, getLayer(i));
        }
        else
        {
            out->writeBool(false);
            out->writeString(getCompoundName(i));
        }
    }
}

void CompositeLayer::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVECOMPOSITELAYER)
        in_THROW_EXCEPTION("CompositeLayer::read(): Expected CompositeLayer identification.");

    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osgTerrain::Layer*  layer = dynamic_cast<osgTerrain::Layer*>(this);
    if (layer)
        ((ive::Layer*)(layer))->read(in);
    else
        in_THROW_EXCEPTION("CompositeLayer::read(): Could not cast this osgLayer::Layer to an osg::Group.");

    LayerHelper helper;

    unsigned int numLayers = in->readUInt();
    for(unsigned int i=0; i<numLayers; ++i)
    {
        bool readInlineLayer = in->readBool();
        if (readInlineLayer)
        {
            addLayer(helper.readLayer(in));
        }
        else
        {
            addLayer(in->readString());
        }
    }
}
