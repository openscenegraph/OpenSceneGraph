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

#include <osgTerrain/TerrainTechnique>
#include <osgTerrain/TerrainTile>

using namespace osgTerrain;

/////////////////////////////////////////////////////////////////////////////////////
//
// TerrainNeighbours
//
TerrainNeighbours::TerrainNeighbours()
{
}

TerrainNeighbours::~TerrainNeighbours()
{
    clear();
}

void TerrainNeighbours::addNeighbour(TerrainTile* tile)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_neighboursMutex);
    _neighbours.insert(tile);
}

void TerrainNeighbours::removeNeighbour(TerrainTile* tile)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_neighboursMutex);
    _neighbours.erase(tile);
}

void TerrainNeighbours::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_neighboursMutex);
    _neighbours.clear();
}


bool TerrainNeighbours::containsNeighbour(TerrainTile* tile) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_neighboursMutex);
    return _neighbours.count(tile)!=0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// TerrainTechnique
//
TerrainTechnique::TerrainTechnique():
    _terrainTile(0)
{
    setThreadSafeRefUnref(true);
}

TerrainTechnique::TerrainTechnique(const TerrainTechnique& TerrainTechnique,const osg::CopyOp& copyop):
    osg::Object(TerrainTechnique,copyop),
    _terrainTile(0)
{
}

TerrainTechnique::~TerrainTechnique()
{
}

void TerrainTechnique::setTerrainTile(TerrainTile* tile)
{
    if (_terrainTile==tile) return;

    _neighbours.clear();

    _terrainTile = tile;
}

void TerrainTechnique::init(int /*dirtyMask*/, bool /*assumeMultiThreaded*/)
{
    OSG_NOTICE<<className()<<"::init(..) not implementated yet"<<std::endl;
}

void TerrainTechnique::update(osgUtil::UpdateVisitor* uv)
{
    OSG_NOTICE<<className()<<"::update(..) not implementated yet"<<std::endl;
    if (_terrainTile) _terrainTile->osg::Group::traverse(*uv);
}

void TerrainTechnique::cull(osgUtil::CullVisitor* cv)
{
    OSG_NOTICE<<className()<<"::cull(..) not implementated yet"<<std::endl;
    if (_terrainTile) _terrainTile->osg::Group::traverse(*cv);
}

void TerrainTechnique::cleanSceneGraph()
{
    OSG_NOTICE<<className()<<"::cleanSceneGraph(..) not implementated yet"<<std::endl;
}

void TerrainTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_terrainTile) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_terrainTile->getDirty()) _terrainTile->init(_terrainTile->getDirtyMask(), false);

        osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
        if (uv)
        {
            update(uv);
            return;
        }

    }
    else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
            cull(cv);
            return;
        }
    }

    if (_terrainTile->getDirty())
    {
        _terrainTile->init(_terrainTile->getDirtyMask(), false);
    }

    // otherwise fallback to the Group::traverse()
    _terrainTile->osg::Group::traverse(nv);
}
