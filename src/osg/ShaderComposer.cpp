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

#include <osg/ShaderComposer>
#include <osg/Notify>

using namespace osg;

ShaderComposer::ShaderComposer()
{
    OSG_INFO<<"ShaderComposer::ShaderComposer() "<<this<<std::endl;
}

ShaderComposer::ShaderComposer(const ShaderComposer& sa, const CopyOp& copyop):
    Object(sa, copyop)
{
    OSG_INFO<<"ShaderComposer::ShaderComposer(const ShaderComposer&, const CopyOp& copyop) "<<this<<std::endl;
}

ShaderComposer::~ShaderComposer()
{
    OSG_INFO<<"ShaderComposer::~ShaderComposer() "<<this<<std::endl;
}

