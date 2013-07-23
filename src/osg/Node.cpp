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

#include <osg/Node>
#include <osg/Group>
#include <osg/NodeVisitor>
#include <osg/Notify>
#include <osg/OccluderNode>
#include <osg/Transform>
#include <osg/UserDataContainer>

#include <algorithm>

using namespace osg;

namespace osg
{
    /// Helper class for generating NodePathList.
    class CollectParentPaths : public NodeVisitor
    {
    public:
        CollectParentPaths(const osg::Node* haltTraversalAtNode=0) :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS),
            _haltTraversalAtNode(haltTraversalAtNode)
        {
        }

        virtual void apply(osg::Node& node)
        {
            if (node.getNumParents()==0 || &node==_haltTraversalAtNode)
            {
                _nodePaths.push_back(getNodePath());
            }
            else
            {
                traverse(node);
            }
       }

        const Node*     _haltTraversalAtNode;
        NodePath        _nodePath;
        NodePathList    _nodePaths;
    };
}

Node::Node()
    :Object(true)
{
    _boundingSphereComputed = false;
    _nodeMask = 0xffffffff;

    _numChildrenRequiringUpdateTraversal = 0;

    _numChildrenRequiringEventTraversal = 0;

    _cullingActive = true;
    _numChildrenWithCullingDisabled = 0;

    _numChildrenWithOccluderNodes = 0;
}

Node::Node(const Node& node,const CopyOp& copyop):
        Object(node,copyop),
        _initialBound(node._initialBound),
        _boundingSphere(node._boundingSphere),
        _boundingSphereComputed(node._boundingSphereComputed),
        _parents(), // leave empty as parentList is managed by Group.
        _updateCallback(copyop(node._updateCallback.get())),
        _numChildrenRequiringUpdateTraversal(0), // assume no children yet.
        _numChildrenRequiringEventTraversal(0), // assume no children yet.
        _cullCallback(copyop(node._cullCallback.get())),
        _cullingActive(node._cullingActive),
        _numChildrenWithCullingDisabled(0), // assume no children yet.
        _numChildrenWithOccluderNodes(0),
        _nodeMask(node._nodeMask)
{
    setStateSet(copyop(node._stateset.get()));
}

Node::~Node()
{
    // cleanly detatch any associated stateset (include remove parent links)
    setStateSet(0);
}

void Node::addParent(osg::Group* node)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    _parents.push_back(node);
}

void Node::removeParent(osg::Group* node)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

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

void Node::setStateSet(osg::StateSet* stateset)
{
    // do nothing if nothing changed.
    if (_stateset==stateset) return;

    // track whether we need to account for the need to do a update or event traversal.
    int delta_update = 0;
    int delta_event = 0;

    // remove this node from the current statesets parent list
    if (_stateset.valid())
    {
        _stateset->removeParent(this);
        if (_stateset->requiresUpdateTraversal()) --delta_update;
        if (_stateset->requiresEventTraversal()) --delta_event;
    }

    // set the stateset.
    _stateset = stateset;

    // add this node to the new stateset to the parent list.
    if (_stateset.valid())
    {
        _stateset->addParent(this);
        if (_stateset->requiresUpdateTraversal()) ++delta_update;
        if (_stateset->requiresEventTraversal()) ++delta_event;
    }

    if (delta_update!=0)
    {
        setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+delta_update);
    }

    if (delta_event!=0)
    {
        setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()+delta_event);
    }
}

osg::StateSet* Node::getOrCreateStateSet()
{
    if (!_stateset) setStateSet(new StateSet);
    return _stateset.get();
}

NodePathList Node::getParentalNodePaths(osg::Node* haltTraversalAtNode) const
{
    CollectParentPaths cpp(haltTraversalAtNode);
    const_cast<Node*>(this)->accept(cpp);
    return cpp._nodePaths;
}

MatrixList Node::getWorldMatrices(const osg::Node* haltTraversalAtNode) const
{
    CollectParentPaths cpp(haltTraversalAtNode);
    const_cast<Node*>(this)->accept(cpp);

    MatrixList matrices;

    for(NodePathList::iterator itr = cpp._nodePaths.begin();
        itr != cpp._nodePaths.end();
        ++itr)
    {
        NodePath& nodePath = *itr;
        if (nodePath.empty())
        {
            matrices.push_back(osg::Matrix::identity());
        }
        else
        {
            matrices.push_back(osg::computeLocalToWorld(nodePath));
        }
    }

    return matrices;
}

void Node::setUpdateCallback(NodeCallback* nc)
{
    // if no changes just return.
    if (_updateCallback==nc) return;

    // updated callback has been changed, will need to update
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
            // required on this subgraph.
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
            // required on this subgraph.
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


void Node::setEventCallback(NodeCallback* nc)
{
    // if no changes just return.
    if (_eventCallback==nc) return;

    // event callback has been changed, will need to Event
    // both _EventCallback and possibly the numChildrenRequiringAppTraversal
    // if the number of callbacks changes.


    // Event the parents numChildrenRequiringAppTraversal
    // note, if _numChildrenRequiringEventTraversal!=0 then the
    // parents won't be affected by any app callback change,
    // so no need to inform them.
    if (_numChildrenRequiringEventTraversal==0 && !_parents.empty())
    {
        int delta = 0;
        if (_eventCallback.valid()) --delta;
        if (nc) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                (*itr)->setNumChildrenRequiringEventTraversal(
                        (*itr)->getNumChildrenRequiringEventTraversal()+delta );
            }

        }
    }

    // set the app callback itself.
    _eventCallback = nc;

}

void Node::setNumChildrenRequiringEventTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringEventTraversal==num) return;

    // note, if _EventCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringEventTraversal so no need to inform them.
    if (!_eventCallback && !_parents.empty())
    {

        // need to pass on changes to parents.
        int delta = 0;
        if (_numChildrenRequiringEventTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                (*itr)->setNumChildrenRequiringEventTraversal(
                    (*itr)->getNumChildrenRequiringEventTraversal()+delta
                    );
            }

        }
    }

    // finally Event this objects value.
    _numChildrenRequiringEventTraversal=num;

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
            // required on this subgraph.
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
            // required on this subgraph.
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
            // required on this subgraph.
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

void Node::setDescriptions(const DescriptionList& descriptions)
{
    // only assign a description list (and associated UseDataContainer) if we need to.
    if (!descriptions.empty() || getUserDataContainer())
    {
        getOrCreateUserDataContainer()->setDescriptions(descriptions);
    }
}

Node::DescriptionList& Node::getDescriptions()
{
    return getOrCreateUserDataContainer()->getDescriptions();
}

static OpenThreads::Mutex s_mutex_StaticDescriptionList;
static const Node::DescriptionList& getStaticDescriptionList()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_StaticDescriptionList);
    static Node::DescriptionList s_descriptionList;
    return s_descriptionList;
}

const Node::DescriptionList& Node::getDescriptions() const
{
    if (_userDataContainer) return _userDataContainer->getDescriptions();
    else return getStaticDescriptionList();
}

std::string& Node::getDescription(unsigned int i)
{
    return getOrCreateUserDataContainer()->getDescriptions()[i];
}

const std::string& Node::getDescription(unsigned int i) const
{
    if (_userDataContainer) return _userDataContainer->getDescriptions()[i];
    else return getStaticDescriptionList()[i];
}

unsigned int Node::getNumDescriptions() const
{
    return _userDataContainer ? _userDataContainer->getDescriptions().size() : 0;
}

void Node::addDescription(const std::string& desc)
{
    getOrCreateUserDataContainer()->getDescriptions().push_back(desc);
}

BoundingSphere Node::computeBound() const
{
    return BoundingSphere();
}


void Node::dirtyBound()
{
    if (_boundingSphereComputed)
    {
        _boundingSphereComputed = false;

        // dirty parent bounding sphere's to ensure that all are valid.
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->dirtyBound();
        }

    }
}

void Node::setThreadSafeRefUnref(bool threadSafe)
{
    Object::setThreadSafeRefUnref(threadSafe);

    if (_stateset.valid()) _stateset->setThreadSafeRefUnref(threadSafe);
    if (_updateCallback.valid()) _updateCallback->setThreadSafeRefUnref(threadSafe);
    if (_eventCallback.valid()) _eventCallback->setThreadSafeRefUnref(threadSafe);
    if (_cullCallback.valid()) _cullCallback->setThreadSafeRefUnref(threadSafe);
}

void Node::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_stateset.valid()) _stateset->resizeGLObjectBuffers(maxSize);
    if (_updateCallback.valid()) _updateCallback->resizeGLObjectBuffers(maxSize);
    if (_eventCallback.valid()) _eventCallback->resizeGLObjectBuffers(maxSize);
    if (_cullCallback.valid()) _cullCallback->resizeGLObjectBuffers(maxSize);
}

void Node::releaseGLObjects(osg::State* state) const
{
    if (_stateset.valid()) _stateset->releaseGLObjects(state);
    if (_updateCallback.valid()) _updateCallback->releaseGLObjects(state);
    if (_eventCallback.valid()) _eventCallback->releaseGLObjects(state);
    if (_cullCallback.valid()) _cullCallback->releaseGLObjects(state);
}



