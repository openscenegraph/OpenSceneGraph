#include <stdio.h>
#include <math.h>
#include "osg/Group"
#include "osg/BoundingBox"

#include <algorithm>

#define square(x)   ((x)*(x))

using namespace osg;

Group::Group()
{
}


Group::~Group()
{

    for(ChildList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        Node* child = itr->get();
        ParentList::iterator pitr = std::find(child->_parents.begin(),child->_parents.end(),this);
        if (pitr!=child->_parents.end()) child->_parents.erase(pitr);
    }

}


void Group::traverse(NodeVisitor& nv)
{
    for(ChildList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->accept(nv);
    }
}


bool Group::addChild( Node *child )
{
    if (child && !containsNode(child))
    {
        // note ref_ptr<> automatically handles incrementing child's reference count.
        _children.push_back(child);

        // register as parent of child.
        child->_parents.push_back(this);

        dirtyBound();

        // could now require app traversal thanks to the new subgraph,
        // so need to check and update if required.
        if (child->getNumChildrenRequiringAppTraversal()>0 ||
            child->getAppCallback())
        {
            setNumChildrenRequiringAppTraversal(
                getNumChildrenRequiringAppTraversal()+1
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

        return true;
    }
    else return false;
}


bool Group::removeChild( Node *child )
{
    ChildList::iterator itr = findNode(child);
    if (itr!=_children.end())
    {
        // remove this group from the child parent list.
        ParentList::iterator pitr = std::find(child->_parents.begin(),child->_parents.end(),this);
        if (pitr!=child->_parents.end()) child->_parents.erase(pitr);

        // could now require app traversal thanks to the new subgraph,
        // so need to check and update if required.
        // note, need to do this checking before the erase of the child
        // otherwise child will be invalid.
        if (child->getNumChildrenRequiringAppTraversal()>0 ||
            child->getAppCallback())
        {
            setNumChildrenRequiringAppTraversal(
                getNumChildrenRequiringAppTraversal()-1
                );
        }

        if (child->getNumChildrenWithCullingDisabled()>0 ||
            !child->getCullingActive())
        {
            setNumChildrenWithCullingDisabled(
                getNumChildrenWithCullingDisabled()-1
                );
        }

        // note ref_ptr<> automatically handles decrementing child's reference count.
        _children.erase(itr);
        dirtyBound();


        return true;
    }
    else return false;
}


bool Group::replaceChild( Node *origNode, Node *newNode )
{
    if (newNode==NULL || origNode==newNode) return false;

    ChildList::iterator itr = findNode(origNode);
    if (itr!=_children.end())
    {
        ParentList::iterator pitr = std::find(origNode->_parents.begin(),origNode->_parents.end(),this);
        if (pitr!=origNode->_parents.end()) origNode->_parents.erase(pitr);

        // note ref_ptr<> automatically handles decrementing origNode's reference count,
        // and inccrementing newNode's reference count.
        *itr = newNode;

        // register as parent of child.
        newNode->_parents.push_back(this);

        dirtyBound();


        // could now require app traversal thanks to the new subgraph,
        // so need to check and update if required.
        int delta_numChildrenRequiringAppTraversal = 0;
        if (origNode->getNumChildrenRequiringAppTraversal()>0 ||
            origNode->getAppCallback())
        {
            --delta_numChildrenRequiringAppTraversal;
        }
        if (newNode->getNumChildrenRequiringAppTraversal()>0 ||
            newNode->getAppCallback())
        {
            ++delta_numChildrenRequiringAppTraversal;
        }

        if (delta_numChildrenRequiringAppTraversal!=0)
        {
            setNumChildrenRequiringAppTraversal(
                getNumChildrenRequiringAppTraversal()+delta_numChildrenRequiringAppTraversal
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
                getNumChildrenWithCullingDisabled()-1
                );
        }

        return true;
    }
    else return false;

}

const bool Group::computeBound() const
{

    _bsphere_computed = true;

    _bsphere.init();
    if (_children.empty()) return false;

    BoundingBox bb;
    bb.init();
    ChildList::const_iterator itr;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        bb.expandBy((*itr)->getBound());
    }

    if (!bb.isValid()) return false;

    _bsphere._center = bb.center();
    _bsphere._radius = 0.0f;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        _bsphere.expandRadiusBy((*itr)->getBound());
    }

    return true;
}
