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

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GLBufferObject
//
GLBufferObject::GLBufferObject(unsigned int contextID, BufferObject* bufferObject):
    _contextID(contextID),
    _glObjectID(0),
    _target(0),
    _usage(0),
    _dirty(true),
    _totalSize(0),
    _bufferObject(0),
    _extensions(0)
{
    assign(bufferObject);
    _extensions = GLBufferObject::getExtensions(contextID, true);
    _extensions->glGenBuffers(1, &_glObjectID);
}

GLBufferObject::~GLBufferObject()
{
    if (_glObjectID!=0) GLBufferObject::deleteBufferObject(_contextID, _glObjectID);

}

void GLBufferObject::assign(BufferObject* bufferObject)
{
    _bufferObject = bufferObject;

    if (_bufferObject)
    {
        _target = bufferObject->getTarget();
        _usage = bufferObject->getUsage();

        _totalSize = 0;

        _dirty = true;

        _bufferEntries.clear();
    }
    else
    {
        _target = 0;
        _usage = 0;

        _totalSize = 0;

        // clear all previous entries;
        _bufferEntries.clear();
    }
}

void GLBufferObject::clear()
{
    _bufferEntries.clear();
    _dirty = true;
}

void GLBufferObject::compileBuffer()
{
    _dirty = false;

    _bufferEntries.reserve(_bufferObject->getNumBufferData());

    _totalSize = 0;

    bool compileAll = false;
    bool offsetChanged = false;

    GLsizeiptrARB newTotalSize = 0;
    unsigned int i=0;
    for(; i<_bufferObject->getNumBufferData(); ++i)
    {
        BufferData* bd = _bufferObject->getBufferData(i);
        if (i<_bufferEntries.size())
        {
            BufferEntry& entry = _bufferEntries[i];
            if (offsetChanged ||
                entry.dataSource != bd ||
                entry.dataSize != bd->getTotalDataSize())
            {
                GLsizeiptrARB previousEndOfBufferDataMarker = GLsizeiptrARB(entry.offset) + entry.dataSize;

                // osg::notify(osg::NOTICE)<<"GLBufferObject::compileBuffer(..) updating BufferEntry"<<std::endl;


                entry.offset = newTotalSize;
                entry.modifiedCount = 0xffffff;
                entry.dataSize = bd->getTotalDataSize();
                entry.dataSource = bd;

                newTotalSize += entry.dataSize;
                if (previousEndOfBufferDataMarker==newTotalSize)
                {
                    offsetChanged = true;
                }
            }
        }
        else
        {
            BufferEntry entry;
            entry.offset = newTotalSize;
            entry.modifiedCount = 0xffffff;
            entry.dataSize = bd->getTotalDataSize();
            entry.dataSource = bd;
#if 0
            osg::notify(osg::NOTICE)<<"entry"<<std::endl;
            osg::notify(osg::NOTICE)<<"   offset "<<entry.offset<<std::endl;
            osg::notify(osg::NOTICE)<<"   dataSize "<<entry.dataSize<<std::endl;
            osg::notify(osg::NOTICE)<<"   dataSource "<<entry.dataSource<<std::endl;
            osg::notify(osg::NOTICE)<<"   modifiedCount "<<entry.modifiedCount<<std::endl;
#endif
            newTotalSize += entry.dataSize;

            _bufferEntries.push_back(entry);
        }
    }

    if (i<_bufferEntries.size())
    {
        // triming end of bufferEntries as the source data is has less entries than the originally held.
        _bufferEntries.erase(_bufferEntries.begin()+i, _bufferEntries.end());
    }

    _extensions->glBindBuffer(_target, _glObjectID);

    if (newTotalSize != _totalSize)
    {
        _totalSize = newTotalSize;
        _extensions->glBufferData(_target, _totalSize, NULL, _usage);
    }

    char* vboMemory = 0;

#if 0
    vboMemory = extensions->glMapBuffer(_target, GL_WRITE_ONLY_ARB);
#endif

    for(BufferEntries::iterator itr = _bufferEntries.begin();
        itr != _bufferEntries.end();
        ++itr)
    {
        BufferEntry& entry = *itr;
        if (compileAll || entry.modifiedCount != entry.dataSource->getModifiedCount())
        {
            // osg::notify(osg::NOTICE)<<"GLBufferObject::compileBuffer(..) downloading BufferEntry "<<&entry<<std::endl;
            entry.modifiedCount = entry.dataSource->getModifiedCount();

            if (vboMemory)
                memcpy(vboMemory + (GLsizeiptrARB)entry.offset, entry.dataSource->getDataPointer(), entry.dataSize);
            else
                _extensions->glBufferSubData(_target, (GLintptrARB)entry.offset, (GLsizeiptrARB)entry.dataSize, entry.dataSource->getDataPointer());

        }
    }

    // Unmap the texture image buffer
    if (vboMemory) _extensions->glUnmapBuffer(_target);
}

GLBufferObject* GLBufferObject::createGLBufferObject(unsigned int contextID, const BufferObject* bufferObject)
{
    return new GLBufferObject(contextID, const_cast<BufferObject*>(bufferObject));
}

void GLBufferObject::deleteGLObject()
{
    if (_glObjectID!=0)
    {
        _extensions->glDeleteBuffers(1, &_glObjectID);
        _glObjectID = 0;

        _totalSize = 0;
        _bufferEntries.clear();
    }
}


void GLBufferObject::deleteBufferObject(unsigned int contextID,GLuint globj)
{
    if (globj!=0)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedBufferObjectCache);

        // insert the globj into the cache for the appropriate context.
        s_deletedBufferObjectCache[contextID].insert(BufferObjectMap::value_type(0,globj));
    }
}

void GLBufferObject::flushDeletedBufferObjects(unsigned int contextID,double /*currentTime*/, double& availableTime)
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

void GLBufferObject::discardDeletedBufferObjects(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedBufferObjectCache);
    BufferObjectMap& dll = s_deletedBufferObjectCache[contextID];
    dll.clear();
}

//////////////////////////////////////////////////////////////////////////////
//
//  Extension support
//

typedef buffered_value< ref_ptr<GLBufferObject::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

GLBufferObject::Extensions* GLBufferObject::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new GLBufferObject::Extensions(contextID);
    return s_extensions[contextID].get();
}

void GLBufferObject::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

GLBufferObject::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

GLBufferObject::Extensions::Extensions(const Extensions& rhs):
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


void GLBufferObject::Extensions::lowestCommonDenominator(const Extensions& rhs)
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

void GLBufferObject::Extensions::setupGLExtensions(unsigned int contextID)
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

void GLBufferObject::Extensions::glGenBuffers(GLsizei n, GLuint *buffers) const
{
    if (_glGenBuffers) _glGenBuffers(n, buffers);
    else notify(WARN)<<"Error: glGenBuffers not supported by OpenGL driver"<<std::endl;
}

void GLBufferObject::Extensions::glBindBuffer(GLenum target, GLuint buffer) const
{
    if (_glBindBuffer) _glBindBuffer(target, buffer);
    else notify(WARN)<<"Error: glBindBuffer not supported by OpenGL driver"<<std::endl;
}

void GLBufferObject::Extensions::glBufferData(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage) const
{
    if (_glBufferData) _glBufferData(target, size, data, usage);
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void GLBufferObject::Extensions::glBufferSubData(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data) const
{
    if (_glBufferSubData) _glBufferSubData(target, offset, size, data);
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void GLBufferObject::Extensions::glDeleteBuffers(GLsizei n, const GLuint *buffers) const
{
    if (_glDeleteBuffers) _glDeleteBuffers(n, buffers);
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

GLboolean GLBufferObject::Extensions::glIsBuffer (GLuint buffer) const
{
    if (_glIsBuffer) return _glIsBuffer(buffer);
    else
    {
        notify(WARN)<<"Error: glIsBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void GLBufferObject::Extensions::glGetBufferSubData (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data) const
{
    if (_glGetBufferSubData) _glGetBufferSubData(target,offset,size,data);
    else notify(WARN)<<"Error: glGetBufferSubData not supported by OpenGL driver"<<std::endl;
}

GLvoid* GLBufferObject::Extensions::glMapBuffer (GLenum target, GLenum access) const
{
    if (_glMapBuffer) return _glMapBuffer(target,access);
    else
    {
        notify(WARN)<<"Error: glMapBuffer not supported by OpenGL driver"<<std::endl;
        return 0;
    }
}

GLboolean GLBufferObject::Extensions::glUnmapBuffer (GLenum target) const
{
    if (_glUnmapBuffer) return _glUnmapBuffer(target);
    else
    {
        notify(WARN)<<"Error: glUnmapBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void GLBufferObject::Extensions::glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) const
{
    if (_glGetBufferParameteriv) _glGetBufferParameteriv(target,pname,params);
    else notify(WARN)<<"Error: glGetBufferParameteriv not supported by OpenGL driver"<<std::endl;
}

void GLBufferObject::Extensions::glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params) const
{
    if (_glGetBufferPointerv) _glGetBufferPointerv(target,pname,params);
    else notify(WARN)<<"Error: glGetBufferPointerv not supported by OpenGL driver"<<std::endl;
}

#if 1

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BufferObject
//
BufferObject::BufferObject():
    _target(0),
    _usage(0)
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

void BufferObject::setBufferData(unsigned int index, BufferData* bd)
{
    if (index>=_bufferDataList.size()) _bufferDataList.resize(index+1);
    _bufferDataList[index] = bd;
}

void BufferObject::dirty()
{
    for(unsigned int i=0; i<_glBufferObjects.size(); ++i)
    {
        if (_glBufferObjects[i].valid()) _glBufferObjects[i]->dirty();
    }
}

void BufferObject::resizeGLObjectBuffers(unsigned int maxSize)
{
    _glBufferObjects.resize(maxSize);
}

void BufferObject::releaseGLObjects(State* state) const
{
    if (state)
    {
        _glBufferObjects[state->getContextID()] = 0;
    }
    else
    {
        _glBufferObjects.clear();
    }
}

unsigned int BufferObject::addBufferData(BufferData* bd)
{
    if (!bd) return 0;

    // check to see if bd exists in BufferObject already, is so return without doing anything
    for(BufferDataList::iterator itr = _bufferDataList.begin();
        itr != _bufferDataList.end();
        ++itr)
    {
        if (*itr == bd) return bd->getBufferIndex();
    }

    // bd->setBufferIndex(_bufferDataList.size());

    _bufferDataList.push_back(bd);

    // osg::notify(osg::NOTICE)<<"BufferObject "<<this<<":"<<className()<<"::addBufferData("<<bd<<"), bufferIndex= "<<_bufferDataList.size()-1<<std::endl;

    return _bufferDataList.size()-1;
}

void BufferObject::removeBufferData(unsigned int index)
{
    if (index>=_bufferDataList.size())
    {
        osg::notify(osg::WARN)<<"Error "<<className()<<"::removeBufferData("<<index<<") out of range."<<std::endl;
        return;
    }

    // osg::notify(osg::NOTICE)<<"BufferObject::"<<this<<":"<<className()<<"::removeBufferData("<<index<<"), size= "<<_bufferDataList.size()<<std::endl;

    // alter the indices of the BufferData after the entry to be removed so their indices are correctly placed.
    for(unsigned int i=index+1; i<_bufferDataList.size(); ++i)
    {
        _bufferDataList[i]->setBufferIndex(i-1);
    }

    // remove the entry
    _bufferDataList.erase(_bufferDataList.begin() + index);

    for(unsigned int i=0; i<_glBufferObjects.size(); ++i)
    {
        if (_glBufferObjects[i].valid()) _glBufferObjects[i]->clear();
    }

}

void BufferObject::removeBufferData(BufferData* bd)
{
    // osg::notify(osg::NOTICE)<<"BufferObject::"<<this<<":"<<className()<<"::removeBufferData("<<bd<<"), index="<<bd->getBufferIndex()<<" size= "<<_bufferDataList.size()<<std::endl;

    if (!bd || bd->getBufferObject()!=this) return;

    removeBufferData(bd->getBufferIndex());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BufferData
//
BufferData::~BufferData()
{
    setBufferObject(0);
}

void BufferData::setBufferObject(BufferObject* bufferObject)
{
    if (_bufferObject==bufferObject) return;

    if (_bufferObject.valid())
    {
        _bufferObject->removeBufferData(_bufferIndex);
    }

    _bufferObject = bufferObject;
    _bufferIndex = _bufferObject.valid() ? _bufferObject->addBufferData(this) : 0;
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
    return addBufferData(array);
}

void VertexBufferObject::removeArray(osg::Array* array)
{
    removeBufferData(array);
}

void VertexBufferObject::setArray(unsigned int i, Array* array)
{
    setBufferData(i,array);
}

Array* VertexBufferObject::getArray(unsigned int i)
{
    return dynamic_cast<osg::Array*>(getBufferData(i));
}

const Array* VertexBufferObject::getArray(unsigned int i) const
{
    return dynamic_cast<const osg::Array*>(getBufferData(i));
}
#if 0
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
#endif

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
    return addBufferData(drawElements);
}

void ElementBufferObject::removeDrawElements(osg::DrawElements* drawElements)
{
    removeBufferData(drawElements);
}

void ElementBufferObject::setDrawElements(unsigned int i, DrawElements* drawElements)
{
    setBufferData(i,drawElements);
}

DrawElements* ElementBufferObject::getDrawElements(unsigned int i)
{
    return dynamic_cast<DrawElements*>(getBufferData(i));
}

const DrawElements* ElementBufferObject::getDrawElements(unsigned int i) const
{
    return dynamic_cast<const DrawElements*>(getBufferData(i));
}


#if 0
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
#endif

//////////////////////////////////////////////////////////////////////////////////
//
//  PixelBufferObject
//
PixelBufferObject::PixelBufferObject(osg::Image* image):
    BufferObject()
{
    _target = GL_PIXEL_UNPACK_BUFFER_ARB;
    _usage = GL_STREAM_DRAW_ARB;

    osg::notify(osg::NOTICE)<<"Constructing PixelBufferObject for image="<<image<<std::endl;

    setBufferData(0, image);
}

PixelBufferObject::PixelBufferObject(const PixelBufferObject& buffer,const CopyOp& copyop):
    BufferObject(buffer,copyop)
{
}

PixelBufferObject::~PixelBufferObject()
{
}

void PixelBufferObject::setImage(osg::Image* image)
{
    setBufferData(0, image);
}

Image* PixelBufferObject::getImage()
{
    return dynamic_cast<Image*>(getBufferData(0));
}

const Image* PixelBufferObject::getImage() const
{
    return dynamic_cast<const Image*>(getBufferData(0));
}

#if 0
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
#endif

//////////////////////////////////////////////////////////////////////////////////
//
//  PixelDataBufferObject
//
//--------------------------------------------------------------------------------
PixelDataBufferObject::PixelDataBufferObject()
{
    _target = GL_ARRAY_BUFFER_ARB;
    _usage = GL_DYNAMIC_DRAW_ARB;
}

//--------------------------------------------------------------------------------
PixelDataBufferObject::PixelDataBufferObject(const PixelDataBufferObject& buffer,const CopyOp& copyop):
    BufferObject(buffer,copyop)
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
    if ( _dataSize == 0) return;

    GLBufferObject* bo = getOrCreateGLBufferObject(contextID);
    if (!bo || !bo->isDirty()) return;

    bo->_extensions->glBindBuffer(_target, bo->getGLObjectID());
    bo->_extensions->glBufferData(_target, _dataSize, NULL, _usage);
    bo->_extensions->glBindBuffer(_target, 0);
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::bindBufferInReadMode(State& state)
{
    unsigned int contextID = state.getContextID();    

    GLBufferObject* bo = getOrCreateGLBufferObject(contextID);
    if (!bo) return;

    if (bo->isDirty()) compileBuffer(state);

    bo->_extensions->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, bo->getGLObjectID());

    _mode[contextID] = READ;
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::bindBufferInWriteMode(State& state)
{
    unsigned int contextID = state.getContextID();    

    GLBufferObject* bo = getOrCreateGLBufferObject(contextID);
    if (!bo) return;

    if (bo->isDirty()) compileBuffer(state);

    bo->_extensions->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, bo->getGLObjectID());

    _mode[contextID] = WRITE;
}

//--------------------------------------------------------------------------------
void PixelDataBufferObject::unbindBuffer(unsigned int contextID) const
{ 
    GLBufferObject::Extensions* extensions = GLBufferObject::getExtensions(contextID,true);

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


#endif