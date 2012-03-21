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
#include <osg/Depth>

using namespace osg;

Depth::Depth(Function func,double zNear, double zFar,bool writeMask):
    _func(func),
    _zNear(zNear),
    _zFar(zFar),
    _depthWriteMask(writeMask)
{
}

Depth::~Depth()
{
}

void Depth::apply(State&) const
{
    glDepthFunc((GLenum)_func);
    glDepthMask((GLboolean)_depthWriteMask);
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    glDepthRangef(_zNear,_zFar);
#else
    glDepthRange(_zNear,_zFar);
#endif
}

