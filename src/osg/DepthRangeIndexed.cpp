/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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
#include <osg/DepthRangeIndexed>
#include <osg/GLExtensions>
#include <osg/State>

using namespace osg;

DepthRangeIndexed::DepthRangeIndexed():
    _index(0),
    _zNear(0.0),
    _zFar(1.0)
{
}

DepthRangeIndexed::~DepthRangeIndexed()
{
}

void DepthRangeIndexed::setIndex(unsigned int index)
{
    if (_index==index) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _index = index;
}

void DepthRangeIndexed::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (extensions->glDepthRangeIndexed)
    {
        extensions->glDepthRangeIndexed(static_cast<GLuint>(_index), static_cast<GLdouble>(_zNear), static_cast<GLdouble>(_zFar));
    }
    else if (extensions->glDepthRangeIndexedf)
    {
        extensions->glDepthRangeIndexedf(static_cast<GLuint>(_index), static_cast<GLfloat>(_zNear), static_cast<GLfloat>(_zFar));
    }
    else
    {
        OSG_WARN<<"Warning: DepthRangeIndexed::apply(..) failed, glDepthRangeIndexed is not support by OpenGL driver."<<std::endl;
    }
}

