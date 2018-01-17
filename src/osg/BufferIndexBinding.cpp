/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2010 Tim Moore
 * Copyright (C) 2012 David Callu
 * Copyright (C) 2017 Julien Valentin
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
#include <osg/RenderInfo>

#include <string.h> // for memcpy

#ifndef GL_DRAW_INDIRECT_BUFFER
    #define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#endif

namespace osg {


BufferIndexBinding::BufferIndexBinding(GLenum target, GLuint index)
    :_target(target), _bufferData(0), _index(index), _offset(0), _size(0)
{
}

BufferIndexBinding::BufferIndexBinding(GLenum target, GLuint index, BufferData* bo,
                                       GLintptr offset, GLsizeiptr size)
    : _target(target), _index(index), _offset(offset), _size(size)
{
    setBufferData(bo);
}

BufferIndexBinding::BufferIndexBinding(const BufferIndexBinding& rhs, const CopyOp& copyop):StateAttribute(rhs,copyop),
    _target(rhs._target),
    _bufferData(static_cast<BufferData*>(copyop(rhs._bufferData.get()))),
    _index(rhs._index),
    _offset(rhs._offset),
    _size(rhs._size)
{
}

BufferIndexBinding::~BufferIndexBinding()
{
}

void BufferIndexBinding::setIndex(unsigned int index)
{
    if (_index==index) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _index = index;
}

void BufferIndexBinding::apply(State& state) const
{
    if (_bufferData.valid())
    {
        GLBufferObject* glObject
            = _bufferData->getBufferObject()->getOrCreateGLBufferObject(state.getContextID());
        if (glObject->isDirty()) glObject->compileBuffer();
        glObject->_extensions->glBindBufferRange(_target, _index,
                glObject->getGLObjectID(), glObject->getOffset(_bufferData->getBufferIndex())+_offset, _size-_offset);
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

UniformBufferBinding::UniformBufferBinding(GLuint index, BufferData* bo, GLintptr offset,
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

TransformFeedbackBufferBinding::TransformFeedbackBufferBinding(GLuint index, BufferData* bo, GLintptr offset, GLsizeiptr size)
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

AtomicCounterBufferBinding::AtomicCounterBufferBinding(GLuint index, BufferData* bo, GLintptr offset, GLsizeiptr size)
    : BufferIndexBinding(GL_ATOMIC_COUNTER_BUFFER, index, bo, offset, size)
{

}

AtomicCounterBufferBinding::AtomicCounterBufferBinding(const AtomicCounterBufferBinding& rhs, const CopyOp& copyop)
    : BufferIndexBinding(rhs, copyop)
{
}

void AtomicCounterBufferBinding::readData(osg::State & state, osg::UIntArray & uintArray) const
{
    if (!_bufferData) return;

    GLBufferObject* bo = _bufferData->getBufferObject()->getOrCreateGLBufferObject( state.getContextID() );
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

ShaderStorageBufferBinding::ShaderStorageBufferBinding(GLuint index, BufferData* bo, GLintptr offset, GLsizeiptr size)
    : BufferIndexBinding(GL_SHADER_STORAGE_BUFFER, index, bo, offset, size)
{

}

ShaderStorageBufferBinding::ShaderStorageBufferBinding(const ShaderStorageBufferBinding& rhs, const CopyOp& copyop)
    : BufferIndexBinding(rhs, copyop)
{
}


/// Delegate Camera Post Draw Callback to SyncBufferDataCallback
class BufferDataReadBack : public Camera::DrawCallback{
public:
    BufferDataReadBack(SyncBufferDataCallback* u): Camera::DrawCallback(), _upcb(u){}
    virtual void operator () (osg::RenderInfo& renderInfo) const{ _upcb->updateFlags(renderInfo); }

protected:
    SyncBufferDataCallback* _upcb;
};

//#define BUFER_RANGE_ATOMIC_COUNTER_BUFFER_AVAILABLE
//seams to require more than GL4.2?!
//perhaps a bug in Linux 4.5.0 NVIDIA 381.22

bool SyncBufferDataCallback::readBackBufferData (RenderInfo& renderInfo) const
{
    GLenum target = _bd->getBufferObject()->getTarget();
    GLubyte* src;

    GLBufferObject* glObject = _bd->getBufferObject()->getOrCreateGLBufferObject(renderInfo.getContextID());
    if (glObject->isDirty())
    {
        OSG_DEBUG << "osg::SyncBufferDataCallback::readBackBufferData it seams we wanna read gpu data before its first upload" << std::endl;
        return false;
    }

    glObject->_extensions->glBindBuffer(target, glObject->getGLObjectID());

#ifndef BUFER_RANGE_ATOMIC_COUNTER_BUFFER_AVAILABLE
    if(target == GL_ATOMIC_COUNTER_BUFFER )
    {
        src= (GLubyte*) glObject->_extensions->glMapBuffer( target, GL_READ_ONLY_ARB);
        if(src) memcpy(const_cast<GLvoid*>(_bd->getDataPointer()), src + glObject->getOffset(_bd->getBufferIndex()), _bd->getTotalDataSize());
    }
    else
#endif
    {
        src= (GLubyte*) glObject->_extensions->glMapBufferRange(
                 target,
                 glObject->getOffset(_bd->getBufferIndex()),
                 _bd->getTotalDataSize(),
                 GL_READ_ONLY_ARB
             );
        if(src) memcpy(const_cast<GLvoid*>(_bd->getDataPointer()), src, _bd->getTotalDataSize());
    }
    glObject->_extensions->glUnmapBuffer(target);
    return true;
}

struct FindNearestCamera : public NodeVisitor
{
    ref_ptr<Camera> _root;
    FindNearestCamera() : NodeVisitor(NodeVisitor::TRAVERSE_PARENTS) {}
    void apply(Transform& node)
    {
        if (_root.valid())
            return;
        _root = dynamic_cast<Camera*>(&node);
        traverse(node);
    }
};

SyncBufferDataCallback::~SyncBufferDataCallback()
{

    if(_cam.valid())
        _cam->removePostDrawCallback(_dcb);
}

void SyncBufferDataCallback::operator()(Node* node, NodeVisitor* nv)
{

    if(!_cam.valid() && _GpuAccess & GL_READ_WRITE)
    {
        FindNearestCamera fnc;
        node->accept(fnc);
        if(!fnc._root.valid())
        {
            OSG_WARN << "osg::SyncBufferDataCallback::operator() no camera found!?" << std::endl;
            return;
        }
        _cam = fnc._root;
        _dcb = new BufferDataReadBack(this);
        _cam->addPostDrawCallback(_dcb );
    }

    _lock.lock();

    if(_GpuAccess & GL_READ_WRITE)
    {
        if(gpuproduced && (cpuproduced = synctraversal( node,  nv)) )
            gpuproduced = false;
    }
    else cpuproduced = synctraversal( node,  nv);

    _lock.unlock();
}

///called by the Camera read back callback
void SyncBufferDataCallback::updateFlags( RenderInfo&ri)
{
    _lock.lock();

    if(_CpuAccess & GL_READ_WRITE)
    {
        if(cpuproduced)
        {
            if(!gpuproduced && (gpuproduced = readBackBufferData(ri)) )
                cpuproduced = false;
        }
    }
    else if(!gpuproduced)
        gpuproduced = readBackBufferData(ri);

    _lock.unlock();
}


} // namespace osg
