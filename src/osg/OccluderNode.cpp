#include <osg/OccluderNode>

using namespace osg;

OccluderNode::OccluderNode()
{
}

OccluderNode::OccluderNode(const OccluderNode& node,const CopyOp& copyop):
    Group(node,copyop),
    _occluder(dynamic_cast<ConvexPlanarOccluder*>(copyop(node._occluder.get())))
{
}

const bool OccluderNode::computeBound() const
{
    bool result = Group::computeBound();
    
    if (getOccluder())
    {
        BoundingBox bb;
        const ConvexPlanarPolygon::VertexList& vertexList = getOccluder()->getOccluder().getVertexList();
        for(ConvexPlanarPolygon::VertexList::const_iterator itr=vertexList.begin();
            itr!=vertexList.end();
            ++itr)
        {
            bb.expandBy(*itr);
        }
        if (bb.valid())
        {
            _bsphere.expandBy(bb);
            _bsphere_computed=true;            
            result = true;
        }
    }
    return result;
}
