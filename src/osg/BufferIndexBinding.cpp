/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2010 Tim Moore
 * Copyright (C) 2012 David Callu
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

#include <osg/BufferIndexBinding>
#include <osg/State>

#include <string.h> // for memcpy

#ifndef GL_DRAW_INDIRECT_BUFFER
    #define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#endif

namespace osg {

BufferBinding::BufferBinding(GLenum target)
    : _target(target), _bufferObject(0)
{
}

BufferBinding::BufferBinding(GLenum target, BufferObject* bo)
    : _target(target), _bufferObject(bo)
{
}

BufferBinding::BufferBinding(const BufferBinding& rhs, const CopyOp& copyop)
    : StateAttribute(rhs, copyop),
      _target(rhs._target),
      _bufferObject(static_cast<BufferObject*>(copyop(rhs._bufferObject.get())))
{
}

BufferBinding::~BufferBinding()
{
}

void BufferBinding::apply(State& state) const
{
    if (_bufferObject.valid())
    {
        GLBufferObject* glObject
            = _bufferObject->getOrCreateGLBufferObject(state.getContextID());
        if (!glObject->_extensions->isUniformBufferObjectSupported)
            return;
        if (glObject->isDirty()) glObject->compileBuffer();
        glObject->_extensions->glBindBuffer (_target, glObject->getGLObjectID());
    }
}

BufferIndexBinding::BufferIndexBinding(GLenum target, GLuint index)
    :BufferBinding(target), _index(index), _offset(0), _size(0)
{
}

BufferIndexBinding::BufferIndexBinding(GLenum target, GLuint index, BufferObject* bo,
                                       GLintptr offset, GLsizeiptr size)
    : BufferBinding(target,bo), _index(index), _offset(offset), _size(size)
{
}

BufferIndexBinding::BufferIndexBinding(const BufferIndexBinding& rhs, const CopyOp& copyop)
    : BufferBinding(rhs, copyop),
      _index(rhs._index),
      _offset(rhs._offset),
      _size(rhs._size)
{
}

BufferIndexBinding::~BufferIndexBinding()
{
}

void BufferIndexBinding::setIndex(GLuint index){

    StateSet**parents=new StateSet* [getNumParents()];
    StateAttribute::GLModeValue *parentmodes =new StateAttribute::GLModeValue  [getNumParents()];
    unsigned int numparent=getNumParents();
    for(unsigned int i=0;i<numparent;i++)
    {
        parents[i]=getParent(i);
        parentmodes[i]=parents[i]->getAttributeList().find(getTypeMemberPair())->second.second;
    }

    ref_ptr<StateAttribute> dummy=this;

    for(unsigned int i=0;i<numparent;i++)parents[i]->removeAttribute(this);

    _index=index;

    for(unsigned int i=0;i<numparent;i++)
        parents[i]->setAttributeAndModes(this,  parentmodes[i]);

    delete [] parents;
    delete [] parentmodes;

}
void BufferIndexBinding::apply(State& state) const
{
    if (_bufferObject.valid())
    {
        GLBufferObject* glObject
            = _bufferObject->getOrCreateGLBufferObject(state.getContextID());
        if (!glObject->_extensions->isUniformBufferObjectSupported)
            return;
        if (glObject->isDirty()) glObject->compileBuffer();
        glObject->_extensions->glBindBufferRange(_target, _index,
                                                 glObject->getGLObjectID(), _offset, _size);
    }
}

UniformBufferBinding::UniformBufferBinding()
    : BufferIndexBinding(GL_UNIFORM_BUFFER, 0)
{
}

UniformBufferBinding::UniformBufferBinding(GLuint index)
  : BufferIndexBinding(GL_UNIFORM_BUFFER, index)
{
}

UniformBufferBinding::UniformBufferBinding(GLuint index, BufferObject* bo, GLintptr offset,
                                           GLsizeiptr size)
    : BufferIndexBinding(GL_UNIFORM_BUFFER, index, bo, offset, size)
{

}

UniformBufferBinding::UniformBufferBinding(const UniformBufferBinding& rhs,
                                           const CopyOp& copyop)
    : BufferIndexBinding(rhs, copyop)
{
}


TransformFeedbackBufferBinding::TransformFeedbackBufferBinding(GLuint index)
  : BufferIndexBinding(GL_TRANSFORM_FEEDBACK_BUFFER, index)
{
}

TransformFeedbackBufferBinding::TransformFeedbackBufferBinding(GLuint index, BufferObject* bo, GLintptr offset, GLsizeiptr size)
    : BufferIndexBinding(GL_TRANSFORM_FEEDBACK_BUFFER, index, bo, offset, size)
{

}

TransformFeedbackBufferBinding::TransformFeedbackBufferBinding(const TransformFeedbackBufferBinding& rhs, const CopyOp& copyop)
    : BufferIndexBinding(rhs, copyop)
{
}


AtomicCounterBufferBinding::AtomicCounterBufferBinding(GLuint index)
  : BufferIndexBinding(GL_ATOMIC_COUNTER_BUFFER, index)
{
}

AtomicCounterBufferBinding::AtomicCounterBufferBinding(GLuint index, BufferObject* bo, GLintptr offset, GLsizeiptr size)
    : BufferIndexBinding(GL_ATOMIC_COUNTER_BUFFER, index, bo, offset, size)
{

}

AtomicCounterBufferBinding::AtomicCounterBufferBinding(const AtomicCounterBufferBinding& rhs, const CopyOp& copyop)
    : BufferIndexBinding(rhs, copyop)
{
}

void AtomicCounterBufferBinding::readData(osg::State & state, osg::UIntArray & uintArray) const
{
    if (!_bufferObject) return;

    GLBufferObject* bo = _bufferObject->getOrCreateGLBufferObject( state.getContextID() );
    if (!bo) return;


    GLint previousID = 0;
    glGetIntegerv(GL_ATOMIC_COUNTER_BUFFER_BINDING, &previousID);

    if (static_cast<GLuint>(previousID) != bo->getGLObjectID())
        bo->_extensions->glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, bo->getGLObjectID());

    GLubyte* src = (GLubyte*)bo->_extensions->glMapBuffer(GL_ATOMIC_COUNTER_BUFFER,
                                                          GL_READ_ONLY_ARB);
    if(src)
    {
        size_t size = osg::minimum<int>(_size, uintArray.getTotalDataSize());
        memcpy((void*) &(uintArray.front()), src+_offset, size);
        bo->_extensions->glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    }

    if (static_cast<GLuint>(previousID) != bo->getGLObjectID())
        bo->_extensions->glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, static_cast<GLuint>(previousID));
}


ShaderStorageBufferBinding::ShaderStorageBufferBinding(GLuint index)
    : BufferIndexBinding(GL_SHADER_STORAGE_BUFFER, index)
{
}

ShaderStorageBufferBinding::ShaderStorageBufferBinding(GLuint index, BufferObject* bo, GLintptr offset, GLsizeiptr size)
    : BufferIndexBinding(GL_SHADER_STORAGE_BUFFER, index, bo, offset, size)
{

}

ShaderStorageBufferBinding::ShaderStorageBufferBinding(const ShaderStorageBufferBinding& rhs, const CopyOp& copyop)
    : BufferIndexBinding(rhs, copyop)
{
}


DrawIndirectBufferBinding::DrawIndirectBufferBinding( )
  : BufferBinding(GL_DRAW_INDIRECT_BUFFER)
{
}
void DrawIndirectBufferBinding::apply(State& state) const
{
    if (_bufferObject.valid())
    {
        GLBufferObject* glObject
            = _bufferObject->getOrCreateGLBufferObject(state.getContextID());
        if (!glObject->_extensions->isUniformBufferObjectSupported)
            return;
        //  if (glObject->isDirty()) glObject->compileBuffer(); assuming buffer is GPU generated
        glObject->_extensions->glBindBuffer (_target, glObject->getGLObjectID());
    }
}
DrawIndirectBufferBinding::DrawIndirectBufferBinding(  BufferObject* bo)
    : BufferBinding(GL_DRAW_INDIRECT_BUFFER, bo)
{

}

DrawIndirectBufferBinding::DrawIndirectBufferBinding(const DrawIndirectBufferBinding& rhs,
                                           const CopyOp& copyop)
    : BufferBinding(rhs, copyop)
{
}


} // namespace osg
