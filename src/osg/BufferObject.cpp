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
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/BufferObject>
#include <osg/Notify>
#include <osg/GLExtensions>
#include <osg/Timer>
#include <osg/Image>
#include <osg/State>
#include <osg/PrimitiveSet>
#include <osg/Array>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

using namespace osg;

// static cache of deleted buffer object lists which can only
// by completely deleted once the appropriate OpenGL context
// is set.  Used osg::BufferObject::deleteBufferObject(..) and flushDeletedBufferObjects(..) below.
typedef std::multimap<unsigned int,GLuint> BufferObjectMap;
typedef osg::buffered_object<BufferObjectMap> DeletedBufferObjectCache;

static OpenThreads::Mutex s_mutex_deletedBufferObjectCache;
static DeletedBufferObjectCache s_deletedBufferObjectCache;

void BufferObject::deleteBufferObject(unsigned int contextID,GLuint globj)
{
    if (globj!=0)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedBufferObjectCache);

        // insert the globj into the cache for the appropriate context.
        s_deletedBufferObjectCache[contextID].insert(BufferObjectMap::value_type(0,globj));
    }
}

void BufferObject::flushDeletedBufferObjects(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;


    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedBufferObjectCache);

        const Extensions* extensions = getExtensions(contextID,true);

        unsigned int noDeleted = 0;

        BufferObjectMap& dll = s_deletedBufferObjectCache[contextID];

        BufferObjectMap::iterator ditr=dll.begin();
        for(;
            ditr!=dll.end() && elapsedTime<availableTime;
            ++ditr)
        {
            extensions->glDeleteBuffers(1,&(ditr->second));
            elapsedTime = timer.delta_s(start_tick,timer.tick());
            ++noDeleted;
        }
        if (ditr!=dll.begin()) dll.erase(dll.begin(),ditr);

        // if (noDeleted!=0) notify(osg::NOTICE)<<"Number VBOs deleted = "<<noDeleted<<" BO's left"<<dll.size()<<std::endl;
    }

    availableTime -= elapsedTime;
}

void BufferObject::discardDeletedBufferObjects(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedBufferObjectCache);
    BufferObjectMap& dll = s_deletedBufferObjectCache[contextID];
    dll.clear();
}


BufferObject::BufferObject():
    _target(0),
    _usage(0),
    _totalSize(0)
{
}

BufferObject::BufferObject(const BufferObject& bo,const CopyOp& copyop):
    Object(bo,copyop)
{
}

BufferObject::~BufferObject()
{
    releaseGLObjects(0);
}

void BufferObject::resizeGLObjectBuffers(unsigned int maxSize)
{
    _bufferObjectList.resize(maxSize);
}

void BufferObject::releaseGLObjects(State* state) const
{
    if (state)
    {
        unsigned int contextID = state->getContextID();
        if (_bufferObjectList[contextID])
        {
             deleteBufferObject(contextID,_bufferObjectList[contextID]);
            _bufferObjectList[contextID] = 0;
        }
    }
    else
    {
        for(unsigned int contextID=0;contextID<_bufferObjectList.size();++contextID)
        {
            if (_bufferObjectList[contextID])
            {
                 deleteBufferObject(contextID,_bufferObjectList[contextID]);
                _bufferObjectList[contextID] = 0;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
//  Extension support
//

typedef buffered_value< ref_ptr<BufferObject::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

BufferObject::Extensions* BufferObject::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new BufferObject::Extensions(contextID);
    return s_extensions[contextID].get();
}

void BufferObject::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

BufferObject::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

BufferObject::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _glGenBuffers = rhs._glGenBuffers;
    _glBindBuffer = rhs._glBindBuffer;
    _glBufferData = rhs._glBufferData;
    _glBufferSubData = rhs._glBufferSubData;
    _glDeleteBuffers = rhs._glDeleteBuffers;
    _glIsBuffer = rhs._glIsBuffer;
    _glGetBufferSubData = rhs._glGetBufferSubData;
    _glMapBuffer = rhs._glMapBuffer;
    _glUnmapBuffer = rhs._glUnmapBuffer;
    _glGetBufferParameteriv = rhs._glGetBufferParameteriv;
    _glGetBufferPointerv = rhs._glGetBufferPointerv;
}


void BufferObject::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._glGenBuffers) _glGenBuffers = rhs._glGenBuffers;
    if (!rhs._glBindBuffer) _glBindBuffer = rhs._glBindBuffer;
    if (!rhs._glBufferData) _glBufferData = rhs._glBufferData;
    if (!rhs._glBufferSubData) _glBufferSubData = rhs._glBufferSubData;
    if (!rhs._glDeleteBuffers) _glDeleteBuffers = rhs._glDeleteBuffers;
    if (!rhs._glIsBuffer) _glIsBuffer = rhs._glIsBuffer;
    if (!rhs._glGetBufferSubData) _glGetBufferSubData = rhs._glGetBufferSubData;
    if (!rhs._glMapBuffer) _glMapBuffer = rhs._glMapBuffer;
    if (!rhs._glUnmapBuffer) _glUnmapBuffer = rhs._glUnmapBuffer;
    if (!rhs._glGetBufferParameteriv) _glGetBufferParameteriv = rhs._glGetBufferParameteriv;
    if (!rhs._glGetBufferParameteriv) _glGetBufferPointerv = rhs._glGetBufferPointerv;
}

void BufferObject::Extensions::setupGLExtensions(unsigned int contextID)
{
    setGLExtensionFuncPtr(_glGenBuffers, "glGenBuffers","glGenBuffersARB");
    setGLExtensionFuncPtr(_glBindBuffer, "glBindBuffer","glBindBufferARB");
    setGLExtensionFuncPtr(_glBufferData, "glBufferData","glBufferDataARB");
    setGLExtensionFuncPtr(_glBufferSubData, "glBufferSubData","glBufferSubDataARB");
    setGLExtensionFuncPtr(_glDeleteBuffers, "glDeleteBuffers","glDeleteBuffersARB");
    setGLExtensionFuncPtr(_glIsBuffer, "glIsBuffer","glIsBufferARB");
    setGLExtensionFuncPtr(_glGetBufferSubData, "glGetBufferSubData","glGetBufferSubDataARB");
    setGLExtensionFuncPtr(_glMapBuffer, "glMapBuffer","glMapBufferARB");
    setGLExtensionFuncPtr(_glUnmapBuffer, "glUnmapBuffer","glUnmapBufferARB");
    setGLExtensionFuncPtr(_glGetBufferParameteriv, "glGetBufferParameteriv","glGetBufferParameterivARB");
    setGLExtensionFuncPtr(_glGetBufferPointerv, "glGetBufferPointerv","glGetBufferPointervARB");
    _isPBOSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_pixel_buffer_object");
}

void BufferObject::Extensions::glGenBuffers(GLsizei n, GLuint *buffers) const
{
    if (_glGenBuffers) _glGenBuffers(n, buffers);
    else notify(WARN)<<"Error: glGenBuffers not supported by OpenGL driver"<<std::endl;
}

void BufferObject::Extensions::glBindBuffer(GLenum target, GLuint buffer) const
{
    if (_glBindBuffer) _glBindBuffer(target, buffer);
    else notify(WARN)<<"Error: glBindBuffer not supported by OpenGL driver"<<std::endl;
}

void BufferObject::Extensions::glBufferData(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage) const
{
    if (_glBufferData) _glBufferData(target, size, data, usage);
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void BufferObject::Extensions::glBufferSubData(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data) const
{
    if (_glBufferSubData) _glBufferSubData(target, offset, size, data);
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void BufferObject::Extensions::glDeleteBuffers(GLsizei n, const GLuint *buffers) const
{
    if (_glDeleteBuffers) _glDeleteBuffers(n, buffers);
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

GLboolean BufferObject::Extensions::glIsBuffer (GLuint buffer) const
{
    if (_glIsBuffer) return _glIsBuffer(buffer);
    else
    {
        notify(WARN)<<"Error: glIsBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void BufferObject::Extensions::glGetBufferSubData (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data) const
{
    if (_glGetBufferSubData) _glGetBufferSubData(target,offset,size,data);
    else notify(WARN)<<"Error: glGetBufferSubData not supported by OpenGL driver"<<std::endl;
}

GLvoid* BufferObject::Extensions::glMapBuffer (GLenum target, GLenum access) const
{
    if (_glMapBuffer) return _glMapBuffer(target,access);
    else
    {
        notify(WARN)<<"Error: glMapBuffer not supported by OpenGL driver"<<std::endl;
        return 0;
    }
}

GLboolean BufferObject::Extensions::glUnmapBuffer (GLenum target) const
{
    if (_glUnmapBuffer) return _glUnmapBuffer(target);
    else
    {
        notify(WARN)<<"Error: glUnmapBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void BufferObject::Extensions::glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) const
{
    if (_glGetBufferParameteriv) _glGetBufferParameteriv(target,pname,params);
    else notify(WARN)<<"Error: glGetBufferParameteriv not supported by OpenGL driver"<<std::endl;
}

void BufferObject::Extensions::glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params) const
{
    if (_glGetBufferPointerv) _glGetBufferPointerv(target,pname,params);
    else notify(WARN)<<"Error: glGetBufferPointerv not supported by OpenGL driver"<<std::endl;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  VertexBufferObject
//
VertexBufferObject::VertexBufferObject()
{
    _target = GL_ARRAY_BUFFER_ARB;
    _usage = GL_STATIC_DRAW_ARB;
//    _usage = GL_DYNAMIC_DRAW_ARB;
//    _usage = GL_STREAM_DRAW_ARB;
}

VertexBufferObject::VertexBufferObject(const VertexBufferObject& vbo,const CopyOp& copyop):
    BufferObject(vbo,copyop)
{
}

VertexBufferObject::~VertexBufferObject()
{
}

unsigned int VertexBufferObject::addArray(osg::Array* array)
{
    unsigned int i = _bufferEntryArrayPairs.size();

    _bufferEntryArrayPairs.resize(i+1);
    _bufferEntryArrayPairs[i].second = array;
    _bufferEntryArrayPairs[i].first.modifiedCount.setAllElementsTo(0xffffffff);
    _bufferEntryArrayPairs[i].first.offset = 0;

    dirty();

    return i;
}

void VertexBufferObject::removeArray(osg::Array* array)
{
    BufferEntryArrayPairs::iterator itr;
    for(itr = _bufferEntryArrayPairs.begin();
        itr != _bufferEntryArrayPairs.end();
        ++itr)
    {
        if (itr->second == array) break;
    }
    if (itr != _bufferEntryArrayPairs.end()) _bufferEntryArrayPairs.erase(itr);
}

void VertexBufferObject::setArray(unsigned int i, Array* array)
{
    if (i+1>=_bufferEntryArrayPairs.size()) _bufferEntryArrayPairs.resize(i+1);

    _bufferEntryArrayPairs[i].second = array;
    _bufferEntryArrayPairs[i].first.modifiedCount.setAllElementsTo(0xffffffff);
    _bufferEntryArrayPairs[i].first.offset = 0;

    dirty();
}
void VertexBufferObject::compileBuffer(State& state) const
{
    unsigned int contextID = state.getContextID();

    _compiledList[contextID] = 1;

    Extensions* extensions = getExtensions(contextID,true);

    // osg::notify(osg::NOTICE)<<"VertexBufferObject::compileBuffer frameNumber="<<state.getFrameStamp()->getFrameNumber()<<std::endl;

    unsigned int totalSizeRequired = 0;
    unsigned int numNewArrays = 0;
    for(BufferEntryArrayPairs::const_iterator itr = _bufferEntryArrayPairs.begin();
        itr != _bufferEntryArrayPairs.end();
        ++itr)
    {
        const BufferEntryArrayPair& bep = *itr;
        if (bep.second)
        {
            totalSizeRequired += bep.second->getTotalDataSize();
            if (bep.first.dataSize == 0) ++numNewArrays;
        }
    }

    bool copyAll = false;
    GLuint& vbo = buffer(contextID);
    if (vbo==0)
    {
        // building for the first time.

        _totalSize = totalSizeRequired;

        // don't generate buffer if size is zero.
        if (_totalSize==0) return;

        extensions->glGenBuffers(1, &vbo);
        extensions->glBindBuffer(_target, vbo);
        extensions->glBufferData(_target, _totalSize, NULL, _usage);

        copyAll = true;
    }
    else
    {
        extensions->glBindBuffer(_target, vbo);

        if (_totalSize != totalSizeRequired)
        {
            // resize vbo.
            _totalSize = totalSizeRequired;
            extensions->glBufferData(_target, _totalSize, NULL, _usage);

            copyAll = true;
        }
    }

    typedef std::map<unsigned int,std::vector<unsigned int> > SizePosMap_t;
    SizePosMap_t freeList;
    if (copyAll == false && numNewArrays > 0)
    {
        std::map<unsigned int,unsigned int> usedList;
        for(BufferEntryArrayPairs::const_iterator itr = _bufferEntryArrayPairs.begin();
            itr != _bufferEntryArrayPairs.end();
            ++itr)
        {
            const BufferEntryArrayPair& bep = *itr;
            if (bep.second==NULL) continue;
            if (bep.first.dataSize == 0) continue;
            usedList[bep.first.offset] = bep.first.dataSize;
        }
        unsigned int numFreeBlocks = 0;
        unsigned int pos=0;

        for (std::map<unsigned int,unsigned int>::const_iterator it=usedList.begin(); it!=usedList.end(); ++it)
        {
            unsigned int start = it->first;
            unsigned int length = it->second;
            if (pos < start)
            {
                freeList[start-pos].push_back(pos);
                ++numFreeBlocks;
            }
            pos = start+length;
        }
        if (pos < totalSizeRequired)
        {
            freeList[totalSizeRequired-pos].push_back(pos);
            ++numFreeBlocks;
        }
        if (numNewArrays < numFreeBlocks)
        {
            copyAll = true;     // too fragmented, fallback to copyAll
            freeList.clear();
        }
    }


//    osg::Timer_t start_tick = osg::Timer::instance()->tick();


    void* vboMemory = 0;

#if 0
    vboMemory = extensions->glMapBuffer(_target, GL_WRITE_ONLY_ARB);
#endif

    unsigned int offset = 0;
    for(BufferEntryArrayPairs::const_iterator itr = _bufferEntryArrayPairs.begin();
        itr != _bufferEntryArrayPairs.end();
        ++itr)
    {
        const BufferEntryArrayPair& bep = *itr;
        const Array* de = bep.second;
        if (de)
        {
            const unsigned int arraySize = de->getTotalDataSize();
            if (copyAll ||
                bep.first.modifiedCount[contextID] != bep.second->getModifiedCount() ||
                bep.first.dataSize != arraySize)
            {
                // copy data across
                unsigned int newOffset = bep.first.offset;              
                if (copyAll)
                {
                    newOffset = offset;
                    offset += arraySize;
                }
                else if (bep.first.dataSize == 0)
                {
                    SizePosMap_t::iterator findIt = freeList.lower_bound(arraySize);
                    if (findIt==freeList.end())
                    {
                        osg::notify(osg::FATAL)<<"No suitable Memory in VBO found!"<<std::endl;
                        continue;
                    }
                    const unsigned int oldOffset = findIt->second.back();
                    newOffset = oldOffset;
                    if (findIt->first > arraySize) // using larger block
                    {
                        freeList[findIt->first-arraySize].push_back(oldOffset+arraySize);
                    }
                    findIt->second.pop_back();
                    if (findIt->second.empty())
                    {
                        freeList.erase(findIt);
                    }
                }
                bep.first.dataSize = arraySize;
                bep.first.modifiedCount[contextID] = de->getModifiedCount();
                bep.first.offset = newOffset;
                de->setVertexBufferObjectOffset((GLvoid*)newOffset);

                // osg::notify(osg::NOTICE)<<"   copying vertex buffer data "<<bep.first.dataSize<<" bytes"<<std::endl;

                if (vboMemory)
                    memcpy((char*)vboMemory + bep.first.offset, de->getDataPointer(), bep.first.dataSize);
                else
                    extensions->glBufferSubData(_target, bep.first.offset, bep.first.dataSize, de->getDataPointer());

            }
        }
    }


    // Unmap the texture image buffer
    if (vboMemory) extensions->glUnmapBuffer(_target);

//    osg::notify(osg::NOTICE)<<"pbo _totalSize="<<_totalSize<<std::endl;
//    osg::notify(osg::NOTICE)<<"pbo "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
}

void VertexBufferObject::resizeGLObjectBuffers(unsigned int maxSize)
{
    BufferObject::resizeGLObjectBuffers(maxSize);

    for(BufferEntryArrayPairs::iterator itr = _bufferEntryArrayPairs.begin();
        itr != _bufferEntryArrayPairs.end();
        ++itr)
    {
        itr->first.modifiedCount.resize(maxSize);
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//  ElementBufferObject
//
ElementBufferObject::ElementBufferObject()
{
    _target = GL_ELEMENT_ARRAY_BUFFER_ARB;
    _usage = GL_STATIC_DRAW_ARB;
}

ElementBufferObject::ElementBufferObject(const ElementBufferObject& vbo,const CopyOp& copyop):
    BufferObject(vbo,copyop)
{
}

ElementBufferObject::~ElementBufferObject()
{
}

unsigned int ElementBufferObject::addDrawElements(osg::DrawElements* drawElements)
{
    unsigned int i = _bufferEntryDrawElementsPairs.size();
    _bufferEntryDrawElementsPairs.resize(i+1);
    _bufferEntryDrawElementsPairs[i].second = drawElements;
    _bufferEntryDrawElementsPairs[i].first.modifiedCount.setAllElementsTo(0xffffffff);
    _bufferEntryDrawElementsPairs[i].first.dataSize = 0;

    return i;
}

void ElementBufferObject::removeDrawElements(osg::DrawElements* drawElements)
{
    BufferEntryDrawElementsPairs::iterator itr;
    for(itr = _bufferEntryDrawElementsPairs.begin();
        itr != _bufferEntryDrawElementsPairs.end();
        ++itr)
    {
        if (itr->second == drawElements) break;
    }
    if (itr != _bufferEntryDrawElementsPairs.end()) _bufferEntryDrawElementsPairs.erase(itr);
}

void ElementBufferObject::setDrawElements(unsigned int i, DrawElements* drawElements)
{
    if (i+1>=_bufferEntryDrawElementsPairs.size()) _bufferEntryDrawElementsPairs.resize(i+1);

    _bufferEntryDrawElementsPairs[i].second = drawElements;
    _bufferEntryDrawElementsPairs[i].first.modifiedCount.setAllElementsTo(0xffffffff);
    _bufferEntryDrawElementsPairs[i].first.dataSize = 0;
}

void ElementBufferObject::compileBuffer(State& state) const
{
    unsigned int contextID = state.getContextID();

    _compiledList[contextID] = 1;

    // osg::notify(osg::NOTICE)<<"ElementBufferObject::compile"<<std::endl;

    Extensions* extensions = getExtensions(contextID,true);

    unsigned int totalSizeRequired = 0;
//    unsigned int numModified = 0;
//    unsigned int numNotModified = 0;
    for(BufferEntryDrawElementsPairs::const_iterator itr = _bufferEntryDrawElementsPairs.begin();
        itr != _bufferEntryDrawElementsPairs.end();
        ++itr)
    {
        const BufferEntryDrawElementsPair& bep = *itr;
        if (bep.second)
        {
            totalSizeRequired += bep.second->getTotalDataSize();
        }
    }

    bool copyAll = false;
    GLuint& ebo = buffer(contextID);
    if (ebo==0)
    {
        // building for the first time.

        _totalSize = totalSizeRequired;

        // don't generate buffer if size is zero.
        if (_totalSize==0) return;

        extensions->glGenBuffers(1, &ebo);
        extensions->glBindBuffer(_target, ebo);
        extensions->glBufferData(_target, _totalSize, NULL, _usage);

        copyAll = true;
    }
    else
    {
        extensions->glBindBuffer(_target, ebo);

        if (_totalSize != totalSizeRequired)
        {
            // resize EBO.
            _totalSize = totalSizeRequired;
            extensions->glBufferData(_target, _totalSize, NULL, _usage);

            copyAll = true;
        }
    }

//    osg::Timer_t start_tick = osg::Timer::instance()->tick();


    void* eboMemory = 0;

#if 0
    eboMemory = extensions->glMapBuffer(_target, GL_WRITE_ONLY_ARB);
#endif

    size_t offset = 0;
    for(BufferEntryDrawElementsPairs::const_iterator itr = _bufferEntryDrawElementsPairs.begin();
        itr != _bufferEntryDrawElementsPairs.end();
        ++itr)
    {
        const BufferEntryDrawElementsPair& bep = *itr;
        const DrawElements* de = bep.second;
        if (de)
        {
            if (copyAll ||
                bep.first.modifiedCount[contextID] != bep.second->getModifiedCount() ||
                bep.first.dataSize != bep.second->getTotalDataSize())
            {
                // copy data across
                bep.first.dataSize = bep.second->getTotalDataSize();
                bep.first.modifiedCount[contextID] = de->getModifiedCount();
                if (copyAll)
                {
                    bep.first.offset = offset;
                    de->setElementBufferObjectOffset((GLvoid*)offset);
                    offset += bep.first.dataSize;
                }

                if (eboMemory)
                    memcpy((char*)eboMemory + bep.first.offset, de->getDataPointer(), bep.first.dataSize);
                else
                    extensions->glBufferSubData(_target, bep.first.offset, bep.first.dataSize, de->getDataPointer());

            }
        }
    }


    // Unmap the texture image buffer
    if (eboMemory) extensions->glUnmapBuffer(_target);

//    osg::notify(osg::NOTICE)<<"pbo _totalSize="<<_totalSize<<std::endl;
//    osg::notify(osg::NOTICE)<<"pbo "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
}

void ElementBufferObject::resizeGLObjectBuffers(unsigned int maxSize)
{
    BufferObject::resizeGLObjectBuffers(maxSize);

    for(BufferEntryDrawElementsPairs::iterator itr = _bufferEntryDrawElementsPairs.begin();
        itr != _bufferEntryDrawElementsPairs.end();
        ++itr)
    {
        itr->first.modifiedCount.resize(maxSize);
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//  PixelBufferObject
//
PixelBufferObject::PixelBufferObject(osg::Image* image):
    BufferObject()
{
    _target = GL_PIXEL_UNPACK_BUFFER_ARB;
    _usage = GL_STREAM_DRAW_ARB;
    _bufferEntryImagePair.second = image;
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
PixelBufferObject::PixelBufferObject(const PixelBufferObject& buffer,const CopyOp& copyop):
    BufferObject(buffer,copyop),
    _bufferEntryImagePair(buffer._bufferEntryImagePair)
{
}

PixelBufferObject::~PixelBufferObject()
{
}

void PixelBufferObject::setImage(osg::Image* image)
{
    if (_bufferEntryImagePair.second == image) return;

    _bufferEntryImagePair.second = image;

    dirty();
}
void PixelBufferObject::compileBuffer(State& state) const
{
    unsigned int contextID = state.getContextID();

    _compiledList[contextID] = 1;

    osg::Image* image = _bufferEntryImagePair.second;

    _bufferEntryImagePair.first.modifiedCount[contextID] = image->getModifiedCount();
    if (!image->valid()) return;

    Extensions* extensions = getExtensions(contextID,true);

    GLuint& pbo = buffer(contextID);
    if (pbo==0)
    {
        // building for the first time.

        _totalSize = image->getTotalSizeInBytes();

        // don't generate buffer if size is zero.
        if (_totalSize==0) return;

        extensions->glGenBuffers(1, &pbo);
        extensions->glBindBuffer(_target, pbo);
        extensions->glBufferData(_target, _totalSize, NULL, _usage);

    }
    else
    {
        extensions->glBindBuffer(_target, pbo);

        if (_totalSize != image->getTotalSizeInBytes())
        {
            // resize PBO.
            _totalSize = image->getTotalSizeInBytes();
            extensions->glBufferData(_target, _totalSize, NULL, _usage);
        }
    }

//    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    void* pboMemory = extensions->glMapBuffer(_target,
                 GL_WRITE_ONLY_ARB);

    // copy data across
    memcpy(pboMemory, image->data(), _totalSize);

    // Unmap the texture image buffer
    extensions->glUnmapBuffer(_target);

    _bufferEntryImagePair.first.modifiedCount[contextID] = image->getModifiedCount();

//    osg::notify(osg::NOTICE)<<"pbo _totalSize="<<_totalSize<<std::endl;
//    osg::notify(osg::NOTICE)<<"pbo "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
}

void PixelBufferObject::resizeGLObjectBuffers(unsigned int maxSize)
{
    BufferObject::resizeGLObjectBuffers(maxSize);

    _bufferEntryImagePair.first.modifiedCount.resize(maxSize);
}


//////////////////////////////////////////////////////////////////////////////////
//
//  PixelDataBufferObject
//
//--------------------------------------------------------------------------------
PixelDataBufferObject::PixelDataBufferObject()
{
    _target = GL_ARRAY_BUFFER_ARB;
    _usage = GL_DYNAMIC_DRAW_ARB;
    _bufferData.dataSize = 0;
}

//--------------------------------------------------------------------------------
PixelDataBufferObject::PixelDataBufferObject(const PixelDataBufferObject& buffer,const CopyOp& copyop):
    BufferObject(buffer,copyop),
    _bufferData(buffer._bufferData)
{
}

//--------------------------------------------------------------------------------
PixelDataBufferObject::~PixelDataBufferObject()
{
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::compileBuffer(State& state) const
{
    unsigned int contextID = state.getContextID();    
    if (!isDirty(contextID) || _bufferData.dataSize == 0) return;

    Extensions* extensions = getExtensions(contextID,true);

    GLuint& pbo = buffer(contextID);
    if (pbo == 0)
    {
        extensions->glGenBuffers(1, &pbo);
    }

    extensions->glBindBuffer(_target, pbo);
    extensions->glBufferData(_target, _bufferData.dataSize, NULL, _usage);
    extensions->glBindBuffer(_target, 0);

    _compiledList[contextID] = 1;
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::bindBufferInReadMode(State& state)
{
    unsigned int contextID = state.getContextID();    
    if (isDirty(contextID)) compileBuffer(state);

    Extensions* extensions = getExtensions(contextID,true);

    extensions->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, buffer(contextID));
    _mode[contextID] = READ;
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::bindBufferInWriteMode(State& state)
{
    unsigned int contextID = state.getContextID();    
    if (isDirty(contextID)) compileBuffer(state);

    Extensions* extensions = getExtensions(contextID,true);

    extensions->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, buffer(contextID));
    _mode[contextID] = WRITE;
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::unbindBuffer(unsigned int contextID) const
{ 
    Extensions* extensions = getExtensions(contextID,true);

    switch(_mode[contextID])
    {
        case READ:
            extensions->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB,0);
            break;
        case WRITE:
            extensions->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB,0);
            break;
        default:
            extensions->glBindBuffer(_target,0);
            break;
    }

    _mode[contextID] = NONE;
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::resizeGLObjectBuffers(unsigned int maxSize)
{
    BufferObject::resizeGLObjectBuffers(maxSize);

    _mode.resize(maxSize);
}

