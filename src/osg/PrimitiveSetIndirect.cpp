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
 *
 * osg/PrimitiveSetIndirect.cpp
 * Author: Julien Valentin 2016-2017
*/

#include <osg/PrimitiveSetIndirect>
#include <osg/BufferObject>
#include <osg/State>
#include <osg/Notify>
#include <assert.h>

using namespace osg;
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DrawArrayIndirectCommand
//

DrawArraysIndirectCommand::DrawArraysIndirectCommand():BufferData(),MixinVector<DrawArraysIndirectCmd>() {}
DrawArraysIndirectCommand::DrawArraysIndirectCommand(const DrawArraysIndirectCommand& copy,const CopyOp& copyop/*=CopyOp::SHALLOW_COPY*/)
    :BufferData(copy, copyop),MixinVector<DrawArraysIndirectCmd>() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DrawElementIndirectCommand
//
DrawElementsIndirectCommand::DrawElementsIndirectCommand():BufferData(),MixinVector<DrawElementsIndirectCmd>() {}
DrawElementsIndirectCommand::DrawElementsIndirectCommand(const DrawElementsIndirectCommand& copy,const CopyOp& copyop/*=CopyOp::SHALLOW_COPY*/)
    :BufferData(copy, copyop), MixinVector<DrawElementsIndirectCmd>(){
}
MultiDrawElementsIndirectUByte::~MultiDrawElementsIndirectUByte()
{
    releaseGLObjects();
}
/*
void DrawArraysIndirect::draw(State& state, bool) const
{
    GLBufferObject* dibo=_indirectCommand->getBufferObject()->getOrCreateGLBufferObject(state.getContextID());
    state.bindDrawIndirectBufferObject(dibo);

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
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawElementsIndirectUByte
//

void MultiDrawElementsIndirectUByte::draw(State& state, bool useVertexBufferObjects) const
{
    GLBufferObject* dibo=_indirectCommand->getBufferObject()->getOrCreateGLBufferObject(state.getContextID());
    state.bindDrawIndirectBufferObject(dibo);
    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif


    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    assert (useVertexBufferObjects && ebo);

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glMultiDrawElementsIndirect(mode, GL_UNSIGNED_BYTE, (const GLvoid *)(dibo->getOffset(_indirectCommand->getBufferIndex())),_indirectCommand->size(), _stride);

}

void MultiDrawElementsIndirectUByte::accept(PrimitiveFunctor& functor) const
{
   /* if (!empty())
        for(DrawElementsIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
            functor.drawElements(_mode,itcmd->count,&(*this)[itcmd->firstIndex],itcmd->baseVertex);*/
}

void MultiDrawElementsIndirectUByte::accept(PrimitiveIndexFunctor& functor) const
{
    /* if (!empty())
        for(DrawElementsIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
            functor.drawElements(_mode,itcmd->count,&(*this)[itcmd->firstIndex],itcmd->baseVertex); */
}

void MultiDrawElementsIndirectUByte::offsetIndices(int offset)
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
// MultiDrawElementsIndirectUShort
//
MultiDrawElementsIndirectUShort::~MultiDrawElementsIndirectUShort()
{
    releaseGLObjects();
}

void MultiDrawElementsIndirectUShort::draw(State& state, bool useVertexBufferObjects) const
{   GLBufferObject* dibo=_indirectCommand->getBufferObject()->getOrCreateGLBufferObject(state.getContextID());
    state.bindDrawIndirectBufferObject(dibo);

    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif

    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    assert (useVertexBufferObjects && ebo);

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glMultiDrawElementsIndirect(mode, GL_UNSIGNED_SHORT, (const GLvoid *)(dibo->getOffset(_indirectCommand->getBufferIndex())),_indirectCommand->size(),_stride);

}

void MultiDrawElementsIndirectUShort::accept(PrimitiveFunctor& functor) const
{
    /* if (!empty())
        for(DrawElementsIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
            functor.drawElements(_mode,itcmd->count,&(*this)[itcmd->firstIndex],itcmd->baseVertex); */
}

void MultiDrawElementsIndirectUShort::accept(PrimitiveIndexFunctor& functor) const
{
     /* if (!empty())
        for(DrawElementsIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
            functor.drawElements(_mode,itcmd->count,&(*this)[itcmd->firstIndex],itcmd->baseVertex); */
}

void MultiDrawElementsIndirectUShort::offsetIndices(int offset)
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
// MultiDrawElementsIndirectUInt
//
MultiDrawElementsIndirectUInt::~MultiDrawElementsIndirectUInt()
{
    releaseGLObjects();
}

void MultiDrawElementsIndirectUInt::draw(State& state, bool useVertexBufferObjects) const
{
    GLBufferObject* dibo=_indirectCommand->getBufferObject()->getOrCreateGLBufferObject(state.getContextID());
    state.bindDrawIndirectBufferObject(dibo);
    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif


    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    assert (useVertexBufferObjects && ebo);

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glMultiDrawElementsIndirect(mode, GL_UNSIGNED_INT, (const GLvoid *)(dibo->getOffset(_indirectCommand->getBufferIndex())),_indirectCommand->size(),_stride);

}

void MultiDrawElementsIndirectUInt::accept(PrimitiveFunctor& functor) const
{

    /* if (!empty())
        for(DrawElementsIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
            functor.drawElements(_mode,itcmd->count,&(*this)[itcmd->firstIndex],itcmd->baseVertex); */
}

void MultiDrawElementsIndirectUInt::accept(PrimitiveIndexFunctor& functor) const
{
    /* if (!empty())
        for(DrawElementsIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
            functor.drawElements(_mode,itcmd->count,&(*this)[itcmd->firstIndex],itcmd->baseVertex); */

}

void MultiDrawElementsIndirectUInt::offsetIndices(int offset)
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
void MultiDrawArraysIndirect::draw(osg::State& state, bool) const
{
    GLBufferObject* dibo=_indirectCommand->getBufferObject()->getOrCreateGLBufferObject(state.getContextID());
    state.bindDrawIndirectBufferObject(dibo);

    GLExtensions* ext = state.get<GLExtensions>();

    ext->glMultiDrawArraysIndirect(_mode,  (const GLvoid *)(dibo->getOffset(_indirectCommand->getBufferIndex())),_indirectCommand->size(),_stride);

}

void MultiDrawArraysIndirect::accept(PrimitiveFunctor& functor) const
{
    for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
    {
        functor.drawArrays(_mode, itcmd->first, itcmd->count);
    }
}

void MultiDrawArraysIndirect::accept(PrimitiveIndexFunctor& functor) const
{
    for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
    {
        functor.drawArrays(_mode, itcmd->first, itcmd->count);
    }
}

unsigned int MultiDrawArraysIndirect::getNumIndices() const
{
    unsigned int total=0;

    for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
        total+= itcmd->count;
    return total;
}

unsigned int MultiDrawArraysIndirect::index(unsigned int pos) const
{
    DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin();
    for(; itcmd!=_indirectCommand->end(); itcmd++)
    {
        unsigned int count = itcmd->count;
        if (pos<count) break;
        pos -= count;
    }
    return itcmd->first + pos;

}

void MultiDrawArraysIndirect::offsetIndices(int offset)
{

    for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
        itcmd->first += offset;
}

unsigned int MultiDrawArraysIndirect::getNumPrimitives() const
{
    unsigned int total=0;
    switch(_mode)
    {
    case(POINTS):
         for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
            total+=itcmd->count;
    case(LINES):
        for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
           total+=itcmd->count/2;
    case(TRIANGLES):
        for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
           total+=itcmd->count/3;
    case(QUADS):
        for(DrawArraysIndirectCommand::iterator itcmd=_indirectCommand->begin(); itcmd!=_indirectCommand->end(); itcmd++)
           total+=itcmd->count/4;
    case(LINE_STRIP):
    case(LINE_LOOP):
    case(TRIANGLE_STRIP):
    case(TRIANGLE_FAN):
    case(QUAD_STRIP):
    case(PATCHES):
    case(POLYGON):
    {
        unsigned int primcount = _indirectCommand->size();
        return primcount;
    }
    }
    return total;
}
