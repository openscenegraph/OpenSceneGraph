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
#include <osg/DrawMeshTasksIndirect>

using namespace osg;

DrawMeshTasksIndirect::DrawMeshTasksIndirect() :
    _offset(0)
{
    // turn off display lists as they are inappropriate
    setSupportsDisplayList(false);
}

DrawMeshTasksIndirect::DrawMeshTasksIndirect(GLintptr offset):
    _offset(offset)
{
}

DrawMeshTasksIndirect::DrawMeshTasksIndirect(const DrawMeshTasksIndirect& dmt,const CopyOp& copyop):
    Drawable(dmt, copyop),
    _offset(dmt._offset)
{
}

DrawMeshTasksIndirect::~DrawMeshTasksIndirect()
{
}

void DrawMeshTasksIndirect::drawImplementation(RenderInfo& renderInfo) const
{
    const GLExtensions* extensions = renderInfo.getState()->get<GLExtensions>();
    if (extensions->isMeshShaderSupported && extensions->glDrawMeshTasksIndirectNV)
    {
        extensions->glDrawMeshTasksIndirectNV(_offset);
    }
    else
    {
        OSG_NOTICE<<"glDrawMeshTasksIndirectNV not supported. "<<std::endl;
    }
}

