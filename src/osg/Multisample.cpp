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
#include <osg/GLExtensions>
#include <osg/Multisample>
#include <osg/State>
#include <osg/Notify>
#include <osg/buffered_value>

using namespace osg;


Multisample::Multisample() : _mode(DONT_CARE)
{
    _coverage = 1;
    _invert   = false;
}

Multisample::~Multisample()
{
}

void Multisample::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (!extensions->isMultisampleSupported)
    {
        OSG_WARN<<"Warning: Multisample::apply(..) failed, Multisample is not support by OpenGL driver."<<std::endl;
        return;
    }

    if(extensions->isMultisampleFilterHintSupported)
    {
        glHint(GL_MULTISAMPLE_FILTER_HINT_NV, _mode);
    }

    extensions->glSampleCoverage(_coverage, _invert);
}
