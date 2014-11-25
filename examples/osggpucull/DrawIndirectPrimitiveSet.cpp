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

void DrawArraysIndirect::draw(osg::State& state, bool useVertexBufferObjects) const 
{
    if( !_buffer.valid() )
        return;
    _buffer->bindBufferAs( state.getContextID(), GL_DRAW_INDIRECT_BUFFER );

// if you want to see how many primitives were rendered - uncomment code below, but 
// be warned : it is a serious performance killer ( because of GPU->CPU roundtrip )
    
// osg::Drawable::Extensions *dext = osg::Drawable::getExtensions( state.getContextID(),true );
// int* tab = (int*)dext->glMapBuffer(GL_DRAW_INDIRECT_BUFFER,GL_READ_ONLY);
// int val = _indirect/sizeof(int);
// OSG_WARN<<"DrawArraysIndirect ("<<val<<"): "<< tab[val] << " " << tab[val+1] << " " << tab[val+2] << " " << tab[val+3] << std::endl;
// dext->glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    
    DrawIndirectGLExtensions *ext = DrawIndirectGLExtensions::getExtensions( state.getContextID(),true );
    ext->glDrawArraysIndirect( _mode, reinterpret_cast<const void*>(_indirect) );
    _buffer->unbindBufferAs( state.getContextID(), GL_DRAW_INDIRECT_BUFFER );
}

void MultiDrawArraysIndirect::draw(osg::State& state, bool useVertexBufferObjects) const 
{
    if( !_buffer.valid() )
        return;
    _buffer->bindBufferAs( state.getContextID(), GL_DRAW_INDIRECT_BUFFER );
    
    DrawIndirectGLExtensions *ext = DrawIndirectGLExtensions::getExtensions( state.getContextID(),true );
    ext->glMultiDrawArraysIndirect( _mode, reinterpret_cast<const void*>(_indirect), _drawcount, _stride );
    _buffer->unbindBufferAs( state.getContextID(), GL_DRAW_INDIRECT_BUFFER );
}

DrawIndirectGLExtensions::DrawIndirectGLExtensions( unsigned int contextID )
{
    setupGLExtensions( contextID );
}

DrawIndirectGLExtensions::DrawIndirectGLExtensions( const DrawIndirectGLExtensions &rhs )
  : Referenced()
{
    _glDrawArraysIndirect           = rhs._glDrawArraysIndirect;
    _glMultiDrawArraysIndirect      = rhs._glMultiDrawArraysIndirect;
    _glMemoryBarrier                = rhs._glMemoryBarrier;
}


void DrawIndirectGLExtensions::lowestCommonDenominator( const DrawIndirectGLExtensions &rhs )
{
    if ( !rhs._glDrawArraysIndirect )
    {
        _glDrawArraysIndirect = rhs._glDrawArraysIndirect;
    }
    if ( !rhs._glMultiDrawArraysIndirect )
    {
        _glMultiDrawArraysIndirect = rhs._glMultiDrawArraysIndirect;
    }
    if ( !rhs._glMemoryBarrier )
    {
        _glMemoryBarrier = rhs._glMemoryBarrier;
    }
}

void DrawIndirectGLExtensions::setupGLExtensions( unsigned int contextID )
{
    _glDrawArraysIndirect = 0;
    _glMultiDrawArraysIndirect = 0;
    osg::setGLExtensionFuncPtr( _glDrawArraysIndirect, "glDrawArraysIndirect" );
    osg::setGLExtensionFuncPtr( _glMultiDrawArraysIndirect, "glMultiDrawArraysIndirect" );
    osg::setGLExtensionFuncPtr( _glMemoryBarrier, "glMemoryBarrier" );
}

void DrawIndirectGLExtensions::glDrawArraysIndirect(GLenum  mode,  const void * indirect) const
{
    if ( _glDrawArraysIndirect )
    {
        _glDrawArraysIndirect( mode, indirect );
    }
    else
    {
        OSG_WARN<<"Error: glDrawArraysIndirect not supported by OpenGL driver"<<std::endl;
    }
}

void DrawIndirectGLExtensions::glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    if ( _glMultiDrawArraysIndirect )
    {
        _glMultiDrawArraysIndirect( mode, indirect, drawcount, stride );
    }
    else
    {
        OSG_WARN<<"Error: glMultiDrawArraysIndirect not supported by OpenGL driver"<<std::endl;
    }    
}

void DrawIndirectGLExtensions::glMemoryBarrier(GLbitfield barriers)
{
    if ( _glMemoryBarrier )
    {
        _glMemoryBarrier( barriers );
    }
    else
    {
        OSG_WARN<<"Error: glMemoryBarrier not supported by OpenGL driver"<<std::endl;
    }
}


typedef osg::buffered_value< osg::ref_ptr<DrawIndirectGLExtensions> > BufferedDrawIndirectGLExtensions;
static BufferedDrawIndirectGLExtensions bdiExtensions;

DrawIndirectGLExtensions* DrawIndirectGLExtensions::getExtensions( unsigned int contextID,bool createIfNotInitalized )
{
    if ( !bdiExtensions[contextID] && createIfNotInitalized )
    {
        bdiExtensions[contextID] = new DrawIndirectGLExtensions( contextID );
    }
    return bdiExtensions[contextID].get();
}
