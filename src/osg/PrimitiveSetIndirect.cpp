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
 * Copyright (C) 2016 Julien Valentin
*/
#include <osg/PrimitiveSetIndirect>
#include <osg/BufferObject>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

#define BINDINDIRECTCOMMAND \
    if(_indirectCommand.valid())\
    {\
        GLBufferObject* glBufferObject = _indirectCommand->getBufferObject()->getOrCreateGLBufferObject(state.getContextID());\
        if (glBufferObject && glBufferObject->isDirty())\
        {\
            OSG_WARN<<"Compiling IndirectDraw buffer in draw...Should not happens if GPU production would had happened earlier ..."<<glBufferObject<<std::endl;\
            glBufferObject->compileBuffer();\
        }\
        state.get<GLExtensions>()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER,glBufferObject->getGLObjectID());\
    }\

    //else
    //{
    //    ///No _indirectCommand assuming user have bound a DrawIndirectBufferBinding to the current stateset
    //}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DrawArrayIndirect
//
void DrawArraysIndirect::draw(State& state, bool, bool) const
{
BINDINDIRECTCOMMAND
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    GLenum mode = _mode;
    if (_mode==GL_QUADS)
    {
        state.drawQuads(_first, _count, _numInstances);
        return;
    }
    else if (mode==GL_POLYGON)
    {
        mode = GL_TRIANGLE_FAN;
    }
    else if (mode==GL_QUAD_STRIP)
    {
        mode = GL_TRIANGLE_STRIP;
    }

    if (_numInstances>=1) state.glDrawArraysInstanced(mode,_first,_count, _numInstances);
    else glDrawArrays(mode,_first,_count);
#else
   // if (_numInstances>=1) state.glDrawArraysItInstanced(_mode,_first,_count, _numInstances);
    //else
    state.get<GLExtensions>()->glDrawArraysIndirect(_mode,_indirect);
#endif
}

void DrawArraysIndirect::accept(PrimitiveFunctor& functor) const
{
    //cant mimic GPU stored drawcall functor.drawArrays(_mode,_first,_count);
}

void DrawArraysIndirect::accept(PrimitiveIndexFunctor& functor) const
{
    //cant mimic GPU stored drawcall functor.drawArrays(_mode,_first,_count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DrawElementsIndirectUByte
//
DrawElementsIndirectUByte::~DrawElementsIndirectUByte()
{
    releaseGLObjects();
}

void DrawElementsIndirectUByte::draw(State& state, bool useVertexBufferObjects, bool bindElementBuffer) const
{
BINDINDIRECTCOMMAND
    GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    if (useVertexBufferObjects)
    {
        GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());
        if(bindElementBuffer)state.bindElementBufferObject(ebo);
        if (ebo)
        {
            //if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_BYTE, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
           // else
            state.get<GLExtensions>()->glDrawElementsIndirect(mode, GL_UNSIGNED_BYTE, _indirect);
        }
        else
        {
            //if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_BYTE, &front(), _numInstances);
            state.get<GLExtensions>()->glDrawElementsIndirect(mode,  GL_UNSIGNED_BYTE, _indirect);
        }
    }
    else
    {
        //if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_BYTE, &front(), _numInstances);else
        state.get<GLExtensions>()->glDrawElementsIndirect(mode,  GL_UNSIGNED_BYTE, _indirect);
    }
}

void DrawElementsIndirectUByte::accept(PrimitiveFunctor& functor) const
{
    //cant mimic GPU stored drawcall  if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsIndirectUByte::accept(PrimitiveIndexFunctor& functor) const
{
     //cant mimic GPU stored drawcall if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsIndirectUByte::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DrawElementsIndirectUShort
//
DrawElementsIndirectUShort::~DrawElementsIndirectUShort()
{
    releaseGLObjects();
}

void DrawElementsIndirectUShort::draw(State& state, bool useVertexBufferObjects, bool bindElementBuffer) const
{
BINDINDIRECTCOMMAND
    GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    if (useVertexBufferObjects)
    {
        GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());
        if(bindElementBuffer)state.bindElementBufferObject(ebo);
        if (ebo)
        {
           // if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_SHORT, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
             state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_SHORT,_indirect);
        }
        else
        {
            //if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_SHORT, &front(), _numInstances);
             state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_SHORT,_indirect);
        }
    }
    else
    {
       // if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_SHORT, &front(), _numInstances);
         state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_SHORT, _indirect);
    }
}

void DrawElementsIndirectUShort::accept(PrimitiveFunctor& functor) const
{
    //cant mimic GPU stored drawcall if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsIndirectUShort::accept(PrimitiveIndexFunctor& functor) const
{
    //cant mimic GPU stored drawcall if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsIndirectUShort::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DrawElementsIndirectUInt
//
DrawElementsIndirectUInt::~DrawElementsIndirectUInt()
{
    releaseGLObjects();
}

void DrawElementsIndirectUInt::draw(State& state, bool useVertexBufferObjects, bool bindElementBuffer) const
{
BINDINDIRECTCOMMAND
    GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    if (useVertexBufferObjects)
    {
        GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());
        if(bindElementBuffer)state.bindElementBufferObject(ebo);
        if (ebo)
        {
             //if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
             state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex())));
        }
        else
        {
            // if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_INT, &front(), _numInstances);
            state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_INT, &front());
        }
    }
    else
    {
        //if (_numInstances>=1) state.glDrawElementsIndirectInstanced(mode, size(), GL_UNSIGNED_INT, &front(), _numInstances);
        state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_INT, &front());
    }
}

void DrawElementsIndirectUInt::accept(PrimitiveFunctor& functor) const
{
    //cant mimic GPU stored drawcall  if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsIndirectUInt::accept(PrimitiveIndexFunctor& functor) const
{
    //cant mimic GPU stored drawcall   if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsIndirectUInt::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawArrays
//
#ifdef OSG_HAS_MULTIDRAWARRAYS
void MultiDrawArraysIndirect::draw(osg::State& state, bool, bool) const
{
BINDINDIRECTCOMMAND
    // OSG_NOTICE<<"osg::MultiDrawArraysIndirect::draw"<<std::endl;

    GLExtensions* ext = state.get<GLExtensions>();
    if (ext->glMultiDrawArrays)
    {
      //  GLsizei primcount = osg::minimum(_firsts.size(), _counts.size());

        ext->glMultiDrawArraysIndirect(_mode, _indirect,_drawcount,_stride);
    }
}

void MultiDrawArraysIndirect::accept(PrimitiveFunctor& functor) const
{
    /*unsigned int primcount = osg::minimum(_firsts.size(), _counts.size());
    for(unsigned int i=0; i<primcount; ++i)
    {
        functor.drawArrays(_mode, _firsts[i], _counts[i]);
    }*/
}

void MultiDrawArraysIndirect::accept(PrimitiveIndexFunctor& functor) const
{
    /*unsigned int primcount = osg::minimum(_firsts.size(), _counts.size());
    for(unsigned int i=0; i<primcount; ++i)
    {
        functor.drawArrays(_mode, _firsts[i], _counts[i]);
    }*/
}

unsigned int MultiDrawArraysIndirect::getNumIndices() const
{
  unsigned int total=0;
     /* for(Counts::const_iterator itr = _counts.begin(); itr!=_counts.end(); ++itr)
    {
        total += *itr;
    }*/
    return total;
}

unsigned int MultiDrawArraysIndirect::index(unsigned int pos) const
{
   /* unsigned int i;
    for(i=0; i<_counts.size(); ++i)
    {
        unsigned int count = _counts[i];
        if (pos<count) break;
        pos -= count;
    }
    if (i>=_firsts.size()) return 0;

    return _firsts[i] + pos;*/
    return 0;
}

void MultiDrawArraysIndirect::offsetIndices(int offset)
{
    /*for(Firsts::iterator itr = _firsts.begin(); itr!=_firsts.end(); ++itr)
    {
        *itr += offset;
    }*/
}

unsigned int MultiDrawArraysIndirect::getNumPrimitives() const
{
    switch(_mode)
    {
        case(POINTS): return getNumIndices();
        case(LINES): return getNumIndices()/2;
        case(TRIANGLES): return getNumIndices()/3;
        case(QUADS): return getNumIndices()/4;
        case(LINE_STRIP):
        case(LINE_LOOP):
        case(TRIANGLE_STRIP):
        case(TRIANGLE_FAN):
        case(QUAD_STRIP):
        case(PATCHES):
        case(POLYGON):
        {
            //unsigned int primcount = osg::minimum(_firsts.size(), _counts.size());
            //return primcount;
        }
    }
    return 0;
}
/*
void MultiDrawArraysIndirect::add(GLint first, GLsizei count)
{
    _firsts.push_back(first);
    _counts.push_back(count);
}*/
#endif
