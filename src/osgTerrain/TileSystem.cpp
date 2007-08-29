/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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

#include <osgTerrain/TileSystem>

using namespace osgTerrain;

TileSystem::TileSystem()
{
}

TileSystem::TileSystem(const TileSystem& tileSystem,const osg::CopyOp& copyop):
    osg::Object(tileSystem)
{
}

TileSystem::~TileSystem()
{
}

TileID::TileID():
    _tileSystem(0),
    _layer(-1),
    _x(-1),
    _y(-1)
{
}

TileID::TileID(const TileID& tileID,const osg::CopyOp& copyop):
    osg::Object(tileID),
    _tileSystem(tileID._tileSystem),
    _layer(tileID._layer),
    _x(tileID._x),
    _y(tileID._y)    
{
}

TileID::~TileID()
{
}

