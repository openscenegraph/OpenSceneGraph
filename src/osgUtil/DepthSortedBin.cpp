#include <osgUtil/DepthSortedBin>
#include <osgUtil/RenderStage>

#include <algorithm>

using namespace osg;
using namespace osgUtil;


#ifndef __DARWIN_OSX__
// this is a hack to test OS X ....
// register a RenderStage prototype with the RenderBin prototype list.
RegisterRenderBinProxy<DepthSortedBin> s_registerDepthSortedBinProxy;
#endif


DepthSortedBin::DepthSortedBin()
{
    _drawOrder = BACK_TO_FRONT;
}

DepthSortedBin::~DepthSortedBin()
{
}

void DepthSortedBin::reset()
{
    RenderBin::reset();
    
    _renderLeafList.clear();
}

struct DepthSortFunctor2
{
    const bool operator() (const RenderLeaf* lhs,const RenderLeaf* rhs)
    {
        return (lhs->_depth<rhs->_depth);
    }
};



void DepthSortedBin::sort_local()
{
    _renderLeafList.clear();

    int totalsize=0;
    RenderGraphList::iterator itr;
    for(itr=_renderGraphList.begin();
        itr!=_renderGraphList.end();
        ++itr)
    {
        totalsize += (*itr)->_leaves.size();
    }

    _renderLeafList.reserve(totalsize);
    
    // first copy all the leaves from the render graphs into the leaf list.
    for(itr=_renderGraphList.begin();
        itr!=_renderGraphList.end();
        ++itr)
    {
        for(RenderGraph::LeafList::iterator dw_itr = (*itr)->_leaves.begin();
            dw_itr != (*itr)->_leaves.end();
            ++dw_itr)
        {
            _renderLeafList.push_back(dw_itr->get());
        }
    }
    
    // now sort the list into acending depth order.
    std::sort(_renderLeafList.begin(),_renderLeafList.end(),DepthSortFunctor2());

}

void DepthSortedBin::draw_local(osg::State& state,RenderLeaf*& previous)
{
    if (_drawOrder==BACK_TO_FRONT)
    {
        // render the bin from back to front.
        for(RenderLeafList::reverse_iterator itr= _renderLeafList.rbegin();
            itr!= _renderLeafList.rend();
            ++itr)
        {
            RenderLeaf* rl = *itr;
            rl->render(state,previous);
            previous = rl;
        }
    }
    else
    {
        // render the from front to back.
        for(RenderLeafList::iterator itr= _renderLeafList.begin();
            itr!= _renderLeafList.end();
            ++itr)
        {
            RenderLeaf* rl = *itr;
            rl->render(state,previous);
            previous = rl;
        }
    }
}
