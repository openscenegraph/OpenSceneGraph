#include <osgUtil/RenderLeaf>
#include <osgUtil/RenderGraph>

using namespace osg;
using namespace osgUtil;

// comment if you are are working with vertex programs,
// but it does break osgreflect demo and perhaps others, so keep an eye
// out artifacts.
// #define APPLY_MATICES_BEFORE_STATE

void RenderLeaf::render(State& state,RenderLeaf* previous)
{

    if (previous)
    {

#ifdef APPLY_MATICES_BEFORE_STATE
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());
#endif        
        // apply state if required.
        RenderGraph* prev_rg = previous->_parent;
        RenderGraph* prev_rg_parent = prev_rg->_parent;
        RenderGraph* rg = _parent;
        if (prev_rg_parent!=rg->_parent)
        {
            RenderGraph::moveRenderGraph(state,prev_rg_parent,rg->_parent);

            // send state changes and matrix changes to OpenGL.
            state.apply(rg->_stateset.get());

        }
        else if (rg!=prev_rg)
        {

            // send state changes and matrix changes to OpenGL.
            state.apply(rg->_stateset.get());

        }
        
#ifndef APPLY_MATICES_BEFORE_STATE
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());
#endif

        // draw the drawable
        _drawable->draw(state);
    }
    else
    {
#ifdef APPLY_MATICES_BEFORE_STATE
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());
#endif        

        // apply state if required.
         RenderGraph::moveRenderGraph(state,NULL,_parent->_parent);

        state.apply(_parent->_stateset.get());

#ifndef APPLY_MATICES_BEFORE_STATE
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());
#endif

        // draw the drawable
        _drawable->draw(state);
    }
}
