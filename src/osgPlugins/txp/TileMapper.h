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

#ifndef __TILEMAPPER_H_
#define __TILEMAPPER_H_

#include "trpage_sys.h"
#include "trpage_read.h"

#include <osg/CullStack>
#include <osg/NodeVisitor>

#include <set>

namespace txp
{

struct TileIdentifier : public osg::Referenced
{
    TileIdentifier():
        x(-1),
        y(-1),
        lod(-1)
	{}

    TileIdentifier(int ax, int ay, int alod):
        x(ax),
        y(ay),
        lod(alod)
	{}
    
    TileIdentifier(const TileIdentifier& rhs):
        osg::Referenced(),
        x(rhs.x),
        y(rhs.y),
        lod(rhs.lod)
	{}

    TileIdentifier& operator = (const TileIdentifier& rhs)
    {
        if (this==&rhs) return *this;
        x = rhs.x;
        y = rhs.y;
        lod = rhs.lod;
        return *this;
    }

    void set(int ax, int ay, int alod)
    {
        x = ax;
        y = ay;
        lod = alod;
    }

    bool operator < (const TileIdentifier& rhs) const
    {
        if (lod<rhs.lod)
            return true;
        if (lod>rhs.lod)
            return false;
        if (x<rhs.x)
            return true;
        if (x>rhs.x)
            return false;
        if (y<rhs.y)
            return true;
        if (y>rhs.y)
            return false;
        return false;
    }

    int x,y,lod;

};

class TileMapper : public osg::NodeVisitor, public osg::CullStack
{
public:
    
    typedef osg::Matrix::value_type value_type;


    TileMapper():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) {}

            
    virtual osg::Vec3 getEyePoint() const
    {
	return getEyeLocal();
    }
    virtual float getDistanceToEyePoint(const osg::Vec3& pos, bool withLODScale) const;
    virtual float getDistanceFromEyePoint(const osg::Vec3& pos, bool withLODScale) const;

    virtual void apply(osg::Node& node);
    virtual void apply(osg::Group& node);
    virtual void apply(osg::Geode& node);
    virtual void apply(osg::PagedLOD& node);
    
    void insertTile(const TileIdentifier& tid);
    
    bool canParentBeTraversed(const TileIdentifier& tid) const;
    
    void checkValidityOfAllVisibleTiles();
    
    bool containsBlackListedNodes() const
    {
        return !_blackListedNodeSet.empty();
    }

    inline bool isNodeBlackListed(const osg::Node* node) const
    {
        return _blackListedNodeSet.count(node)!=0;
    }
    
    bool isTileNeighbourALowerLODLevel(const TileIdentifier& tid, int dx, int dy) const;
    
protected:

    typedef std::vector< std::pair<TileIdentifier,osg::Node*> > TileStack;
    typedef std::map< TileIdentifier, TileStack > TileMap;
    typedef std::set< const osg::Node* > BlacklistedNodeSet;
    
    TileStack           _tileStack;
    TileMap             _tileMap;
    bool                _containsGeode;
    
    BlacklistedNodeSet  _blackListedNodeSet;
    
};

} // namespace

#endif // __TILEMAPPER_H_
