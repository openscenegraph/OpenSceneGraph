#include <osg/LOD>

#include <algorithm>

using namespace osg;

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
        float maxRange = 0.0f;
        if (!_rangeList.empty()) maxRange=_rangeList.back().second;
        if (_children.size()>_rangeList.size()) _rangeList.resize(_children.size(),MinMaxPair(maxRange,maxRange));
        return true;
    }
    return false;
}

bool LOD::removeChild( Node *child )
{
    // find the child's position.
    unsigned int pos=findChildNo(child);
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
