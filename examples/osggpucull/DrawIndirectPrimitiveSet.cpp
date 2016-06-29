/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *  Copyright (C) 2014 Pawel Ksiezopolski
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
 *
*/
#include "DrawIndirectPrimitiveSet.h"
#include <osg/State>
#include <osg/buffered_value>
#include <osg/ref_ptr>
#include <osg/GLExtensions>
#include <osg/Drawable>

void DrawArraysIndirect::draw(osg::State& state, bool /*useVertexBufferObjects*/) const
{
// if you want to see how many primitives were rendered - uncomment code below, but
// be warned : it is a serious performance killer ( because of GPU->CPU roundtrip )

// osg::Drawable::Extensions *dext = osg::Drawable::getExtensions( state.getContextID(),true );
// int* tab = (int*)dext->glMapBuffer(GL_DRAW_INDIRECT_BUFFER,GL_READ_ONLY);
// int val = _indirect/sizeof(int);
// OSG_WARN<<"DrawArraysIndirect ("<<val<<"): "<< tab[val] << " " << tab[val+1] << " " << tab[val+2] << " " << tab[val+3] << std::endl;
// dext->glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

   state.get<osg::GLExtensions>()->glDrawArraysIndirect( _mode, reinterpret_cast<const void*>(_indirect) );

}

void MultiDrawArraysIndirect::draw(osg::State& state, bool /*useVertexBufferObjects*/) const
{
   // DrawIndirectGLExtensions *ext = DrawIndirectGLExtensions::getExtensions( state.getContextID(),true );
    state.get<osg::GLExtensions>()->glMultiDrawArraysIndirect( _mode, reinterpret_cast<const void*>(_indirect), _drawcount, _stride );
}
