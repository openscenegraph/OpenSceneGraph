#include <osg/LOD>

#include <algorithm>

using namespace osg;

LOD::LOD(const LOD& lod,const CopyOp& copyop):
        Group(lod,copyop),
        _rangeList(lod._rangeList),
        _rangeList2(lod._rangeList2),
        _center(lod._center)
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
            if (_children.size()!=0) _children.front()->accept(nv);
            break;
        default:
            break;
    }
}


void LOD::setRange(const unsigned int index, const float range)
{
    if (index<_rangeList.size()) _rangeList[index] = range;
    else while (index>=_rangeList.size()) _rangeList.push_back(range);

    if (index<_rangeList2.size()) _rangeList2[index] = range*range;
    else while (index>=_rangeList2.size()) _rangeList2.push_back(range*range);
}


const int LOD::evaluate(const Vec3& eye_local, const float bias) const
{
    // For cache coherency, use _rangeList2 exclusively
    if (_rangeList2.empty()) return -1;

    // Test distance-squared against the stored array of squared ranges
    float LODRange = (eye_local-_center).length2()*bias;
    if (LODRange<_rangeList2[0]) return -1;

    unsigned int end_marker = _rangeList2.size()-1;
    if (end_marker>_children.size()) end_marker=_children.size();
    for(unsigned int i=0;i<end_marker;++i)
    {
        if (LODRange<_rangeList2[i+1])
        {
            return i;
        }
    }
    return -1;
}

