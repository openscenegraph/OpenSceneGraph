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

#include <osg/Hint>
#include <osg/StateSet>

using namespace osg;

void Hint::apply(State& /*state*/) const
{
    if (_target==GL_NONE || _mode==GL_NONE) return;

    glHint(_target, _mode);
}

void Hint::setTarget(GLenum target)
{
    if (_target==target) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _target = target;
}

