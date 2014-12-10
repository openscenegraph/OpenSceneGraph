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
#include <osg/BlendFunc>
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

BlendFunc::BlendFunc():
    _source_factor(SRC_ALPHA),
    _destination_factor(ONE_MINUS_SRC_ALPHA),
    _source_factor_alpha(SRC_ALPHA),
    _destination_factor_alpha(ONE_MINUS_SRC_ALPHA)
{
}

BlendFunc::BlendFunc(GLenum source, GLenum destination):
    _source_factor(source),
    _destination_factor(destination),
    _source_factor_alpha(source),
    _destination_factor_alpha(destination)
{
}

BlendFunc::BlendFunc(GLenum source, GLenum destination, GLenum source_alpha, GLenum destination_alpha):
    _source_factor(source),
    _destination_factor(destination),
    _source_factor_alpha(source_alpha),
    _destination_factor_alpha(destination_alpha)
{
}

BlendFunc::~BlendFunc()
{
}

void BlendFunc::apply(State& state) const
{
    if (_source_factor != _source_factor_alpha ||
        _destination_factor != _destination_factor_alpha)
    {
        const GLExtensions* extensions = state.get<GLExtensions>();
        if (!extensions->isBlendFuncSeparateSupported)
        {
            OSG_WARN<<"Warning: BlendFunc::apply(..) failed, BlendFuncSeparate is not support by OpenGL driver, falling back to BlendFunc."<<std::endl;
        }
        else
        {
            extensions->glBlendFuncSeparate(_source_factor, _destination_factor, _source_factor_alpha, _destination_factor_alpha);
            return;
        }
    }

    glBlendFunc( _source_factor, _destination_factor );
}
