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
#include "SwitchLayer.h"
#include "CompositeLayer.h"

using namespace ive;

void SwitchLayer::write(DataOutputStream* out)
{
    // Write Layer's identification.
    out->writeInt(IVESWITCHLAYER);

    // If the osg class is inherited by any other class we should also write this to file.
    osgTerrain::CompositeLayer*  layer = dynamic_cast<osgTerrain::CompositeLayer*>(this);
    if  (layer)
        ((ive::CompositeLayer*)(layer))->write(out);
    else
        throw Exception("SwitchLayer::write(): Could not cast this osgLayer::SwitchLayer to an osgTerrain::Layer.");


    out->writeInt(getActiveLayer());
}

void SwitchLayer::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVESWITCHLAYER)
        throw Exception("SwitchLayer::read(): Expected SwitchLayer identification.");
    
    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osgTerrain::CompositeLayer*  layer = dynamic_cast<osgTerrain::CompositeLayer*>(this);
    if (layer)
        ((ive::CompositeLayer*)(layer))->read(in);
    else
        throw Exception("SwitchLayer::read(): Could not cast this osgLayer::Layer to an osg::Group.");

    setActiveLayer(in->readInt());
}
