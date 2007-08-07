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

#include <osgTerrain/Terrain>

using namespace osg;
using namespace osgTerrain;

Terrain::Terrain():
    _requiresNormals(true),
    _treatBoundariesToValidDataAsDefaultValue(false)
{
    setNumChildrenRequiringUpdateTraversal(1);
    setThreadSafeRefUnref(true);
}

Terrain::Terrain(const Terrain& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _elevationLayer(terrain._elevationLayer),
    _colorLayers(terrain._colorLayers),
    _requiresNormals(terrain._requiresNormals),
    _treatBoundariesToValidDataAsDefaultValue(terrain._treatBoundariesToValidDataAsDefaultValue)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    
    if (terrain.getTerrainTechnique()) setTerrainTechnique(dynamic_cast<TerrainTechnique*>(terrain.getTerrainTechnique()->cloneType()));
}

Terrain::~Terrain()
{
}

void Terrain::traverse(osg::NodeVisitor& nv)
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

void Terrain::init()
{
    if (_terrainTechnique.valid() && _terrainTechnique->isDirty())
    {
        _terrainTechnique->init();
    }    
}


void Terrain::setTerrainTechnique(osgTerrain::TerrainTechnique* terrainTechnique)
{
    if (_terrainTechnique == terrainTechnique) return; 

    if (_terrainTechnique.valid()) _terrainTechnique->_terrain = 0;

    _terrainTechnique = terrainTechnique;
    
    if (_terrainTechnique.valid()) _terrainTechnique->_terrain = this;
    
}


void Terrain::setElevationLayer(osgTerrain::Layer* layer)
{
    _elevationLayer = layer;
}

void Terrain::setColorLayer(unsigned int i, osgTerrain::Layer* layer)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i].layer = layer;
}

void Terrain::setColorTransferFunction(unsigned int i, osg::TransferFunction* tf)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i].transferFunction = tf;
}

void Terrain::setColorFilter(unsigned int i, Filter filter)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i].filter = filter;
}

osg::BoundingSphere Terrain::computeBound() const
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
