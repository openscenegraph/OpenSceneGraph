#include "osg/Node"
#include "osg/Group"
#include "osg/NodeVisitor"

#include "osg/Notify"

#include <algorithm>

using namespace osg;

Node::Node()
{
    _bsphere_computed = false;
    _userData = NULL;
    _nodeMask = 0xffffffff;
}


Node::~Node()
{
    if (_userData && _memoryAdapter.valid()) _memoryAdapter->unref_data(_userData);
}


void Node::accept(NodeVisitor& nv)
{
    nv.apply(*this);
}


void Node::ascend(NodeVisitor& nv)
{
    std::for_each(_parents.begin(),_parents.end(),NodeAcceptOp(nv));
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
