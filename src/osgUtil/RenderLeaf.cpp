#include <osgUtil/RenderLeaf>
#include <osgUtil/RenderGraph>

using namespace osg;
using namespace osgUtil;

void RenderLeaf::render(State& state,RenderLeaf* previous)
{

    if (previous)
    {

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
        
        state.applyModelViewMatrix(_matrix.get());

        _drawable->draw(state);
    }
    else
    {
        RenderGraph::moveRenderGraph(state,NULL,_parent->_parent);

        // send state changes and matrix changes to OpenGL.
        state.apply(_parent->_stateset.get());

        state.applyModelViewMatrix(_matrix.get());

        _drawable->draw(state);
    }
}
