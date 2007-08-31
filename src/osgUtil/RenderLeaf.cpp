/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <osgUtil/StateGraph>
#include <osg/Notify>

using namespace osg;
using namespace osgUtil;

void RenderLeaf::render(osg::RenderInfo& renderInfo,RenderLeaf* previous)
{
    osg::State& state = *renderInfo.getState();

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
        StateGraph* prev_rg = previous->_parent;
        StateGraph* prev_rg_parent = prev_rg->_parent;
        StateGraph* rg = _parent;
        if (prev_rg_parent!=rg->_parent)
        {
            StateGraph::moveStateGraph(state,prev_rg_parent,rg->_parent);

            // send state changes and matrix changes to OpenGL.
            state.apply(rg->getStateSet());

        }
        else if (rg!=prev_rg)
        {

            // send state changes and matrix changes to OpenGL.
            state.apply(rg->getStateSet());

        }
        

        // draw the drawable
        _drawable->draw(renderInfo);
    }
    else
    {
        // apply matrices if required.
        state.applyProjectionMatrix(_projection.get());
        state.applyModelViewMatrix(_modelview.get());

        // apply state if required.
        StateGraph::moveStateGraph(state,NULL,_parent->_parent);

        state.apply(_parent->getStateSet());

        // draw the drawable
        _drawable->draw(renderInfo);
    }
    
    if (_dynamic)
    {
        state.decrementDynamicObjectCount();
    }
    
    // osg::notify(osg::NOTICE)<<"RenderLeaf "<<_drawable->getName()<<" "<<_depth<<std::endl;
}
