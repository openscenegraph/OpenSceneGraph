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
#include <osg/PrimitiveSet>
#include <osg/BufferObject>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

unsigned int PrimitiveSet::getNumPrimitives() const
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
        case(POLYGON): return 1;
    }
    return 0;
}

void DrawArrays::draw(State& state, bool) const
{
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
    if (_numInstances>=1) state.glDrawArraysInstanced(_mode,_first,_count, _numInstances);
    else glDrawArrays(_mode,_first,_count);
#endif
}

void DrawArrays::accept(PrimitiveFunctor& functor) const
{
    functor.drawArrays(_mode,_first,_count);
}

void DrawArrays::accept(PrimitiveIndexFunctor& functor) const
{
    functor.drawArrays(_mode,_first,_count);
}

unsigned int DrawArrayLengths::getNumPrimitives() const
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
        case(POLYGON): return size();
    }
    return 0;
}

void DrawArrayLengths::draw(State& state, bool) const
{
    GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (_mode==GL_QUADS)
        {
            GLint first = _first;
            for(vector_type::const_iterator itr=begin();
                itr!=end();
                ++itr)
            {
                state.drawQuads(first, *itr, _numInstances);
                first += *itr;
            }

            return;
        }
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    GLint first = _first;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        glDrawArrays(mode,first,*itr);
        first += *itr;
    }

}

void DrawArrayLengths::accept(PrimitiveFunctor& functor) const
{
    GLint first = _first;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        functor.drawArrays(_mode,first,*itr);
        first += *itr;
    }
}

void DrawArrayLengths::accept(PrimitiveIndexFunctor& functor) const
{
    GLint first = _first;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        functor.drawArrays(_mode,first,*itr);
        first += *itr;
    }
}

unsigned int DrawArrayLengths::getNumIndices() const
{
    unsigned int count = 0;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        count += *itr;
    }
    return count;
}

DrawElementsUByte::~DrawElementsUByte()
{
    releaseGLObjects();
}

void DrawElementsUByte::draw(State& state, bool useVertexBufferObjects) const
{
    GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    if (useVertexBufferObjects)
    {
        GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());
        state.bindElementBufferObject(ebo);
        if (ebo)
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_BYTE, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
            else glDrawElements(mode, size(), GL_UNSIGNED_BYTE, (const GLvoid *)(ebo->getOffset(getBufferIndex())));
        }
        else
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_BYTE, &front(), _numInstances);
            else glDrawElements(mode, size(), GL_UNSIGNED_BYTE, &front());
        }
    }
    else
    {
        if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_BYTE, &front(), _numInstances);
        else glDrawElements(mode, size(), GL_UNSIGNED_BYTE, &front());
    }
}

void DrawElementsUByte::accept(PrimitiveFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUByte::accept(PrimitiveIndexFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUByte::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}


DrawElementsUShort::~DrawElementsUShort()
{
    releaseGLObjects();
}

void DrawElementsUShort::draw(State& state, bool useVertexBufferObjects) const
{
    GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    if (useVertexBufferObjects)
    {
        GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());
        state.bindElementBufferObject(ebo);
        if (ebo)
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_SHORT, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
            else glDrawElements(mode, size(), GL_UNSIGNED_SHORT, (const GLvoid *)(ebo->getOffset(getBufferIndex())));
        }
        else
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_SHORT, &front(), _numInstances);
            else glDrawElements(mode, size(), GL_UNSIGNED_SHORT, &front());
        }
    }
    else
    {
        if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_SHORT, &front(), _numInstances);
        else glDrawElements(mode, size(), GL_UNSIGNED_SHORT, &front());
    }
}

void DrawElementsUShort::accept(PrimitiveFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUShort::accept(PrimitiveIndexFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUShort::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}


DrawElementsUInt::~DrawElementsUInt()
{
    releaseGLObjects();
}

void DrawElementsUInt::draw(State& state, bool useVertexBufferObjects) const
{
    GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    if (useVertexBufferObjects)
    {
        GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());
        state.bindElementBufferObject(ebo);
        if (ebo)
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
            else glDrawElements(mode, size(), GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex())));
        }
        else
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_INT, &front(), _numInstances);
            else glDrawElements(mode, size(), GL_UNSIGNED_INT, &front());
        }
    }
    else
    {
        if (_numInstances>=1) state.glDrawElementsInstanced(mode, size(), GL_UNSIGNED_INT, &front(), _numInstances);
        else glDrawElements(mode, size(), GL_UNSIGNED_INT, &front());
    }
}

void DrawElementsUInt::accept(PrimitiveFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUInt::accept(PrimitiveIndexFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUInt::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}
