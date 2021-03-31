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
#include <osg/MultiDrawMeshTasksIndirect>

using namespace osg;

MultiDrawMeshTasksIndirect::MultiDrawMeshTasksIndirect() :
    _offset(0),
    _drawCount(0),
    _stride(0)
{
    // turn off display lists as they are inappropriate
    setSupportsDisplayList(false);
}

MultiDrawMeshTasksIndirect::MultiDrawMeshTasksIndirect(GLintptr offset, GLsizei drawCount, GLsizei stride):
    _offset(offset),
    _drawCount(drawCount),
    _stride(stride)
{
    // turn off display lists as they are inappropriate
    setSupportsDisplayList(false);
}

MultiDrawMeshTasksIndirect::MultiDrawMeshTasksIndirect(const MultiDrawMeshTasksIndirect& dmt,const CopyOp& copyop):
    Drawable(dmt, copyop),
    _offset(dmt._offset),
    _drawCount(dmt._drawCount),
    _stride(dmt._stride)
{
}

MultiDrawMeshTasksIndirect::~MultiDrawMeshTasksIndirect()
{
}

void MultiDrawMeshTasksIndirect::drawImplementation(RenderInfo& renderInfo) const
{
    const GLExtensions* extensions = renderInfo.getState()->get<GLExtensions>();
    if (extensions->isMeshShaderSupported && extensions->glMultiDrawMeshTasksIndirectNV)
    {
        extensions->glMultiDrawMeshTasksIndirectNV(_offset, _drawCount, _stride);
    }
    else
    {
        OSG_NOTICE<<"glMultiDrawMeshTasksIndirectNV not supported. "<<std::endl;
    }
}

