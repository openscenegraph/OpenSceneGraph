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
        
        // tell state about.
        state.haveAppliedAttribute(litr->first.get());
            
    }

    for(TexUnitAttrMatrixListMap::iterator titr=_texAttrListMap.begin();
        titr!=_texAttrListMap.end();
        ++titr)
    {
        state.setActiveTextureUnit(titr->first);
        
        AttrMatrixList attrList = titr->second;
        
        for(AttrMatrixList::iterator litr=attrList.begin();
            litr!=attrList.end();
            ++litr)
        {
            state.applyModelViewMatrix((*litr).second.get());

            // apply the light source.
            litr->first->apply(state);

            // tell state about.
            state.haveAppliedTextureAttribute(titr->first, litr->first.get());

        }

    }
}
