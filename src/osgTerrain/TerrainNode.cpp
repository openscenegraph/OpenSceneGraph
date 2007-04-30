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

TerrainNode::TerrainNode():
    _requiresNormals(true)
{
    setNumChildrenRequiringUpdateTraversal(1);
}

TerrainNode::TerrainNode(const TerrainNode& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _elevationLayer(terrain._elevationLayer),
    _colorLayers(terrain._colorLayers),
    _requiresNormals(terrain._requiresNormals)
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

void TerrainNode::setColorLayer(unsigned int i, osgTerrain::Layer* layer)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i].layer = layer;
}

void TerrainNode::setColorTransferFunction(unsigned int i, osg::TransferFunction* tf)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i].transferFunction = tf;
}

void TerrainNode::setColorFilter(unsigned int i, Filter filter)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i].filter = filter;
}

osg::BoundingSphere TerrainNode::computeBound() const
{
    osg::BoundingSphere bs;
    
    if (_elevationLayer.valid()) bs.expandBy(_elevationLayer->computeBound());

    for(Layers::const_iterator itr = _colorLayers.begin();
        itr != _colorLayers.end();
        ++itr)
    {
        if (itr->layer.valid()) bs.expandBy(itr->layer->computeBound());
    }

    return bs;
}
