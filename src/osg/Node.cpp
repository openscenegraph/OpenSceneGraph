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

#include <osg/Node>
#include <osg/Group>
#include <osg/NodeVisitor>
#include <osg/Notify>
#include <osg/OccluderNode>

#include <algorithm>

using namespace osg;

Node::Node()
{
    _bsphere_computed = false;
    _nodeMask = 0xffffffff;
    
    _numChildrenRequiringUpdateTraversal = 0;

    _cullingActive = true;
    _numChildrenWithCullingDisabled = 0;

    _numChildrenWithOccluderNodes = 0;
}

Node::Node(const Node& node,const CopyOp& copyop):
        Object(node,copyop),
        _bsphere(node._bsphere),
        _bsphere_computed(node._bsphere_computed),
        _name(node._name),
        _parents(), // leave empty as parentList is managed by Group.
        _updateCallback(node._updateCallback),
        _numChildrenRequiringUpdateTraversal(0), // assume no children yet.
        _cullCallback(node._cullCallback),
        _cullingActive(node._cullingActive),
        _numChildrenWithCullingDisabled(0), // assume no children yet.
        _numChildrenWithOccluderNodes(0),
        _nodeMask(node._nodeMask), 
        _descriptions(node._descriptions),
        _stateset(copyop(node._stateset.get()))
{
}

Node::~Node()
{
}

void Node::addParent(osg::Group* node)
{
    _parents.push_back(node);
}

void Node::removeParent(osg::Group* node)
{
    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),node);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}

void Node::accept(NodeVisitor& nv)
{
    if (nv.validNodeMask(*this)) 
    {
        nv.pushOntoNodePath(this);
        nv.apply(*this);
        nv.popFromNodePath();
    }
}


void Node::ascend(NodeVisitor& nv)
{
    std::for_each(_parents.begin(),_parents.end(),NodeAcceptOp(nv));
}

osg::StateSet* Node::getOrCreateStateSet()
{
    if (!_stateset) _stateset = new StateSet;
    return _stateset.get();
}


void Node::setUpdateCallback(NodeCallback* nc)
{
    // if no changes just return.
    if (_updateCallback==nc) return;
    
    // app callback has been changed, will need to update
    // both _updateCallback and possibly the numChildrenRequiringAppTraversal
    // if the number of callbacks changes.


    // update the parents numChildrenRequiringAppTraversal
    // note, if _numChildrenRequiringUpdateTraversal!=0 then the
    // parents won't be affected by any app callback change,
    // so no need to inform them.
    if (_numChildrenRequiringUpdateTraversal==0 && !_parents.empty())
    {
        int delta = 0;
        if (_updateCallback.valid()) --delta;
        if (nc) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // reqired on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {    
                (*itr)->setNumChildrenRequiringUpdateTraversal(
                        (*itr)->getNumChildrenRequiringUpdateTraversal()+delta );
            }

        }
    }

    // set the app callback itself.
    _updateCallback = nc;

}

void Node::setNumChildrenRequiringUpdateTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringUpdateTraversal==num) return;

    // note, if _updateCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringUpdateTraversal so no need to inform them.
    if (!_updateCallback && !_parents.empty())
    {
    
        // need to pass on changes to parents.        
        int delta = 0;
        if (_numChildrenRequiringUpdateTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // reqired on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {    
                (*itr)->setNumChildrenRequiringUpdateTraversal(
                    (*itr)->getNumChildrenRequiringUpdateTraversal()+delta
                    );
            }

        }
    }
    
    // finally update this objects value.
    _numChildrenRequiringUpdateTraversal=num;
    
}

void Node::setCullingActive(bool active)
{
    // if no changes just return.
    if (_cullingActive == active) return;
    
    // culling active has been changed, will need to update
    // both _cullActive and possibly the parents numChildrenWithCullingDisabled
    // if culling disabled changes.

    // update the parents _numChildrenWithCullingDisabled
    // note, if _numChildrenWithCullingDisabled!=0 then the
    // parents won't be affected by any app callback change,
    // so no need to inform them.
    if (_numChildrenWithCullingDisabled==0 && !_parents.empty())
    {
        int delta = 0;
        if (!_cullingActive) --delta;
        if (!active) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // reqired on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {    
                (*itr)->setNumChildrenWithCullingDisabled(
                        (*itr)->getNumChildrenWithCullingDisabled()+delta );
            }

        }
    }

    // set the cullingActive itself.
    _cullingActive = active;
}

void Node::setNumChildrenWithCullingDisabled(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenWithCullingDisabled==num) return;

    // note, if _cullingActive is false then the
    // parents won't be affected by any changes to
    // _numChildrenWithCullingDisabled so no need to inform them.
    if (_cullingActive && !_parents.empty())
    {
    
        // need to pass on changes to parents.        
        int delta = 0;
        if (_numChildrenWithCullingDisabled>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // reqired on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {    
                (*itr)->setNumChildrenWithCullingDisabled(
                    (*itr)->getNumChildrenWithCullingDisabled()+delta
                    );
            }

        }
    }
    
    // finally update this objects value.
    _numChildrenWithCullingDisabled=num;
}


void Node::setNumChildrenWithOccluderNodes(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenWithOccluderNodes==num) return;

    // note, if this node is a OccluderNode then the
    // parents won't be affected by any changes to
    // _numChildrenWithOccluderNodes so no need to inform them.
    if (!dynamic_cast<OccluderNode*>(this) && !_parents.empty())
    {
    
        // need to pass on changes to parents.        
        int delta = 0;
        if (_numChildrenWithOccluderNodes>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // reqired on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {    
                (*itr)->setNumChildrenWithOccluderNodes(
                    (*itr)->getNumChildrenWithOccluderNodes()+delta
                    );
            }

        }
    }
    
    // finally update this objects value.
    _numChildrenWithOccluderNodes=num;
    
}

bool Node::containsOccluderNodes() const
{
    return _numChildrenWithOccluderNodes>0 || dynamic_cast<const OccluderNode*>(this);
}

bool Node::computeBound() const
{
    _bsphere.init();
    return false;
}


void Node::dirtyBound()
{
    if (_bsphere_computed)
    {
        _bsphere_computed = false;

        // dirty parent bounding sphere's to ensure that all are valid.
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->dirtyBound();
        }

    }
}
