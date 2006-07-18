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

BoundingSphere OccluderNode::computeBound() const
{
    BoundingSphere bsphere(Group::computeBound());
    
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
            bsphere.expandBy(bb);
        }
    }
    return bsphere;
}
