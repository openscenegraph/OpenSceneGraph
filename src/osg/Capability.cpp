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

#include <osg/Capability>
#include <osg/GLExtensions>
#include <osg/State>

using namespace osg;


Capability::Capability():
    _capability(0)
{
}

Capability::~Capability()
{
}

Capabilityi::Capabilityi():
    _index(0)
{
}

Capabilityi::~Capabilityi()
{
}

void Enablei::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (extensions->glEnablei)
    {
        OSG_INFO<<"extensions->glEnablei("<<_capability<<", "<<_index<<")"<<std::endl;
        extensions->glEnablei(_capability, static_cast<GLuint>(_index));
    }
    else
    {
        OSG_WARN<<"Warning: Enablei::apply(..) failed, Enablei is not support by OpenGL driver."<<std::endl;
    }
}

void Disablei::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (extensions->glDisablei)
    {
        OSG_INFO<<"extensions->glDisablei("<<_capability<<", "<<_index<<")"<<std::endl;
        extensions->glDisablei(_capability, static_cast<GLuint>(_index));
    }
    else
    {
        OSG_WARN<<"Warning: Enablei::apply(..) failed, Enablei is not support by OpenGL driver."<<std::endl;
    }
}
