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
#include <osg/BlendFunci>
#include <osg/GLExtensions>
#include <osg/State>

using namespace osg;

BlendFunci::BlendFunci():
    _index(0)
{
}


BlendFunci::~BlendFunci()
{
}

void BlendFunci::setIndex(unsigned int buf)
{
    if (_index==buf) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _index = buf;
}

void BlendFunci::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (_source_factor != _source_factor_alpha ||
        _destination_factor != _destination_factor_alpha)
    {
        if (extensions->glBlendFuncSeparatei)
        {
            extensions->glBlendFuncSeparatei(static_cast<GLuint>(_index), _source_factor, _destination_factor, _source_factor_alpha, _destination_factor_alpha);
        }
        else
        {
            OSG_WARN<<"Warning: BlendFunc::apply(..) failed, BlendFuncSeparatei is not support by OpenGL driver."<<std::endl;
        }
    }
    else
    {
        if (extensions->glBlendFunci)
        {
            extensions->glBlendFunci(static_cast<GLuint>(_index), _source_factor, _destination_factor);
        }
        else
        {
            OSG_WARN<<"Warning: BlendFunc::apply(..) failed, BlendFunci is not support by OpenGL driver."<<std::endl;
        }
    }
}

