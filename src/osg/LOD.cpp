/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/LOD>

#include <algorithm>

using namespace osg;

LOD::LOD():
    _centerMode(USE_BOUNDING_SPHERE_CENTER)
{
}

LOD::LOD(const LOD& lod,const CopyOp& copyop):
        Group(lod,copyop),
        _centerMode(lod._centerMode),
        _userDefinedCenter(lod._userDefinedCenter),
        _rangeList(lod._rangeList)
{
}


void LOD::traverse(NodeVisitor& nv)
{
    switch(nv.getTraversalMode())
    {
        case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
            break;
        case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float distance = nv.getDistanceToEyePoint(getCenter(),true);
            unsigned int numChildren = _children.size();
            if (_rangeList.size()<numChildren) numChildren=_rangeList.size();
            
            for(unsigned int i=0;i<numChildren;++i)
            {    
                if (_rangeList[i].first<=distance && distance<_rangeList[i].second)
                {
                    _children[i]->accept(nv);
                }
            }
           break;
        }
        default:
            break;
    }
}

bool LOD::addChild( Node *child )
{
    if (Group::addChild(child))
    {

        if (_children.size()>_rangeList.size()) 
        {
            float maxRange = !_rangeList.empty()?
                 maxRange=_rangeList.back().second : 0.0f;

            _rangeList.resize(_children.size(),MinMaxPair(maxRange,maxRange));
        }

        return true;
    }
    return false;
}

void LOD::childRemoved(unsigned int /*pos*/, unsigned int /*numChildrenToRemove*/)
{
    //std::cout<<"LOD::childRemoved("<<pos<<","<<numChildrenToRemove<<")"<<std::endl;
}

void LOD::childInserted(unsigned int /*pos*/)
{
    //std::cout<<"LOD::childInserted("<<pos<<")"<<std::endl;
}


bool LOD::addChild(Node *child, float min, float max)
{
    if (Group::addChild(child))
    {
        if (_children.size()>_rangeList.size()) _rangeList.resize(_children.size(),MinMaxPair(min,min));
        _rangeList[_children.size()-1].first = min;
        _rangeList[_children.size()-1].second = max;
        return true;
    }
    return false;
}

bool LOD::removeChild( Node *child )
{
    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return false;
    
    _rangeList.erase(_rangeList.begin()+pos);
    
    return Group::removeChild(child);    
}

void LOD::setRange(unsigned int childNo, float min,float max)
{
    if (childNo>=_rangeList.size()) _rangeList.resize(childNo+1,MinMaxPair(min,min));
    _rangeList[childNo].first=min;
    _rangeList[childNo].second=max;
}
