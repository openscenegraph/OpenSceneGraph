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
#include <osg/ViewportIndexed>
#include <osg/GLExtensions>
#include <osg/State>

using namespace osg;

ViewportIndexed::ViewportIndexed():
    _index(0)
{
}

ViewportIndexed::~ViewportIndexed()
{
}

void ViewportIndexed::setIndex(unsigned int index)
{
    if (_index==index) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _index = index;
}

void ViewportIndexed::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (extensions->glViewportIndexedf)
    {
        extensions->glViewportIndexedf(static_cast<GLuint>(_index),
                                       static_cast<GLfloat>(_x),
                                       static_cast<GLfloat>(_y),
                                       static_cast<GLfloat>(_width),
                                       static_cast<GLfloat>(_height));
    }
    else
    {
        OSG_WARN<<"Warning: ViewportIndexed::apply(..) failed, glViewportIndexed is not support by OpenGL driver."<<std::endl;
    }
}

