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

bool Group::removeChild( Node *child )
{
    return removeChild(getChildIndex(child));
}

bool Group::removeChild(unsigned int pos,unsigned int numChildrenToRemove)
{
    if (pos<_children.size() && numChildrenToRemove>0)
    {
        unsigned int endOfRemoveRange = pos+numChildrenToRemove;
        if (endOfRemoveRange>_children.size())
        {
            notify(DEBUG_INFO)<<"Warning: Group::removeChild(i,numChildrenToRemove) has been passed an excessive number"<<std::endl;
            notify(DEBUG_INFO)<<"         of chilren to remove, trimming just to end of child list."<<std::endl;
            endOfRemoveRange=_children.size();
        }

        unsigned int appCallbackRemoved = 0;
        unsigned int numChildrenWithCullingDisabledRemoved = 0;
        unsigned int numChildrenWithOccludersRemoved = 0;

        for(unsigned i=pos;i<endOfRemoveRange;++i)
        {
            osg::Node* child = _children[i].get();
            // remove this Geode from the child parent list.
            child->removeParent(this);

            if (child->getNumChildrenRequiringAppTraversal()>0 || child->getAppCallback()) ++appCallbackRemoved;

            if (child->getNumChildrenWithCullingDisabled()>0 || !child->getCullingActive()) ++numChildrenWithCullingDisabledRemoved;

            if (child->getNumChildrenWithOccluderNodes()>0 || dynamic_cast<osg::OccluderNode*>(child)) ++numChildrenWithOccludersRemoved;

        }

        _children.erase(_children.begin()+pos,_children.begin()+endOfRemoveRange);

        if (appCallbackRemoved)
        {
            setNumChildrenRequiringAppTraversal(getNumChildrenRequiringAppTraversal()-appCallbackRemoved);
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
                getNumChildrenWithCullingDisabled()+delta_numChildrenWithCullingDisabled
                );
        }

        // could now require disabling of culling thanks to the new subgraph,
        // so need to check and update if required.
        int delta_numChildrenWithOccluderNodes = 0;
        if (origNode->getNumChildrenWithOccluderNodes()>0 ||
            !origNode->getCullingActive())
        {
            --delta_numChildrenWithOccluderNodes;
        }
        if (newNode->getNumChildrenWithOccluderNodes()>0 ||
            !newNode->getCullingActive())
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

bool Group::computeBound() const
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
        const osg::Transform* transform = (*itr)->asTransform();
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_TO_PARENTS)
        {
            bb.expandBy((*itr)->getBound());
        }
    }

    if (!bb.valid()) return false;

    _bsphere._center = bb.center();
    _bsphere._radius = 0.0f;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = (*itr)->asTransform();
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_TO_PARENTS)
        {
            _bsphere.expandRadiusBy((*itr)->getBound());
        }
    }

    return true;
}
