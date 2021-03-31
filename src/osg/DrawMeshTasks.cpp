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
#include <osg/DrawMeshTasks>

using namespace osg;

DrawMeshTasks::DrawMeshTasks()
{
    // turn off display lists as they are inappropriate
    setSupportsDisplayList(false);
}

DrawMeshTasks::DrawMeshTasks(GLuint first, GLuint count):
    _first(first),
    _count(count)
{
}

DrawMeshTasks::DrawMeshTasks(const DrawMeshTasks& dmt,const CopyOp& copyop):
    Drawable(dmt, copyop),
    _first(dmt._first),
    _count(dmt._count)
{
}

DrawMeshTasks::~DrawMeshTasks()
{
}

void DrawMeshTasks::drawImplementation(RenderInfo& renderInfo) const
{
    const GLExtensions* extensions = renderInfo.getState()->get<GLExtensions>();
    if (extensions->isMeshShaderSupported && extensions->glDrawMeshTasksNV)
    {
        extensions->glDrawMeshTasksNV(_first, _count);
    }
    else
    {
        OSG_NOTICE<<"glDrawMeshTasksNV not supported. "<<std::endl;
    }
}

