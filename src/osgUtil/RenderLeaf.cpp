/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osgUtil/RenderLeaf>
#include <osgUtil/RenderGraph>

using namespace osg;
using namespace osgUtil;

void RenderLeaf::render(State& state,RenderLeaf* previous)
{
    // don't draw this leaf if the abort rendering flag has been set.
    if (state.getAbortRendering())
    {
        //cout << "early abort"<<endl;
        return;
    }

    if (previous)
    {

        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());

        // apply state if required.
        RenderGraph* prev_rg = previous->_parent;
        RenderGraph* prev_rg_parent = prev_rg->_parent;
        RenderGraph* rg = _parent;
        if (prev_rg_parent!=rg->_parent)
        {
            RenderGraph::moveRenderGraph(state,prev_rg_parent,rg->_parent);

            // send state changes and matrix changes to OpenGL.
            state.apply(rg->_stateset);

        }
        else if (rg!=prev_rg)
        {

            // send state changes and matrix changes to OpenGL.
            state.apply(rg->_stateset);

        }
        

        // draw the drawable
        _drawable->draw(state);
    }
    else
    {
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());

        // apply state if required.
         RenderGraph::moveRenderGraph(state,NULL,_parent->_parent);

        state.apply(_parent->_stateset);

        // draw the drawable
        _drawable->draw(state);
    }
}
