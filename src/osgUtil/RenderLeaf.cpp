/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

// comment if you are are working with vertex programs,
// but it does break osgreflect demo and perhaps others, so keep an eye
// out artifacts.
// #define APPLY_MATRICES_BEFORE_STATE

void RenderLeaf::render(State& state,RenderLeaf* previous)
{

    if (previous)
    {

#ifdef APPLY_MATRICES_BEFORE_STATE
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
        
#ifndef APPLY_MATRICES_BEFORE_STATE
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());
#endif

        // draw the drawable
        _drawable->draw(state);
    }
    else
    {
#ifdef APPLY_MATRICES_BEFORE_STATE
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());
#endif        

        // apply state if required.
         RenderGraph::moveRenderGraph(state,NULL,_parent->_parent);

        state.apply(_parent->_stateset.get());

#ifndef APPLY_MATRICES_BEFORE_STATE
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());
#endif

        // draw the drawable
        _drawable->draw(state);
    }
}
