#include <osg/Node>
#include <osg/Group>
#include <osg/NodeVisitor>
#include <osg/Notify>

#include <algorithm>

using namespace osg;

Node::Node()
{
    _bsphere_computed = false;
    _nodeMask = 0xffffffff;
    
    _numChildrenRequiringAppTraversal = 0;

    _cullingActive = true;
    _numChildrenWithCullingDisabled = 0;

}

Node::Node(const Node& node,const CopyOp& copyop):
        Object(node,copyop),
        _bsphere(_bsphere),
        _bsphere_computed(node._bsphere_computed),
        _name(node._name),
        _parents(), // leave empty as parentList is managed by Group.
        _appCallback(node._appCallback),
        _numChildrenRequiringAppTraversal(0), // assume no children yet.
        _cullCallback(node._cullCallback),
        _cullingActive(node._cullingActive),
        _numChildrenWithCullingDisabled(0), // assume no children yet.
        _userData(copyop(node._userData.get())),
        _nodeMask(node._nodeMask), 
        _descriptions(node._descriptions),
        _dstate(copyop(node._dstate.get()))
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

void Node::setAppCallback(NodeCallback* nc)
{
    // if no changes just return.
    if (_appCallback==nc) return;
    
    // app callback has been changed, will need to update
    // both _appCallback and possibly the numChildrenRequiringAppTraversal
    // if the number of callbacks changes.


    // update the parents numChildrenRequiringAppTraversal
    // note, if _numChildrenRequiringAppTraversal!=0 then the
    // parents won't be affected by any app callback change,
    // so no need to inform them.
    if (_numChildrenRequiringAppTraversal==0 && !_parents.empty())
    {
        int delta = 0;
        if (_appCallback.valid()) --delta;
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
                (*itr)->setNumChildrenRequiringAppTraversal(
                        (*itr)->getNumChildrenRequiringAppTraversal()+delta );
            }

        }
    }

    // set the app callback itself.
    _appCallback = nc;

}

void Node::setNumChildrenRequiringAppTraversal(const int num)
{
    // if no changes just return.
    if (_numChildrenRequiringAppTraversal==num) return;

    // note, if _appCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringAppTraversal so no need to inform them.
    if (!_appCallback && !_parents.empty())
    {
    
        // need to pass on changes to parents.        
        int delta = 0;
        if (_numChildrenRequiringAppTraversal>0) --delta;
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
                (*itr)->setNumChildrenRequiringAppTraversal(
                    (*itr)->getNumChildrenRequiringAppTraversal()+delta
                    );
            }

        }
    }
    
    // finally update this objects value.
    _numChildrenRequiringAppTraversal=num;
    
}

void Node::setCullingActive(const bool active)
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

void Node::setNumChildrenWithCullingDisabled(const int num)
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


const bool Node::computeBound() const
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
