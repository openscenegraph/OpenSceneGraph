#include <osgUtil/RenderBin>
#include <osgUtil/RenderStage>

#include <osg/Statistics>

#include <algorithm>

using namespace osg;
using namespace osgUtil;

// register a RenderStage prototype with the RenderBin prototype list.
RegisterRenderBinProxy<RenderBin> s_registerRenderBinProxy;

#ifdef __DARWIN_OSX__
// PJA 10/1/02 - this is a hack to get OS X to build, if this code is included in DepthSortedBin.cpp it isnt executed
// register a RenderStage prototype with the RenderBin prototype list.
#include <osgUtil/DepthSortedBin>
RegisterRenderBinProxy<DepthSortedBin> s_registerDepthSortedBinProxy;
#endif

typedef std::map< std::string, osg::ref_ptr<RenderBin> > RenderBinPrototypeList;

RenderBinPrototypeList* renderBinPrototypeList()
{
    static RenderBinPrototypeList s_renderBinPrototypeList;
    return &s_renderBinPrototypeList;
}

RenderBin* RenderBin::createRenderBin(const std::string& binName)
{
//    cout << "creating RB "<<binName<<std::endl;

    RenderBinPrototypeList::iterator itr = renderBinPrototypeList()->find(binName);
    if (itr != renderBinPrototypeList()->end()) return dynamic_cast<RenderBin*>(itr->second->cloneType());
    else return NULL;
}

void RenderBin::addRenderBinPrototype(RenderBin* proto)
{
    if (proto)
    {
        (*renderBinPrototypeList())[proto->className()] = proto;
//        cout << "Adding RB "<<proto->className()<<std::endl;
    }
}

void RenderBin::removeRenderBinPrototype(RenderBin* proto)
{
    if (proto)
    {
        RenderBinPrototypeList::iterator itr = renderBinPrototypeList()->find(proto->className());
        if (itr != renderBinPrototypeList()->end()) renderBinPrototypeList()->erase(itr);
    }
}

RenderBin::RenderBin()
{
    _binNum = 0;
    _parent = NULL;
    _stage = NULL;
}

RenderBin::~RenderBin()
{
}

void RenderBin::reset()
{
    _renderGraphList.clear();
    _bins.clear();
}

void RenderBin::sort()
{
    for(RenderBinList::iterator itr = _bins.begin();
        itr!=_bins.end();
        ++itr)
    {
        itr->second->sort();
    }
    
    if (_sortLocalCallback.valid()) _sortLocalCallback->sort(this);
    else sort_local();
}

void RenderBin::sort_local()
{
    switch(_sortMode)
    {
        case(SORT_BY_STATE):
            sort_local_by_state();
            break;
        case(SORT_FRONT_TO_BACK):
            sort_local_front_to_back();
            break;
        case(SORT_BACK_TO_FONT):
            sort_local_back_to_front();
            break;
        default:
            break;
    }
}

struct SortByStateFunctor
{
    const bool operator() (const RenderGraph* lhs,const RenderGraph* rhs) const
    {
        return (*(lhs->_stateset)<*(rhs->_stateset));
    }
};

void RenderBin::sort_local_by_state()
{
    // actually we'll do nothing right now, as fine grained sorting by state
    // appears to cost more to do than it saves in draw.  The contents of
    // the RenderGraph leaves is already coasrse grained sorted, this
    // sorting is as a function of the cull traversal.
}

struct FrontToBackSortFunctor
{
    bool operator() (const RenderLeaf* lhs,const RenderLeaf* rhs) const
    {
        return (lhs->_depth<rhs->_depth);
    }
};

    
void RenderBin::sort_local_front_to_back()
{
    copyLeavesFromRenderGraphListToRenderLeafList();

    // now sort the list into acending depth order.
    std::sort(_renderLeafList.begin(),_renderLeafList.end(),FrontToBackSortFunctor());
}

struct BackToFrontSortFunctor
{
    const bool operator() (const RenderLeaf* lhs,const RenderLeaf* rhs) const
    {
        return (rhs->_depth<lhs->_depth);
    }
};

void RenderBin::sort_local_back_to_front()
{
    copyLeavesFromRenderGraphListToRenderLeafList();

    // now sort the list into acending depth order.
    std::sort(_renderLeafList.begin(),_renderLeafList.end(),BackToFrontSortFunctor());
}

void RenderBin::copyLeavesFromRenderGraphListToRenderLeafList()
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
}

RenderBin* RenderBin::find_or_insert(int binNum,const std::string& binName)
{
    // search for appropriate bin.
    RenderBinList::iterator itr = _bins.find(binNum);
    if (itr!=_bins.end()) return itr->second.get();

    // create a renderin bin and insert into bin list.
    RenderBin* rb = RenderBin::createRenderBin(binName);
    if (rb)
    {

        RenderStage* rs = dynamic_cast<RenderStage*>(rb);
        if (rs)
        {
            rs->_binNum = binNum;
            rs->_parent = NULL;
            rs->_stage = rs;
            _stage->addToDependencyList(rs);
        }
        else
        {
            rb->_binNum = binNum;
            rb->_parent = this;
            rb->_stage = _stage;
            _bins[binNum] = rb;
        }
    }
    return rb;
}

void RenderBin::draw(osg::State& state,RenderLeaf*& previous)
{
    // draw first set of draw bins.
    RenderBinList::iterator itr;
    for(itr = _bins.begin();
        itr!=_bins.end() && itr->first<0;
        ++itr)
    {
        itr->second->draw(state,previous);
    }
    
    draw_local(state,previous);

    for(;
        itr!=_bins.end();
        ++itr)
    {
        itr->second->draw(state,previous);
    }
}

void RenderBin::draw_local(osg::State& state,RenderLeaf*& previous)
{
    sort_local();

    // draw local bin.
    for(RenderGraphList::iterator oitr=_renderGraphList.begin();
        oitr!=_renderGraphList.end();
        ++oitr)
    {

        for(RenderGraph::LeafList::iterator dw_itr = (*oitr)->_leaves.begin();
            dw_itr != (*oitr)->_leaves.end();
            ++dw_itr)
        {
            RenderLeaf* rl = dw_itr->get();
            rl->render(state,previous);
            previous = rl;

        }
    }
}

// stats
bool RenderBin::getStats(osg::Statistics* primStats)
{ // different by return type - collects the stats in this renderrBin
    bool somestats=false;
    for(RenderGraphList::iterator oitr=_renderGraphList.begin();
        oitr!=_renderGraphList.end();
        ++oitr)
    {
        
        for(RenderGraph::LeafList::iterator dw_itr = (*oitr)->_leaves.begin();
            dw_itr != (*oitr)->_leaves.end();
            ++dw_itr)
        {
            RenderLeaf* rl = dw_itr->get();
            Drawable* dw= rl->_drawable;
            primStats->addOpaque(); // number of geosets
            if (rl->_modelview.get()) primStats->addMatrix(); // number of matrices
            if (dw)
            {
                // then tot up the primtive types and no vertices.
                dw->accept(*primStats); // use sub-class to find the stats for each drawable
            }
        }
        somestats=true;
    }
    return somestats;
}

void RenderBin::getPrims(osg::Statistics* primStats)
{
    static int ndepth;
    ndepth++;
    for(RenderBinList::iterator itr = _bins.begin();
        itr!=_bins.end();
        ++itr)
    {
        primStats->addBins(1);
        itr->second->getPrims(primStats);
    }
    getStats(primStats);
    ndepth--;

}

bool RenderBin::getPrims(osg::Statistics* primStats, const int nbin)
{ // collect stats for array of bins, maximum nbin 
    // (which will be modified on next call if array of primStats is too small);
    // return 1 for OK;
    static int ndepth;
    bool ok=false;
    ndepth++;
    int nb=primStats[0].getBins();
    if (nb<nbin)
    { // if statement to protect against writing to bins beyond the maximum seen before
        primStats[nb].setBinNo(nb);
        primStats[nb].setDepth(ndepth);
        getStats(primStats+nb);
    }
    primStats[0].addBins(1);
    for(RenderBinList::iterator itr = _bins.begin();
        itr!=_bins.end();
        ++itr)
    {
        if (itr->second->getPrims(primStats, nbin)) ok = true;
    }
    ok=true;
    ndepth--;
    return ok;
}
