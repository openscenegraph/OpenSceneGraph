#include <osgUtil/RenderGraph>

#include <osg/Notify>

using namespace osg;
using namespace osgUtil;

void RenderGraph::reset()
{
    _parent = NULL;
    _stateset = NULL;

    _depth = 0;

    _children.clear();
    _leaves.clear();
}

/** recursively clean the RenderGraph of all its drawables, lights and depths.
  * Leaves children intact, and ready to be populated again.*/
void RenderGraph::clean()
{

    // clean local drawables etc.
    _leaves.clear();

    // call clean on all children.
    for(ChildList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        itr->second->clean();
    }

}

/** recursively prune the RenderGraph of empty children.*/
void RenderGraph::prune()
{
    std::vector<const osg::StateSet*> toEraseList;

    // call prune on all children.
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        citr->second->prune();

        if (citr->second->empty())
        {
            toEraseList.push_back(citr->first);
        }
    }

    for(std::vector<const osg::StateSet*>::iterator eitr=toEraseList.begin();
        eitr!=toEraseList.end();
        ++eitr)
    {
        _children.erase(*eitr);
    }

}
