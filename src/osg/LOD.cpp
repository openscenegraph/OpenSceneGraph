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
#include <osg/LOD>
#include <osg/CullStack>

#include <algorithm>

using namespace osg;

LOD::LOD():
    _centerMode(USE_BOUNDING_SPHERE_CENTER),
    _radius(-1.0f),
    _rangeMode(DISTANCE_FROM_EYE_POINT)
{
}

LOD::LOD(const LOD& lod,const CopyOp& copyop):
        Group(lod,copyop),
        _centerMode(lod._centerMode),
        _userDefinedCenter(lod._userDefinedCenter),
        _radius(lod._radius),
        _rangeMode(lod._rangeMode),
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
            float required_range = 0;
            if (_rangeMode==DISTANCE_FROM_EYE_POINT)
            {
                required_range = nv.getDistanceToViewPoint(getCenter(),true);
            }
            else
            {
                osg::CullStack* cullStack = dynamic_cast<osg::CullStack*>(&nv);
                if (cullStack && cullStack->getLODScale())
                {
                    required_range = cullStack->clampedPixelSize(getBound()) / cullStack->getLODScale();
                }
                else
                {
                    // fallback to selecting the highest res tile by
                    // finding out the max range
                    for(unsigned int i=0;i<_rangeList.size();++i)
                    {
                        required_range = osg::maximum(required_range,_rangeList[i].first);
                    }
                }
            }
            
            unsigned int numChildren = _children.size();
            if (_rangeList.size()<numChildren) numChildren=_rangeList.size();

            for(unsigned int i=0;i<numChildren;++i)
            {    
                if (_rangeList[i].first<=required_range && required_range<_rangeList[i].second)
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

BoundingSphere LOD::computeBound() const
{
    if (_centerMode==USER_DEFINED_CENTER && _radius>=0.0f)
    {
        return BoundingSphere(_userDefinedCenter,_radius);
    }
    else
    {
        return Group::computeBound();
    }
}

bool LOD::addChild( Node *child )
{
    if (Group::addChild(child))
    {

        if (_children.size()>_rangeList.size()) 
        {
            float maxRange = !_rangeList.empty() ? _rangeList.back().second : 0.0f;

            _rangeList.resize(_children.size(),MinMaxPair(maxRange,maxRange));
        }

        return true;
    }
    return false;
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

bool LOD::removeChildren( unsigned int pos,unsigned int numChildrenToRemove)
{
    if (pos<_rangeList.size()) _rangeList.erase(_rangeList.begin()+pos, osg::minimum(_rangeList.begin()+(pos+numChildrenToRemove), _rangeList.end()) );

    return Group::removeChildren(pos,numChildrenToRemove);
}

void LOD::setRange(unsigned int childNo, float min,float max)
{
    if (childNo>=_rangeList.size()) _rangeList.resize(childNo+1,MinMaxPair(min,min));
    _rangeList[childNo].first=min;
    _rangeList[childNo].second=max;
}
