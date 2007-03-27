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

#include <osgTerrain/TerrainNode>

using namespace osg;
using namespace osgTerrain;

TerrainNode::TerrainNode()
{
    setNumChildrenRequiringUpdateTraversal(1);
}

TerrainNode::TerrainNode(const TerrainNode& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _elevationLayer(terrain._elevationLayer),
    _colorLayer(terrain._colorLayer),
    _colorTransferFunction(terrain._colorTransferFunction)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    
    if (terrain.getTerrainTechnique()) setTerrainTechnique(dynamic_cast<TerrainTechnique*>(terrain.getTerrainTechnique()->cloneType()));
}

TerrainNode::~TerrainNode()
{
}

void TerrainNode::traverse(osg::NodeVisitor& nv)
{
    if (_terrainTechnique.valid())
    {
        _terrainTechnique->traverse(nv);
    }
    else
    {
        osg::Group::traverse(nv);
    }
}

void TerrainNode::setTerrainTechnique(osgTerrain::TerrainTechnique* terrainTechnique)
{
    if (_terrainTechnique == terrainTechnique) return; 

    if (_terrainTechnique.valid()) _terrainTechnique->_terrainNode = 0;

    _terrainTechnique = terrainTechnique;
    
    if (_terrainTechnique.valid()) _terrainTechnique->_terrainNode = this;
    
}


void TerrainNode::setElevationLayer(osgTerrain::Layer* layer)
{
    _elevationLayer = layer;
}

void TerrainNode::setColorLayer(osgTerrain::Layer* layer)
{
    _colorLayer = layer;
}

void TerrainNode::setColorTransferFunction(osg::TransferFunction* tf)
{
    _colorTransferFunction = tf;
}
