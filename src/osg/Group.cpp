#include <stdio.h>
#include <math.h>
#include <osg/Group>
#include <osg/BoundingBox>
#include <osg/Transform>

#include <algorithm>

using namespace osg;

Group::Group()
{
}

Group::Group(const Group& group,const CopyOp& copyop):
    Node(group,copyop)
{
    for(ChildList::const_iterator itr=group._children.begin();
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
    for(ChildList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->removeParent(this);
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
        child->addParent(this);

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
        child->removeParent(this);

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
        // first remove for origNode's parent list.
        origNode->removeParent(this);

        // note ref_ptr<> automatically handles decrementing origNode's reference count,
        // and inccrementing newNode's reference count.
        *itr = newNode;

        // register as parent of child.
        newNode->addParent(this);

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

    // note, special handling of the case when a child is an Transform,
    // such that only Transforms which are relative to their parents coordinates frame (i.e this group)
    // are handled, Transform relative to and absolute reference frame are ignored.

    BoundingBox bb;
    bb.init();
    ChildList::const_iterator itr;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = dynamic_cast<const osg::Transform*>(itr->get());
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_TO_PARENTS)
        {
            bb.expandBy((*itr)->getBound());
        }
    }

    if (!bb.isValid()) return false;

    _bsphere._center = bb.center();
    _bsphere._radius = 0.0f;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = dynamic_cast<const osg::Transform*>(itr->get());
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_TO_PARENTS)
        {
            _bsphere.expandRadiusBy((*itr)->getBound());
        }
    }

    return true;
}
