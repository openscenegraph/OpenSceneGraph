#include "TileMapper.h"
#include <OpenThreads/ScopedLock>

using namespace txp;

TileMapper* TileMapper::instance()
{
    static osg::ref_ptr<TileMapper> s_tilemapper = new TileMapper;
    return s_tilemapper.get();
}

osg::PagedLOD* TileMapper::getPagedLOD(int x, int y, int lod)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    TileMap::iterator itr = _tileMap.find(TileTriple(x,y,lod));
    if (itr!=_tileMap.end()) return itr->second;
    else return 0;
}

void TileMapper::insertPagedLOD(int x, int y, int lod, osg::PagedLOD* pagedLod)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _tileMap[TileTriple(x,y,lod)]=pagedLod;
}

void TileMapper::removePagedLOD(int x, int y, int lod)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    TileMap::iterator itr = _tileMap.find(TileTriple(x,y,lod));
    if (itr!=_tileMap.end()) _tileMap.erase(itr);
}

void TileMapper::prunePagedLOD()
{
//     OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
//     for(TileMap::iterator itr = _tileMap.begin();
//         itr!=_tileMap.end();
//         ++itr)
//     {
//         if (itr->second.valid() && itr->second->referenceCount()==1)
//         {
//             TileMap::iterator eitr = itr;
//             --itr;
//             _tileMap.erase(eitr);
//         }
//     }
    //std::cout<<"_tileMap.size()="<<_tileMap.size()<<std::endl;
}
