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

    if (_parents.empty())
    {
        _clipPlaneNum = num;
        return;
    }

    // take a reference to this clip plane to prevent it from going out of scope
    // when we remove it temporarily from its parents.
    osg::ref_ptr<ClipPlane> clipPlaneRef = this;

    // copy the parents as they _parents list will be changed by the subsequent removeAttributes.
    ParentList parents = _parents;

    // remove this attribute from its parents as its position is being changed
    // and would no longer be valid.
    ParentList::iterator itr;
    for(itr = parents.begin();
        itr != parents.end();
        ++itr)
    {
        osg::StateSet* stateset = *itr;
        stateset->removeAttribute(this);
    }

    // assign the clip plane number
    _clipPlaneNum = num;

    // add this attribute back into its original parents with its new position
    for(itr = parents.begin();
        itr != parents.end();
        ++itr)
    {
        osg::StateSet* stateset = *itr;
        stateset->setAttribute(this);
    }
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

