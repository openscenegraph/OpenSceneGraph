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
#include <osgTerrain/GeometryTechnique>

#include <OpenThreads/ScopedLock>

using namespace osg;
using namespace osgTerrain;

Terrain::Terrain():
    _sampleRatio(1.0),
    _verticalScale(1.0)
{
    _terrainTechnique = new GeometryTechnique;
}

Terrain::Terrain(const Terrain& ts, const osg::CopyOp& copyop):
    osg::Group(ts,copyop),
    _sampleRatio(ts._sampleRatio),
    _verticalScale(ts._verticalScale),
    _terrainTechnique(ts._terrainTechnique)
{
}


Terrain::~Terrain()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(TerrainTileSet::iterator itr = _terrainTileSet.begin();
        itr != _terrainTileSet.end();
        ++itr)
    {
        const_cast<TerrainTile*>(*itr)->_terrain = 0;
    }
    
    _terrainTileSet.clear();
    _terrainTileMap.clear();
}

void Terrain::traverse(osg::NodeVisitor& nv)
{
    Group::traverse(nv);
}

TerrainTile* Terrain::getTile(const TileID& tileID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    TerrainTileMap::iterator itr = _terrainTileMap.find(tileID);
    if (itr == _terrainTileMap.end()) return 0;
    
    return itr->second;
}

const TerrainTile* Terrain::getTile(const TileID& tileID) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    TerrainTileMap::const_iterator itr = _terrainTileMap.find(tileID);
    if (itr == _terrainTileMap.end()) return 0;
    
    return itr->second;
}

void Terrain::dirtyRegisteredTiles()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(TerrainTileSet::iterator itr = _terrainTileSet.begin();
        itr != _terrainTileSet.end();
        ++itr)
    {
        (const_cast<TerrainTile*>(*itr))->setDirty(true);
    }
}

static unsigned int s_maxNumTiles = 0;
void Terrain::registerTerrainTile(TerrainTile* tile)
{
    if (!tile) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    
    if (tile->getTileID().valid())
    {
        _terrainTileMap[tile->getTileID()] = tile;
    }
    
    _terrainTileSet.insert(tile);

    if (_terrainTileSet.size() > s_maxNumTiles) s_maxNumTiles = _terrainTileSet.size();

    // osg::notify(osg::NOTICE)<<"Terrain::registerTerrainTile "<<tile<<" total number of tile "<<_terrainTileSet.size()<<" max = "<<s_maxNumTiles<<std::endl;
}

void Terrain::unregisterTerrainTile(TerrainTile* tile)
{
    if (!tile) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    if (tile->getTileID().valid())
    {
        _terrainTileMap.erase(tile->getTileID());
    }
    
    _terrainTileSet.erase(tile);

    // osg::notify(osg::NOTICE)<<"Terrain::unregisterTerrainTile "<<tile<<" total number of tile "<<_terrainTileSet.size()<<" max = "<<s_maxNumTiles<<std::endl;
}
