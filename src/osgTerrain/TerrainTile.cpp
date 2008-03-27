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

#include <osgTerrain/TerrainTile>
#include <osgTerrain/Terrain>

#include <osg/ClusterCullingCallback>

using namespace osg;
using namespace osgTerrain;

TerrainTile::TerrainTile():
    _terrain(0),
    _hasBeenTraversal(false),
    _requiresNormals(true),
    _treatBoundariesToValidDataAsDefaultValue(false)
{
    //setNumChildrenRequiringUpdateTraversal(1);
    setThreadSafeRefUnref(true);
}

TerrainTile::TerrainTile(const TerrainTile& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _terrain(0),
    _hasBeenTraversal(false),
    _elevationLayer(terrain._elevationLayer),
    _colorLayers(terrain._colorLayers),
    _requiresNormals(terrain._requiresNormals),
    _treatBoundariesToValidDataAsDefaultValue(terrain._treatBoundariesToValidDataAsDefaultValue)
{
    //setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    
    if (terrain.getTerrainTechnique()) setTerrainTechnique(dynamic_cast<TerrainTechnique*>(terrain.getTerrainTechnique()->cloneType()));
}

TerrainTile::~TerrainTile()
{
    if (_terrain) setTerrain(0);
}

void TerrainTile::setTerrain(Terrain* ts)
{
    if (_terrain == ts) return;
    
    if (_terrain) _terrain->unregisterTerrainTile(this);
    
    _terrain = ts;

    if (_terrain) _terrain->registerTerrainTile(this);
}

void TerrainTile::setTileID(const TileID& tileID)
{
    if (_tileID == tileID) return;

    if (_terrain) _terrain->unregisterTerrainTile(this);

    _tileID = tileID;

    if (_terrain) _terrain->registerTerrainTile(this);
}


void TerrainTile::traverse(osg::NodeVisitor& nv)
{
    if (!_hasBeenTraversal)
    {
        if (!_terrain)
        {
            osg::NodePath& nodePath = nv.getNodePath();
            if (!nodePath.empty())
            {
                for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
                    itr != nodePath.rend() && !_terrain;
                    ++itr)
                {
                    osgTerrain::Terrain* ts = dynamic_cast<Terrain*>(*itr);
                    if (ts) 
                    {
                        osg::notify(osg::INFO)<<"Assigning terrain system "<<ts<<std::endl;                        
                        setTerrain(ts);
                    }
                }
            }
        }
            
        _hasBeenTraversal = true;
    }

    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osg::ClusterCullingCallback* ccc = dynamic_cast<osg::ClusterCullingCallback*>(getCullCallback());
        if (ccc)
        {
            if (ccc->cull(&nv,0,static_cast<State *>(0))) return;
        }
    }

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->traverse(nv);
    }
    else
    {
        osg::Group::traverse(nv);
    }
}

void TerrainTile::init()
{
    if (_terrainTechnique.valid() && _terrainTechnique->isDirty())
    {
        _terrainTechnique->init();
    }    
}

void TerrainTile::setTerrainTechnique(TerrainTechnique* terrainTechnique)
{
    if (_terrainTechnique == terrainTechnique) return; 

    if (_terrainTechnique.valid()) _terrainTechnique->_terrainTile = 0;

    _terrainTechnique = terrainTechnique;
    
    if (_terrainTechnique.valid()) _terrainTechnique->_terrainTile = this;
    
}


void TerrainTile::setElevationLayer(Layer* layer)
{
    _elevationLayer = layer;
}

void TerrainTile::setColorLayer(unsigned int i, Layer* layer)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i] = layer;
}

osg::BoundingSphere TerrainTile::computeBound() const
{
    osg::BoundingSphere bs;
    
    if (_elevationLayer.valid())
    {
        bs.expandBy(_elevationLayer->computeBound(true));
    }
    else
    {
        for(Layers::const_iterator itr = _colorLayers.begin();
            itr != _colorLayers.end();
            ++itr)
        {
            if (itr->valid()) bs.expandBy((*itr)->computeBound(false));
        }
    }
    
    return bs;
}
