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

    for(BrickSet::iterator itr = _brickSet.begin();
        itr != _brickSet.end();
        ++itr)
    {
        const_cast<Brick*>(*itr)->_volume = 0;
    }
    
    _brickSet.clear();
    _brickMap.clear();
}

void Volume::traverse(osg::NodeVisitor& nv)
{
    Group::traverse(nv);
}

Brick* Volume::getBrick(const BrickID& BrickID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    BrickMap::iterator itr = _brickMap.find(BrickID);
    if (itr != _brickMap.end()) return 0;
    
    return itr->second;
}

const Brick* Volume::getBrick(const BrickID& BrickID) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    BrickMap::const_iterator itr = _brickMap.find(BrickID);
    if (itr != _brickMap.end()) return 0;
    
    return itr->second;
}

void Volume::dirtyRegisteredBricks()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(BrickSet::iterator itr = _brickSet.begin();
        itr != _brickSet.end();
        ++itr)
    {
        (const_cast<Brick*>(*itr))->setDirty(true);
    }
}

static unsigned int s_maxNumBricks = 0;
void Volume::registerBrick(Brick* Brick)
{
    if (!Brick) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    
    if (Brick->getBrickID().valid())
    {
        _brickMap[Brick->getBrickID()] = Brick;
    }
    
    _brickSet.insert(Brick);

    if (_brickSet.size() > s_maxNumBricks) s_maxNumBricks = _brickSet.size();

    // osg::notify(osg::NOTICE)<<"Volume::registerBrick "<<Brick<<" total number of Brick "<<_brickSet.size()<<" max = "<<s_maxNumBricks<<std::endl;
}

void Volume::unregisterBrick(Brick* Brick)
{
    if (!Brick) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    if (Brick->getBrickID().valid())
    {
        _brickMap.erase(Brick->getBrickID());
    }
    
    _brickSet.erase(Brick);

    // osg::notify(osg::NOTICE)<<"Volume::unregisterBrick "<<Brick<<" total number of Brick "<<_brickSet.size()<<" max = "<<s_maxNumBricks<<std::endl;
}
