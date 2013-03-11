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

#include <osg/Group>
#include <osg/BoundingBox>
#include <osg/Transform>
#include <osg/OccluderNode>
#include <osg/Notify>

#include <stdio.h>
#include <math.h>

#include <algorithm>

using namespace osg;

Group::Group()
{
}

Group::Group(const Group& group,const CopyOp& copyop):
    Node(group,copyop)
{
    for(NodeList::const_iterator itr=group._children.begin();
        itr!=group._children.end();
        ++itr)
    {
        Node* child = copyop(itr->get());
        if (child) addChild(child);
    }
}

Group::~Group()
{
    // remove reference to this from children's parent lists.
    for(NodeList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->removeParent(this);
    }

}


void Group::traverse(NodeVisitor& nv)
{
    for(NodeList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->accept(nv);
    }
}


bool Group::addChild( Node *child )
{
    return Group::insertChild( _children.size(), child );
}

bool Group::insertChild( unsigned int index, Node *child )
{
    if (!child) return false;

#if ENSURE_CHILD_IS_UNIQUE
    if (containsNode(child))
    {
        OSG_WARN<<"Adding non unique child to osg::Group, ignoring call"<<std::endl;
        return false;
    }
#endif

    if (child)
    {
        // note ref_ptr<> automatically handles incrementing child's reference count.
        if (index >= _children.size())
        {
            index = _children.size();      // set correct index value to be passed to the "childInserted" method
            _children.push_back(child);
        }
        else
        {
            _children.insert(_children.begin()+index, child);
        }

        // register as parent of child.
        child->addParent(this);

        // tell any subclasses that a child has been inserted so that they can update themselves.
        childInserted(index);

        dirtyBound();

        // could now require app traversal thanks to the new subgraph,
        // so need to check and update if required.
        if (child->getNumChildrenRequiringUpdateTraversal()>0 ||
            child->getUpdateCallback())
        {
            setNumChildrenRequiringUpdateTraversal(
                getNumChildrenRequiringUpdateTraversal()+1
                );
        }

        // could now require app traversal thanks to the new subgraph,
        // so need to check and update if required.
        if (child->getNumChildrenRequiringEventTraversal()>0 ||
            child->getEventCallback())
        {
            setNumChildrenRequiringEventTraversal(
                getNumChildrenRequiringEventTraversal()+1
                );
        }

        // could now require disabling of culling thanks to the new subgraph,
        // so need to check and update if required.
        if (child->getNumChildrenWithCullingDisabled()>0 ||
            !child->getCullingActive())
        {
            setNumChildrenWithCullingDisabled(
                getNumChildrenWithCullingDisabled()+1
                );
        }

        if (child->getNumChildrenWithOccluderNodes()>0 ||
            dynamic_cast<osg::OccluderNode*>(child))
        {
            setNumChildrenWithOccluderNodes(
                getNumChildrenWithOccluderNodes()+1
                );
        }

        return true;
    }
    else return false;
}

bool Group::removeChildren(unsigned int pos,unsigned int numChildrenToRemove)
{
    if (pos<_children.size() && numChildrenToRemove>0)
    {
        unsigned int endOfRemoveRange = pos+numChildrenToRemove;
        if (endOfRemoveRange>_children.size())
        {
            OSG_DEBUG<<"Warning: Group::removeChild(i,numChildrenToRemove) has been passed an excessive number"<<std::endl;
            OSG_DEBUG<<"         of chilren to remove, trimming just to end of child list."<<std::endl;
            endOfRemoveRange=_children.size();
        }

        unsigned int updateCallbackRemoved = 0;
        unsigned int eventCallbackRemoved = 0;
        unsigned int numChildrenWithCullingDisabledRemoved = 0;
        unsigned int numChildrenWithOccludersRemoved = 0;

        for(unsigned i=pos;i<endOfRemoveRange;++i)
        {
            osg::Node* child = _children[i].get();
            // remove this Geode from the child parent list.
            child->removeParent(this);

            if (child->getNumChildrenRequiringUpdateTraversal()>0 || child->getUpdateCallback()) ++updateCallbackRemoved;

            if (child->getNumChildrenRequiringEventTraversal()>0 || child->getEventCallback()) ++eventCallbackRemoved;

            if (child->getNumChildrenWithCullingDisabled()>0 || !child->getCullingActive()) ++numChildrenWithCullingDisabledRemoved;

            if (child->getNumChildrenWithOccluderNodes()>0 || dynamic_cast<osg::OccluderNode*>(child)) ++numChildrenWithOccludersRemoved;

        }

        childRemoved(pos,endOfRemoveRange-pos);

        _children.erase(_children.begin()+pos,_children.begin()+endOfRemoveRange);

        if (updateCallbackRemoved)
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-updateCallbackRemoved);
        }

        if (eventCallbackRemoved)
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-eventCallbackRemoved);
        }

        if (numChildrenWithCullingDisabledRemoved)
        {
            setNumChildrenWithCullingDisabled(getNumChildrenWithCullingDisabled()-numChildrenWithCullingDisabledRemoved);
        }

        if (numChildrenWithOccludersRemoved)
        {
            setNumChildrenWithOccluderNodes(getNumChildrenWithOccluderNodes()-numChildrenWithOccludersRemoved);
        }

        dirtyBound();

        return true;
    }
    else return false;
}


bool Group::replaceChild( Node *origNode, Node *newNode )
{
    if (newNode==NULL || origNode==newNode) return false;

    unsigned int pos = getChildIndex(origNode);
    if (pos<_children.size())
    {
        return setChild(pos,newNode);
    }
    return false;
}


bool Group::setChild( unsigned  int i, Node* newNode )
{
    if (i<_children.size() && newNode)
    {

        ref_ptr<Node> origNode = _children[i];

        // first remove for origNode's parent list.
        origNode->removeParent(this);

        // note ref_ptr<> automatically handles decrementing origNode's reference count,
        // and inccrementing newNode's reference count.
        _children[i] = newNode;

        // register as parent of child.
        newNode->addParent(this);

        dirtyBound();


        // could now require update traversal thanks to the new subgraph,
        // so need to check and update if required.
        int delta_numChildrenRequiringUpdateTraversal = 0;
        if (origNode->getNumChildrenRequiringUpdateTraversal()>0 ||
            origNode->getUpdateCallback())
        {
            --delta_numChildrenRequiringUpdateTraversal;
        }
        if (newNode->getNumChildrenRequiringUpdateTraversal()>0 ||
            newNode->getUpdateCallback())
        {
            ++delta_numChildrenRequiringUpdateTraversal;
        }

        if (delta_numChildrenRequiringUpdateTraversal!=0)
        {
            setNumChildrenRequiringUpdateTraversal(
                getNumChildrenRequiringUpdateTraversal()+delta_numChildrenRequiringUpdateTraversal
                );
        }

        // could now require event traversal thanks to the new subgraph,
        // so need to check and Event if required.
        int delta_numChildrenRequiringEventTraversal = 0;
        if (origNode->getNumChildrenRequiringEventTraversal()>0 ||
            origNode->getEventCallback())
        {
            --delta_numChildrenRequiringEventTraversal;
        }
        if (newNode->getNumChildrenRequiringEventTraversal()>0 ||
            newNode->getEventCallback())
        {
            ++delta_numChildrenRequiringEventTraversal;
        }

        if (delta_numChildrenRequiringEventTraversal!=0)
        {
            setNumChildrenRequiringEventTraversal(
                getNumChildrenRequiringEventTraversal()+delta_numChildrenRequiringEventTraversal
                );
        }

        // could now require disabling of culling thanks to the new subgraph,
        // so need to check and update if required.
        int delta_numChildrenWithCullingDisabled = 0;
        if (origNode->getNumChildrenWithCullingDisabled()>0 ||
            !origNode->getCullingActive())
        {
            --delta_numChildrenWithCullingDisabled;
        }
        if (newNode->getNumChildrenWithCullingDisabled()>0 ||
            !newNode->getCullingActive())
        {
            ++delta_numChildrenWithCullingDisabled;
        }

        if (delta_numChildrenWithCullingDisabled!=0)
        {
            setNumChildrenWithCullingDisabled(
                getNumChildrenWithCullingDisabled()+delta_numChildrenWithCullingDisabled
                );
        }

        // could now require disabling of culling thanks to the new subgraph,
        // so need to check and update if required.
        int delta_numChildrenWithOccluderNodes = 0;
        if (origNode->getNumChildrenWithOccluderNodes()>0 ||
            dynamic_cast<osg::OccluderNode*>(origNode.get()))
        {
            --delta_numChildrenWithOccluderNodes;
        }
        if (newNode->getNumChildrenWithOccluderNodes()>0 ||
            dynamic_cast<osg::OccluderNode*>(newNode))
        {
            ++delta_numChildrenWithOccluderNodes;
        }

        if (delta_numChildrenWithOccluderNodes!=0)
        {
            setNumChildrenWithOccluderNodes(
                getNumChildrenWithOccluderNodes()+delta_numChildrenWithOccluderNodes
                );
        }

        return true;
    }
    else return false;

}

BoundingSphere Group::computeBound() const
{
    BoundingSphere bsphere;
    if (_children.empty())
    {
        return bsphere;
    }

    // note, special handling of the case when a child is an Transform,
    // such that only Transforms which are relative to their parents coordinates frame (i.e this group)
    // are handled, Transform relative to and absolute reference frame are ignored.

    BoundingBox bb;
    bb.init();
    NodeList::const_iterator itr;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = (*itr)->asTransform();
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_RF)
        {
            bb.expandBy((*itr)->getBound());
        }
    }

    if (!bb.valid())
    {
        return bsphere;
    }

    bsphere._center = bb.center();
    bsphere._radius = 0.0f;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = (*itr)->asTransform();
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_RF)
        {
            bsphere.expandRadiusBy((*itr)->getBound());
        }
    }

    return bsphere;
}

void Group::setThreadSafeRefUnref(bool threadSafe)
{
    Node::setThreadSafeRefUnref(threadSafe);

    for(NodeList::const_iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->setThreadSafeRefUnref(threadSafe);
    }
}

void Group::resizeGLObjectBuffers(unsigned int maxSize)
{
    Node::resizeGLObjectBuffers(maxSize);

    for(NodeList::const_iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }
}

void Group::releaseGLObjects(osg::State* state) const
{
    Node::releaseGLObjects(state);

    for(NodeList::const_iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }
}
