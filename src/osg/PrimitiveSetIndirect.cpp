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

///  TODO: add base vertex feature to PrimitiveFunctor and PrimitiveIndexFunctor
//#define PRIMFUNCTORBASEVERTEX 1

using namespace osg;
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DrawElementsIndirect
//
template<class T>
inline unsigned int getNumPrimitivesDI( const T&_this )
{
    unsigned int offset= _this.getFirstCommandToDraw();
    IndirectCommandDrawElements *cmd=const_cast<IndirectCommandDrawElements *>(_this.getIndirectCommandArray());
    unsigned int total=0;
    switch(_this.getMode())
    {
        case(PrimitiveSet::POINTS):
            return cmd->count(offset);
        case(PrimitiveSet::LINES):
            return cmd->count(offset)/2;
        case(PrimitiveSet::TRIANGLES):
            return cmd->count(offset)/3;
        case(PrimitiveSet::QUADS):
            return cmd->count(offset)/4;
        case(PrimitiveSet::LINE_STRIP):
        case(PrimitiveSet::LINE_LOOP):
        case(PrimitiveSet::TRIANGLE_STRIP):
        case(PrimitiveSet::TRIANGLE_FAN):
        case(PrimitiveSet::QUAD_STRIP):
        case(PrimitiveSet::PATCHES):
        case(PrimitiveSet::POLYGON):
        {
            return 1;
        }
    }
    return total;
}

unsigned int DrawElementsIndirectUInt::getNumPrimitives() const { return getNumPrimitivesDI<DrawElementsIndirectUInt>(*this); }
unsigned int DrawElementsIndirectUByte::getNumPrimitives() const { return getNumPrimitivesDI<DrawElementsIndirectUByte>(*this); }
unsigned int DrawElementsIndirectUShort::getNumPrimitives() const { return getNumPrimitivesDI<DrawElementsIndirectUShort>(*this); }

void DrawElementsIndirectUInt::draw(State& state, bool /*useVertexBufferObjects*/) const
{
    GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
    state.bindDrawIndirectBufferObject(dibo);

    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif

    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_INT,
        (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex()) //command array address
        +_firstCommand* _indirectCommandArray->getElementSize())// runtime offset computaion can be sizeof(*_indirectCommandArray->begin())
    );
}

DrawElementsIndirectUInt::~DrawElementsIndirectUInt()
{
    releaseGLObjects();
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

#ifndef PRIMFUNCTORBASEVERTEX
void DrawElementsIndirectUInt::accept(PrimitiveFunctor&) const {}
void DrawElementsIndirectUInt::accept(PrimitiveIndexFunctor&) const {}
#else
void DrawElementsIndirectUInt::accept(PrimitiveFunctor& functor) const
{
   //  TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
   if (!empty())
        functor.drawElements(_mode,_indirectCommandArray->count(_firstCommand),
                             &(*this)[_indirectCommandArray->firstIndex(_firstCommand)],
                             _indirectCommandArray->baseVertex(_firstCommand));
}

void DrawElementsIndirectUInt::accept(PrimitiveIndexFunctor& functor) const
{
   //  TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
   if (!empty())
        functor.drawElements(_mode,_indirectCommandArray->count(_firstCommand),
                             &(*this)[_indirectCommandArray->firstIndex(_firstCommand)],
                             _indirectCommandArray->baseVertex(_firstCommand));
}
#endif

void DrawElementsIndirectUByte::draw(State& state, bool /*useVertexBufferObjects*/) const
{
    GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
    state.bindDrawIndirectBufferObject(dibo);

    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif

    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_BYTE,
                                                      (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex())+_firstCommand* _indirectCommandArray->getElementSize()));
}

DrawElementsIndirectUByte::~DrawElementsIndirectUByte()
{
    releaseGLObjects();
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

#ifndef PRIMFUNCTORBASEVERTEX
void DrawElementsIndirectUByte::accept(PrimitiveFunctor&) const {}
void DrawElementsIndirectUByte::accept(PrimitiveIndexFunctor&) const {}
#else
void DrawElementsIndirectUByte::accept(PrimitiveFunctor& functor) const
{
   //  TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
   if (!empty())
        functor.drawElements(_mode,_indirectCommandArray->count(_firstCommand),
                             &(*this)[_indirectCommandArray->firstIndex(_firstCommand)],
                             _indirectCommandArray->baseVertex(_firstCommand));
}

void DrawElementsIndirectUByte::accept(PrimitiveIndexFunctor& functor) const
{
   //  TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
   if (!empty())
        functor.drawElements(_mode,_indirectCommandArray->count(_firstCommand),
                      &(*this)[_indirectCommandArray->firstIndex(_firstCommand)],
                      _indirectCommandArray->baseVertex(_firstCommand));
}
#endif

void DrawElementsIndirectUShort::draw(State& state, bool /*useVertexBufferObjects*/) const
{
    GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
    state.bindDrawIndirectBufferObject(dibo);

    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif

    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glDrawElementsIndirect(mode, GL_UNSIGNED_SHORT,
                                                       (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex())+_firstCommand* _indirectCommandArray->getElementSize()));
}

DrawElementsIndirectUShort::~DrawElementsIndirectUShort()
{
    releaseGLObjects();
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

#ifndef PRIMFUNCTORBASEVERTEX
void DrawElementsIndirectUShort::accept(PrimitiveFunctor&) const {}
void DrawElementsIndirectUShort::accept(PrimitiveIndexFunctor&) const {}
#else
void DrawElementsIndirectUShort::accept(PrimitiveFunctor& functor) const
{
   //  TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
   if (!empty())
        functor.drawElements(_mode,_indirectCommandArray->count(_firstCommand),
                             &(*this)[_indirectCommandArray->firstIndex(_firstCommand)],
                             _indirectCommandArray->baseVertex(_firstCommand));
}

void DrawElementsIndirectUShort::accept(PrimitiveIndexFunctor& functor) const
{
   //  TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
   if (!empty())
        functor.drawElements(_mode,_indirectCommandArray->count(_firstCommand),
                             &(*this)[_indirectCommandArray->firstIndex(_firstCommand)],
                             _indirectCommandArray->baseVertex(_firstCommand));
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawElementsIndirect
//
template<class T>   inline
unsigned int getNumPrimitivesMDI( const T&_this)
{
    IndirectCommandDrawElements *_indirectCommandArray=const_cast<IndirectCommandDrawElements *>(_this.getIndirectCommandArray());
    unsigned int total=0;
    switch(_this.getMode())
    {
        case(PrimitiveSet::POINTS):
            for(unsigned int i=0;i<_indirectCommandArray->getNumElements();++i)
                total+=_indirectCommandArray->count(i);
            break;
        case(PrimitiveSet::LINES):
            for(unsigned int i=0;i<_indirectCommandArray->getNumElements();++i)
            total+=_indirectCommandArray->count(i)/2;
            break;
        case(PrimitiveSet::TRIANGLES):
            for(unsigned int i=0;i<_indirectCommandArray->getNumElements();++i)
            total+=_indirectCommandArray->count(i)/3;
            break;
        case(PrimitiveSet::QUADS):
            for(unsigned int i=0;i<_indirectCommandArray->getNumElements();++i)
            total+=_indirectCommandArray->count(i)/4;
            break;
        case(PrimitiveSet::LINE_STRIP):
        case(PrimitiveSet::LINE_LOOP):
        case(PrimitiveSet::TRIANGLE_STRIP):
        case(PrimitiveSet::TRIANGLE_FAN):
        case(PrimitiveSet::QUAD_STRIP):
        case(PrimitiveSet::PATCHES):
        case(PrimitiveSet::POLYGON):
        {
            unsigned int primcount = _indirectCommandArray->getNumElements();
            return primcount;
        }
    }
    return total;
}

unsigned int MultiDrawElementsIndirectUInt::getNumPrimitives() const { return getNumPrimitivesMDI<MultiDrawElementsIndirectUInt>(*this); }
unsigned int MultiDrawElementsIndirectUByte::getNumPrimitives() const { return getNumPrimitivesMDI<MultiDrawElementsIndirectUByte>(*this); }
unsigned int MultiDrawElementsIndirectUShort::getNumPrimitives() const { return getNumPrimitivesMDI<MultiDrawElementsIndirectUShort>(*this); }

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawElementsIndirectUByte
//
MultiDrawElementsIndirectUByte::~MultiDrawElementsIndirectUByte()
{
    releaseGLObjects();
}

void MultiDrawElementsIndirectUByte::draw(State& state, bool /*useVertexBufferObjects*/) const
{
    GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );

    state.bindDrawIndirectBufferObject(dibo);
    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif

    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glMultiDrawElementsIndirect(mode, GL_UNSIGNED_BYTE,
                                                            (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex())),_indirectCommandArray->getNumElements(), _stride);
}

#ifndef PRIMFUNCTORBASEVERTEX
void MultiDrawElementsIndirectUByte::accept(PrimitiveFunctor&) const {}
void MultiDrawElementsIndirectUByte::accept(PrimitiveIndexFunctor&) const {}
#else
void MultiDrawElementsIndirectUByte::accept(PrimitiveFunctor& functor) const
{
  //TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
       unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
       if (!empty() )
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
                    functor.drawElements(_mode,_indirectCommandArray->count(i),
                                         &(*this)[_indirectCommandArray->firstIndex(i)],
                                         _indirectCommandArray->baseVertex(i));
}

void MultiDrawElementsIndirectUByte::accept(PrimitiveIndexFunctor& functor) const
{
  //TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
       unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
       if (!empty() )
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
                    functor.drawElements(_mode,_indirectCommandArray->count(i),
                                         &(*this)[_indirectCommandArray->firstIndex(i)],
                                         _indirectCommandArray->baseVertex(i));
}
#endif




////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawElementsIndirectUShort
//
MultiDrawElementsIndirectUShort::~MultiDrawElementsIndirectUShort()
{
    releaseGLObjects();
}

void MultiDrawElementsIndirectUShort::draw(State& state, bool /*useVertexBufferObjects*/) const
{   GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
    state.bindDrawIndirectBufferObject(dibo);

    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif

    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glMultiDrawElementsIndirect(mode, GL_UNSIGNED_SHORT, (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex())),
                                                            (_count>0) ?_count:_indirectCommandArray->getNumElements(),_stride);
}

#ifndef PRIMFUNCTORBASEVERTEX
void MultiDrawElementsIndirectUShort::accept(PrimitiveFunctor&) const {}
void MultiDrawElementsIndirectUShort::accept(PrimitiveIndexFunctor&) const {}
#else
void MultiDrawElementsIndirectUShort::accept(PrimitiveFunctor& functor) const
{
  //TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
       unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
       if (!empty() )
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
                    functor.drawElements(_mode,_indirectCommandArray->count(i),
                                         &(*this)[_indirectCommandArray->firstIndex(i)],
                                         _indirectCommandArray->baseVertex(i));
}

void MultiDrawElementsIndirectUShort::accept(PrimitiveIndexFunctor& functor) const
{
  //TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
       unsigned int maxindex = (_count>0) ?_firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
       if (!empty() )
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
                    functor.drawElements(_mode,_indirectCommandArray->count(i),
                                         &(*this)[_indirectCommandArray->firstIndex(i)],
                                         _indirectCommandArray->baseVertex(i));
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawElementsIndirectUInt
//
MultiDrawElementsIndirectUInt::~MultiDrawElementsIndirectUInt()
{
    releaseGLObjects();
}

void MultiDrawElementsIndirectUInt::draw(State& state, bool /*useVertexBufferObjects*/) const
{
    GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
    state.bindDrawIndirectBufferObject(dibo);
    GLenum mode = _mode;
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
    if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
#endif

    GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());

    state.bindElementBufferObject(ebo);

    state.get<GLExtensions>()-> glMultiDrawElementsIndirect(mode, GL_UNSIGNED_INT, (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex())),
                                                            (_count>0) ? _count:_indirectCommandArray->getNumElements(), _stride);
}

#ifndef PRIMFUNCTORBASEVERTEX
void MultiDrawElementsIndirectUInt::accept(PrimitiveFunctor&) const {}
void MultiDrawElementsIndirectUInt::accept(PrimitiveIndexFunctor&) const {}
#else
void MultiDrawElementsIndirectUInt::accept(PrimitiveFunctor& functor) const
{
  //TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
       unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
       if (!empty() )
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
                    functor.drawElements(_mode,_indirectCommandArray->count(i),
                                         &(*this)[_indirectCommandArray->firstIndex(i)],
                                         _indirectCommandArray->baseVertex(i));
}

void MultiDrawElementsIndirectUInt::accept(PrimitiveIndexFunctor& functor) const
{
  //TODO: add base vertex parameter in PrimitiveFunctor and PrimitiveIndexFunctor drawelements method
       unsigned int maxindex = (_count>0) ?_firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
       if (!empty() )
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
                    functor.drawElements(_mode,_indirectCommandArray->count(i),
                        &(*this)[_indirectCommandArray->firstIndex(i)],
                        _indirectCommandArray->baseVertex(i));
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawArrays
//
void DrawArraysIndirect::draw(osg::State& state, bool) const
{
    GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
    state.bindDrawIndirectBufferObject(dibo);

    GLExtensions* ext = state.get<GLExtensions>();

    ext->glDrawArraysIndirect(_mode,  (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex())+_firstCommand* _indirectCommandArray->getElementSize()));
}

void DrawArraysIndirect::accept(PrimitiveFunctor& functor) const
{
    functor.drawArrays(_mode, _indirectCommandArray->first(_firstCommand), _indirectCommandArray->count(_firstCommand));
}

void DrawArraysIndirect::accept(PrimitiveIndexFunctor& functor) const
{
    functor.drawArrays(_mode, _indirectCommandArray->first(_firstCommand), _indirectCommandArray->count(_firstCommand));
}

unsigned int DrawArraysIndirect::getNumIndices() const
{
    return _indirectCommandArray->count(_firstCommand);
}

unsigned int DrawArraysIndirect::index(unsigned int pos) const
{
 return _indirectCommandArray->first(_firstCommand)+ pos;
}

void DrawArraysIndirect::offsetIndices(int offset)
{
    _indirectCommandArray->first(_firstCommand)+= offset;
}

unsigned int DrawArraysIndirect::getNumPrimitives() const
{
    switch(_mode)
    {
        case(POINTS):
            return _indirectCommandArray->count(_firstCommand);
        case(LINES):
            return _indirectCommandArray->count(_firstCommand)/2;
        case(TRIANGLES):
            return _indirectCommandArray->count(_firstCommand)/3;
        case(QUADS):
            return _indirectCommandArray->count(_firstCommand)/4;
        case(LINE_STRIP):
        case(LINE_LOOP):
        case(TRIANGLE_STRIP):
        case(TRIANGLE_FAN):
        case(QUAD_STRIP):
        case(PATCHES):
        case(POLYGON):
        {
            return 1;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MultiDrawArrays
//
void MultiDrawArraysIndirect::draw(osg::State& state, bool) const
{
    GLBufferObject* dibo = _indirectCommandArray->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
    state.bindDrawIndirectBufferObject(dibo);

    GLExtensions* ext = state.get<GLExtensions>();

    ext->glMultiDrawArraysIndirect(_mode,  (const GLvoid *)(dibo->getOffset(_indirectCommandArray->getBufferIndex())+_firstCommand*_indirectCommandArray->getElementSize()),
    (_count>0) ?_count:_indirectCommandArray->getNumElements(), _stride);

}

void MultiDrawArraysIndirect::accept(PrimitiveFunctor& functor) const
{
    unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
    for(unsigned int i = _firstCommand; i<maxindex; ++i)
    {
        functor.drawArrays(_mode, _indirectCommandArray->first(i), _indirectCommandArray->count(i));
    }
}

void MultiDrawArraysIndirect::accept(PrimitiveIndexFunctor& functor) const
{
    unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
    for(unsigned int i = _firstCommand; i<maxindex; ++i)
    {
        functor.drawArrays(_mode, _indirectCommandArray->first(i), _indirectCommandArray->count(i));
    }
}

unsigned int MultiDrawArraysIndirect::getNumIndices() const
{
    unsigned int total=0;
    unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
    for(unsigned int i = _firstCommand; i<maxindex; ++i)
        total+= _indirectCommandArray->count(i);

    return total;
}

unsigned int MultiDrawArraysIndirect::index(unsigned int pos) const
{
    unsigned int i = 0, maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
    for(i=_firstCommand; i<maxindex;++i)
      {
        unsigned int count = _indirectCommandArray->count(i);
        if (pos<count) break;
        pos -= count;
    }
    if (i>=maxindex) return 0;
    return _indirectCommandArray->first(maxindex-1) + pos;

}

void MultiDrawArraysIndirect::offsetIndices(int offset)
{
    unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;
    for(unsigned int i = _firstCommand; i<maxindex; ++i)
        _indirectCommandArray->first(i) += offset;
}

unsigned int MultiDrawArraysIndirect::getNumPrimitives() const
{
    unsigned int total=0;unsigned int maxindex= (_count>0) ? _firstCommand + _count : _indirectCommandArray->getNumElements() - _firstCommand;

    switch(_mode)
    {
        case(POINTS):
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
                total+=_indirectCommandArray->count(i);
            break;
        case(LINES):
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
            total+=_indirectCommandArray->count(i)/2;
            break;
        case(TRIANGLES):
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
            total+=_indirectCommandArray->count(i)/3;
            break;
        case(QUADS):
            for(unsigned int i = _firstCommand; i<maxindex; ++i)
            total+=_indirectCommandArray->count(i)/4;
            break;
        case(LINE_STRIP):
        case(LINE_LOOP):
        case(TRIANGLE_STRIP):
        case(TRIANGLE_FAN):
        case(QUAD_STRIP):
        case(PATCHES):
        case(POLYGON):
        {
            unsigned int primcount = _indirectCommandArray->getNumElements();
            return primcount;
        }
    }
    return total;
}
