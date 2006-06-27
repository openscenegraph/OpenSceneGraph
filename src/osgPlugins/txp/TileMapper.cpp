/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield 
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

#include "TileMapper.h"
//#include "TXPTileNode.h"
#include "TXPPagedLOD.h"

#include <osg/Material>

using namespace txp;

float TileMapper::getDistanceToEyePoint(const osg::Vec3& pos, bool withLODScale) const
{
    if (withLODScale)
	return (pos-getEyeLocal()).length()*getLODScale();
    else
	return (pos-getEyeLocal()).length();
}

inline TileMapper::value_type distance(const osg::Vec3& coord,const osg::Matrix& matrix)
{

    //std::cout << "distance("<<coord<<", "<<matrix<<")"<<std::endl;

    return -((TileMapper::value_type)coord[0]*(TileMapper::value_type)matrix(0,2)+
	     (TileMapper::value_type)coord[1]*(TileMapper::value_type)matrix(1,2)+
	     (TileMapper::value_type)coord[2]*(TileMapper::value_type)matrix(2,2)+
	     matrix(3,2));
}

float TileMapper::getDistanceFromEyePoint(const osg::Vec3& pos, bool withLODScale) const
{
    const osg::Matrix& matrix = *_modelviewStack.back();
    float dist = distance(pos,matrix);
    
    if (withLODScale)
	return dist*getLODScale();
    else
	return dist;
}

void TileMapper::apply(osg::Node& node)
{
    if (node.getName()=="TileContent") 
    {
        _containsGeode = true;
        return;
    }

    if (isCulled(node))
	return;

    // push the culling mode.
    pushCurrentMask();

    traverse(node);

    // pop the culling mode.
    popCurrentMask();
}

void TileMapper::apply(osg::Group& node)
{
    if (node.getName()=="TileContent") 
    {
        _containsGeode = true;
        return;
    }

    if (isCulled(node))
	return;

    // push the culling mode.
    pushCurrentMask();

    TileIdentifier* tid = dynamic_cast<TileIdentifier*>(node.getUserData());
    
    if (tid)
    {
        _tileStack.push_back(TileStack::value_type(*tid,&node));

        _containsGeode = false;
    }

    traverse(node);

    if (tid)
    {
        if (_containsGeode)
        {
            insertTile(*tid);

            _containsGeode = false;

#if 0
            std::cout<<"found Group="<<tid->lod
		     <<"  X="<<tid->x
		     <<"  Y="<<tid->y
		     <<"  ptr="<<&node<<std::endl;  

            std::cout<<"      inheritance list "<<_tileStack.size()<<std::endl;
            for(TileStack::iterator itr=_tileStack.begin();
                itr!=_tileStack.end();
                ++itr)
            {
                std::cout<<"      LOD="<<itr->first.lod
                         <<" X="<<itr->first.x
                         <<" Y="<<itr->first.y
                         <<" className="<<itr->second->className()
                         <<" ptr="<<itr->second<<std::endl;      
            }

            
            osg::StateSet* stateset = node.getOrCreateStateSet();
            osg::Material* material = new osg::Material;
            material->setColorMode(osg::Material::OFF);
            stateset->setAttribute(material);
            
            switch(tid->lod)
            {
            case(0): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,1.0f)); break;
            case(1): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,0.0f,0.0f,1.0f)); break;
            case(2): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,1.0f,0.0f,1.0f)); break;
            case(3): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,1.0f,1.0f)); break;
            case(4): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,0.0f,1.0f,1.0f)); break;
            }
            
#endif            

        }
    
        _tileStack.pop_back();
    }

    // pop the culling mode.
    popCurrentMask();
}

void TileMapper::apply(osg::Geode&)
{
    _containsGeode = true;
}

void TileMapper::apply(osg::PagedLOD& node)
{
    if (isCulled(node))
	return;

    // push the culling mode.
    pushCurrentMask();

    TXPPagedLOD* txpPagedLOD = dynamic_cast<TXPPagedLOD*>(&node);
    if (txpPagedLOD)
    {
                                   
        _tileStack.push_back(TileStack::value_type(txpPagedLOD->_tileIdentifier,&node));

        _containsGeode = false;

    }

    traverse(node);

    if (txpPagedLOD)
    {
        if (_containsGeode)
        {
            insertTile(txpPagedLOD->_tileIdentifier);

            _containsGeode = false;

#if 0
            std::cout<<"found txpPagedLOD LOD="<<txpPagedLOD->_tileIdentifier.lod
		     <<"  X="<<txpPagedLOD->_tileIdentifier.x
		     <<"  Y="<<txpPagedLOD->_tileIdentifier.y
		     <<"  ptr="<<txpPagedLOD<<std::endl;  

            std::cout<<"      inheritance list "<<_tileStack.size()<<std::endl;
            for(TileStack::iterator itr=_tileStack.begin();
                itr!=_tileStack.end();
                ++itr)
            {
                std::cout<<"      LOD="<<itr->first.lod
                         <<" X="<<itr->first.x
                         <<" Y="<<itr->first.y
                         <<" className="<<itr->second->className()
                         <<" ptr="<<itr->second<<std::endl;      
            }

            osg::StateSet* stateset = txpPagedLOD->getOrCreateStateSet();
            osg::Material* material = new osg::Material;
            material->setColorMode(osg::Material::OFF);
            stateset->setAttribute(material);

            switch(txpPagedLOD->_tileIdentifier.lod)
            {
            case(0): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,1.0f)); break;
            case(1): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,0.0f,0.0f,1.0f)); break;
            case(2): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,1.0f,0.0f,1.0f)); break;
            case(3): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,1.0f,1.0f)); break;
            case(4): material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,0.0f,1.0f,1.0f)); break;
            }
#endif            
        }
        _tileStack.pop_back();
    }

    // pop the culling mode.
    popCurrentMask();
}

void TileMapper::insertTile(const TileIdentifier& tid)
{
    _tileMap.insert(TileMap::value_type(tid,_tileStack));
}

bool TileMapper::canParentBeTraversed(const TileIdentifier& tid) const
{
    // find the tiles parents.
    TileMap::const_iterator itr = _tileMap.find(tid);
    if (itr==_tileMap.end())
    {
#if 0
        // not found tile in _tileMap, what should we do??
        // return true as a fallback right now.
        std::cout<<"TileMapper::canDescend() Not found tile in map"<<std::endl;
#endif
        return true;
    }

    const TileStack& ts = itr->second;
    
    // note tile here, is tid's parent.
    const TileStack::value_type* tile = (ts.size()>=2) ? &ts[ts.size()-2] : 0;

    // note parent here, is tid's parents parent.
    const TileStack::value_type* parent = (ts.size()>=3) ? &ts[ts.size()-3] : 0;
    
    if (!tile)
    {
        // no self!!! so we can descend safely?!! shouldn't ever get here.
        //std::cout<<"TileMapper::canDescend() tile==0"<<std::endl;
        return true;
    }

    if (!parent)
    {
        // no parent so we can descend safely.
        return true;
    }
    
    bool tileHasNorthNeighour = _tileMap.count(TileIdentifier(tid.x,tid.y+1,tid.lod))!=0;
    bool tileHasEastNeighour = _tileMap.count(TileIdentifier(tid.x+1,tid.y,tid.lod))!=0;
    bool tileHasSouthNeighour = _tileMap.count(TileIdentifier(tid.x,tid.y-1,tid.lod))!=0;
    bool tileHasWestNeighour = _tileMap.count(TileIdentifier(tid.x-1,tid.y,tid.lod))!=0;
    
    if (tileHasNorthNeighour && tileHasEastNeighour && tileHasSouthNeighour && tileHasWestNeighour)
    {
        // tile has neigbours on all sides at the same lod level, so its safe to descend.
        //std::cout<<"TileMapper::canDescend() has neightbours on all sides"<<std::endl;
        return true;
    }

    const TileIdentifier& parent_tid = parent->first;
    
    bool parentHasNorthNeighour = _tileMap.count(TileIdentifier(parent_tid.x,parent_tid.y+1,parent_tid.lod))!=0;
    bool parentHasEastNeighour = _tileMap.count(TileIdentifier(parent_tid.x+1,parent_tid.y,parent_tid.lod))!=0;
    bool parentHasSouthNeighour = _tileMap.count(TileIdentifier(parent_tid.x,parent_tid.y-1,parent_tid.lod))!=0;
    bool parentHasWestNeighour = _tileMap.count(TileIdentifier(parent_tid.x-1,parent_tid.y,parent_tid.lod))!=0;
    

    // identify whether the tile is a NE/SE/SW/NW tile relative to its parent.
    osg::Vec3 delta = tile->second->getBound().center() - parent->second->getBound().center();
    
    if (delta.y()>=0.0f) // noth side
    {        
        if (delta.x()>=0.0f)
        {
            // NE, only traverse if our parent doesn't have any neighbours to the north or east.
            return (!parentHasNorthNeighour && !parentHasEastNeighour);
        }
        else 
        {
            // NW, only traverse if our parent doesn't have any neighbours to the north or west.
            return (!parentHasNorthNeighour && !parentHasWestNeighour);
        }
    }
    else // south side
    {
        if (delta.x()>=0.0f)
        {
            // SE, only traverse if our parent doesn't have any neighbours to the south or east.
            return (!parentHasSouthNeighour && !parentHasEastNeighour);
        }
        else 
        {
            // SW, only traverse if our parent doesn't have any neighbours to the south or west.
            return (!parentHasSouthNeighour && !parentHasWestNeighour);
        }
    }
    // we shouldn't get here...
    
    return true;
}

void TileMapper::checkValidityOfAllVisibleTiles()
{
    typedef std::vector<TileIdentifier> ToRemoveList;
    typedef std::vector<TileStack> ToAddList;

    ToRemoveList toRemoveList;
    ToAddList toAddList;

    do
    {
//        std::cout<<"doing checkAllVisibleTiles() loop with "<<_tileMap.size()<<std::endl;
    
        toRemoveList.clear();
        toAddList.clear();

        for(TileMap::iterator itr=_tileMap.begin();
            itr!=_tileMap.end();
            ++itr)
        {
            if (!canParentBeTraversed(itr->first))
            {
                // need to remove.
                toRemoveList.push_back(itr->first);

                // trim the end of itr's TileStack and add into toAddList
                toAddList.push_back(itr->second);

//                 std::cout<<"Tile failed"
//                          <<" LOD="<<itr->first.lod
//                          <<" X="<<itr->first.x
//                          <<" Y="<<itr->first.y<<std::endl;

            }
        }

        for(ToRemoveList::iterator ritr=toRemoveList.begin();
            ritr!=toRemoveList.end();
            ++ritr)
        {
            //std::cout<<"Removing Tile"<<std::endl;
            _tileMap.erase(*ritr);
        }

        for(ToAddList::iterator aitr=toAddList.begin();
            aitr!=toAddList.end();
            ++aitr)
        {
            //std::cout<<"Adding Parents Tile back in"<<std::endl;
            aitr->pop_back();
            _blackListedNodeSet.insert(aitr->back().second);
            _tileMap.insert(TileMap::value_type(aitr->back().first,*aitr));
        }
        
    }
    while (!toRemoveList.empty());
    

#if 0

	if ( !_blackListedNodeSet.empty() )
		std::cout << "********** We have blacked list " << _blackListedNodeSet.size() << std::endl;

    std::cout<<"TileMap contains "<<_tileMap.size()<<std::endl;
    for(TileMap::iterator itr=_tileMap.begin();
        itr!=_tileMap.end();
        ++itr)
    {
	std::cout<<"    tile="<<itr->first.lod
		 <<"  X="<<itr->first.x
		 <<"  Y="<<itr->first.y<<std::endl;  
        
    }
#endif
    
}

bool TileMapper::isTileNeighbourALowerLODLevel(const TileIdentifier& tid, int dx, int dy) const
{
    if (_tileMap.count(TileIdentifier(tid.x+dx,tid.y+dy,tid.lod))!=0)
    {
        // we have a neightbour at the same lod level.
        return false;
    }

    // find the tiles parents.
    TileMap::const_iterator itr = _tileMap.find(tid);
    if (itr==_tileMap.end())
    {
        // not found tile in _tileMap, what should we do??
        // return true as a fallback right now.
#if 0
	std::cout << "TileMapper::isTileNeighbourALowerLODLevel() Not found tile in map," << std::endl;
	std::cout << "    LOD=" << tid.lod << "  X=" << tid.x << "  Y=" << tid.y << std::endl;
#endif 
        return true;
    }

    const TileStack& ts = itr->second;
    
    // note tile here, is tid's parent.
    const TileStack::value_type* tile = (ts.size()>=1) ? &ts[ts.size()-1] : 0;

    if (!tile)
    {
        // no tile, so must assume that neighbor is now at a lower level
        return false;
    }

    // note parent here, is tid's parents parent.
    const TileStack::value_type* parent = (ts.size()>=2) ? &ts[ts.size()-2] : 0;
    
    if (!parent)
    {
        // no parent so we must assume that is not at a lower level
        return false;
    }
    
    const TileIdentifier& parent_tid = parent->first;
    
    bool parentHasNorthNeighour = _tileMap.count(TileIdentifier(parent_tid.x,  parent_tid.y+1,parent_tid.lod))!=0;
    bool parentHasEastNeighour  = _tileMap.count(TileIdentifier(parent_tid.x+1,parent_tid.y,  parent_tid.lod))!=0;
    bool parentHasSouthNeighour = _tileMap.count(TileIdentifier(parent_tid.x,  parent_tid.y-1,parent_tid.lod))!=0;
    bool parentHasWestNeighour  = _tileMap.count(TileIdentifier(parent_tid.x-1,parent_tid.y,  parent_tid.lod))!=0;
    

    // identify whether the tile is a NE/SE/SW/NW tile relative to its parent.
    osg::Vec3 delta = tile->second->getBound().center() - parent->second->getBound().center();
    
    if (delta.y()>=0.0f) // noth side
    {        
        if (delta.x()>=0.0f)
        {
            // NE
            if (dy==1)
		return parentHasNorthNeighour;
            else if (dx==1)
		return parentHasEastNeighour;
        }
        else 
        {
            // NW
            if (dy==1)
		return parentHasNorthNeighour;
            else if (dx==-1)
		return parentHasWestNeighour;
        }
    }
    else // south side
    {
        if (delta.x()>=0.0f)
        {
            // SE
            if (dy==-1)
		return parentHasSouthNeighour;
            else if (dx==1)
		return parentHasEastNeighour;
        }
        else 
        {
            // SW
            if (dy==-1)
		return parentHasSouthNeighour;
            else if (dx==-1)
		return parentHasWestNeighour;
        }
    }
    
    return false;
}
