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
#include <osg/ClipPlane>
#include <osg/StateSet>
#include <osg/Notify>

using namespace osg;

ClipPlane::ClipPlane()
{
    _clipPlane.set(0.0,0.0,0.0,0.0);
    _clipPlaneNum = 0;
}


ClipPlane::~ClipPlane()
{
}

void ClipPlane::setClipPlaneNum(unsigned int num)
{
    if (_clipPlaneNum==num) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _clipPlaneNum = num;
}

unsigned int ClipPlane::getClipPlaneNum() const
{
    return _clipPlaneNum;
}

void ClipPlane::apply(State&) const
{
#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
    glClipPlane((GLenum)(GL_CLIP_PLANE0+_clipPlaneNum),_clipPlane.ptr());
#else
    OSG_NOTICE<<"Warning: ClipPlane::apply(State&) - not supported."<<std::endl;
#endif
}

