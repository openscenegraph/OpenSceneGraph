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
#include <osg/ClipControl>

#include <osg/GLExtensions>
#include <osg/State>

using namespace osg;

ClipControl::ClipControl(Origin origin, DepthMode depthMode):
    _origin(origin),
    _depthMode(depthMode)
{
}

ClipControl::ClipControl(const ClipControl& clipControl, const CopyOp& copyop):
    StateAttribute(clipControl, copyop),
    _origin(clipControl._origin),
    _depthMode(clipControl._depthMode)
{
}

ClipControl::~ClipControl()
{
}

void ClipControl::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    
    if (!extensions->isClipControlSupported) return;

    extensions->glClipControl((GLenum)_origin, (GLenum)_depthMode);
}
