#include <osg/OccluderNode>

using namespace osg;

OccluderNode::OccluderNode()
{
}

OccluderNode::OccluderNode(const OccluderNode& node,const CopyOp& copyop):
    Group(node,copyop),
    _occluder(dynamic_cast<ConvexPlanerOccluder*>(copyop(node._occluder.get())))
{
}
