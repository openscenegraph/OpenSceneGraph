#include <osgUtil/RenderStageLighting>

using namespace osg;
using namespace osgUtil;

// register a RenderStageLighting prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderStageLighting> s_registerRenderStageLightingProxy;

RenderStageLighting::RenderStageLighting()
{
}

RenderStageLighting::~RenderStageLighting()
{
}

void RenderStageLighting::reset()
{
    _attrList.clear();
}

void RenderStageLighting::draw(osg::State& state,RenderLeaf*& previous)
{

    if (previous)
    {
        RenderGraph::moveToRootRenderGraph(state,previous->_parent);
        state.apply();
        previous = NULL;
    }

    // apply the light list.
    for(AttrMatrixList::iterator litr=_attrList.begin();
        litr!=_attrList.end();
        ++litr)
    {
        state.applyModelViewMatrix((*litr).second.get());

        // apply the light source.
        litr->first->apply(state);
            
    }

}
