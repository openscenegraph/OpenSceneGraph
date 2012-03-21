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
#include <osg/ClearNode>

#include <algorithm>

using namespace osg;

/**
 * ClearNode constructor.
 */
ClearNode::ClearNode():
    _requiresClear(true),
    _clearColor(0.0f,0.0f,0.0f,1.0f),
    _clearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
{
    setCullingActive(false);
    StateSet* stateset = new StateSet;
    stateset->setRenderBinDetails(-1,"RenderBin");
    setStateSet(stateset);
}

