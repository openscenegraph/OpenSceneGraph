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
#include <osg/MultiDrawMeshTasksIndirectCount>

using namespace osg;

MultiDrawMeshTasksIndirectCount::MultiDrawMeshTasksIndirectCount() :
    _offset(0),
    _drawCount(0),
    _maxDrawCount(0),
    _stride(0)
{
    // turn off display lists as they are inappropriate
    setSupportsDisplayList(false);
}

MultiDrawMeshTasksIndirectCount::MultiDrawMeshTasksIndirectCount(GLintptr offset, GLintptr drawCount, GLsizei maxDrawCount, GLsizei stride):
    _offset(offset),
    _drawCount(drawCount),
    _maxDrawCount(maxDrawCount),
    _stride(stride)
{
    // turn off display lists as they are inappropriate
    setSupportsDisplayList(false);
}

MultiDrawMeshTasksIndirectCount::MultiDrawMeshTasksIndirectCount(const MultiDrawMeshTasksIndirectCount& dmt,const CopyOp& copyop):
    Drawable(dmt, copyop),
    _offset(dmt._offset),
    _drawCount(dmt._drawCount),
    _maxDrawCount(dmt._maxDrawCount),
    _stride(dmt._stride)
{
}

MultiDrawMeshTasksIndirectCount::~MultiDrawMeshTasksIndirectCount()
{
}

void MultiDrawMeshTasksIndirectCount::drawImplementation(RenderInfo& renderInfo) const
{
    const GLExtensions* extensions = renderInfo.getState()->get<GLExtensions>();
    if (extensions->isMeshShaderSupported && extensions->glMultiDrawMeshTasksIndirectCountNV)
    {
        extensions->glMultiDrawMeshTasksIndirectCountNV(_offset, _drawCount, _maxDrawCount, _stride);
    }
    else
    {
        OSG_NOTICE<<"glMultiDrawMeshTasksIndirectCountNV not supported. "<<std::endl;
    }
}

