/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield 
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

#include <osgVolume/Volume>
#include <OpenThreads/ScopedLock>

using namespace osgVolume;

Volume::Volume()
{
}

Volume::Volume(const Volume& ts, const osg::CopyOp& copyop):
    osg::Group(ts,copyop)
{
}


Volume::~Volume()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(VolumeTileSet::iterator itr = _volumeTileSet.begin();
        itr != _volumeTileSet.end();
        ++itr)
    {
        const_cast<VolumeTile*>(*itr)->_volume = 0;
    }
    
    _volumeTileSet.clear();
    _volumeTileMap.clear();
}

void Volume::traverse(osg::NodeVisitor& nv)
{
    Group::traverse(nv);
}

VolumeTile* Volume::getVolumeTile(const TileID& tileID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    VolumeTileMap::iterator itr = _volumeTileMap.find(tileID);
    if (itr != _volumeTileMap.end()) return 0;
    
    return itr->second;
}

const VolumeTile* Volume::getVolumeTile(const TileID& tileID) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    VolumeTileMap::const_iterator itr = _volumeTileMap.find(tileID);
    if (itr != _volumeTileMap.end()) return 0;
    
    return itr->second;
}

void Volume::dirtyRegisteredVolumeTiles()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(VolumeTileSet::iterator itr = _volumeTileSet.begin();
        itr != _volumeTileSet.end();
        ++itr)
    {
        (const_cast<VolumeTile*>(*itr))->setDirty(true);
    }
}

static unsigned int s_maxNumVolumeTiles = 0;
void Volume::registerVolumeTile(VolumeTile* volumeTile)
{
    if (!volumeTile) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    
    if (volumeTile->getTileID().valid())
    {
        _volumeTileMap[volumeTile->getTileID()] = volumeTile;
    }
    
    _volumeTileSet.insert(volumeTile);

    if (_volumeTileSet.size() > s_maxNumVolumeTiles) s_maxNumVolumeTiles = _volumeTileSet.size();

    // osg::notify(osg::NOTICE)<<"Volume::registerVolumeTile "<<volumeTile<<" total number of VolumeTile "<<_volumeTileSet.size()<<" max = "<<s_maxNumVolumeTiles<<std::endl;
}

void Volume::unregisterVolumeTile(VolumeTile* volumeTile)
{
    if (!volumeTile) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    if (volumeTile->getTileID().valid())
    {
        _volumeTileMap.erase(volumeTile->getTileID());
    }
    
    _volumeTileSet.erase(volumeTile);

    // osg::notify(osg::NOTICE)<<"Volume::unregisterVolumeTile "<<volumeTile<<" total number of VolumeTile "<<_volumeTileSet.size()<<" max = "<<s_maxNumVolumeTiles<<std::endl;
}
