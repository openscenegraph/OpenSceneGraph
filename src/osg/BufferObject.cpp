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

unsigned int s_minimumNumberOfGLBufferObjectsToRetainInCache = 1000;

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GLBufferObject
//
GLBufferObject::GLBufferObject(unsigned int contextID, BufferObject* bufferObject):
    _contextID(contextID),
    _glObjectID(0),
    _profile(0,0,0),
    _allocatedSize(0),
    _dirty(true),
    _bufferObject(0),
    _set(0),
    _previous(0),
    _next(0),
    _extensions(0)
{
    assign(bufferObject);
    _extensions = GLBufferObject::getExtensions(contextID, true);
    _extensions->glGenBuffers(1, &_glObjectID);
}

GLBufferObject::~GLBufferObject()
{
}

void GLBufferObject::bindBuffer()
{
    _extensions->glBindBuffer(_profile._target,_glObjectID);
    if (_set) _set->moveToBack(this);
}

void GLBufferObject::setBufferObject(BufferObject* bufferObject)
{
    assign(bufferObject);
}

void GLBufferObject::assign(BufferObject* bufferObject)
{
    _bufferObject = bufferObject;

    if (_bufferObject)
    {
        _profile = bufferObject->getProfile();

        _dirty = true;

        _bufferEntries.clear();
    }
    else
    {
        _profile.setProfile(0,0,0);

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

    bool compileAll = false;
    bool offsetChanged = false;

    unsigned int newTotalSize = 0;
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
                unsigned int previousEndOfBufferDataMarker = entry.offset + entry.dataSize;

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

    _extensions->glBindBuffer(_profile._target, _glObjectID);

    if (newTotalSize > _profile._size)
    {
        osg::notify(osg::NOTICE)<<"newTotalSize="<<newTotalSize<<", _profile._size="<<_profile._size<<std::endl;

        _profile._size = newTotalSize;

        if (_set)
        {
            _set->moveToSet(this, _set->getParent()->getGLBufferObjectSet(_profile));
        }

    }

    if (_allocatedSize != _profile._size)
    {
        _allocatedSize = _profile._size;
        _extensions->glBufferData(_profile._target, _profile._size, NULL, _profile._usage);
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
                _extensions->glBufferSubData(_profile._target, (GLintptrARB)entry.offset, (GLsizeiptrARB)entry.dataSize, entry.dataSource->getDataPointer());

        }
    }

    // Unmap the texture image buffer
    if (vboMemory) _extensions->glUnmapBuffer(_profile._target);


}

void GLBufferObject::deleteGLObject()
{
    if (_glObjectID!=0)
    {
        _extensions->glDeleteBuffers(1, &_glObjectID);
        _glObjectID = 0;

        _allocatedSize = 0;
        _bufferEntries.clear();
    }
}

void GLBufferObject::deleteBufferObject(unsigned int contextID,GLuint globj)
{
    osg::notify(osg::NOTICE)<<"GLBufferObject::deleteBufferObject("<<std::endl;
}
#if 0

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
#endif
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

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GLBufferObjectSet
//
GLBufferObjectSet::GLBufferObjectSet(GLBufferObjectManager* parent, const BufferObjectProfile& profile):
    _parent(parent),
    _contextID(parent->getContextID()),
    _profile(profile),
    _numOfGLBufferObjects(0),
    _head(0),
    _tail(0)
{
    osg::notify(osg::NOTICE)<<"GLBufferObjectSet::GLBufferObjectSet _profile._size="<<_profile._size<<std::endl;
}

GLBufferObjectSet::~GLBufferObjectSet()
{
#if 0
    osg::notify(osg::NOTICE)<<"GLBufferObjectSet::~GLBufferObjectSet(), _numOfGLBufferObjects="<<_numOfGLBufferObjects<<std::endl;
    osg::notify(osg::NOTICE)<<"     _orphanedGLBufferObjects = "<<_orphanedGLBufferObjects.size()<<std::endl;
    osg::notify(osg::NOTICE)<<"     _head = "<<_head<<std::endl;
    osg::notify(osg::NOTICE)<<"     _tail = "<<_tail<<std::endl;
#endif
}

bool GLBufferObjectSet::checkConsistency() const
{
    return true;

    // osg::notify(osg::NOTICE)<<"GLBufferObjectSet::checkConsistency()"<<std::endl;
    // check consistency of linked list.
    unsigned int numInList = 0;
    GLBufferObject* to = _head;
    while(to!=0)
    {
        ++numInList;

        if (to->_next)
        {
            if ((to->_next)->_previous != to)
            {
                osg::notify(osg::NOTICE)<<"Error (to->_next)->_previous != to "<<std::endl;
                throw "Error (to->_next)->_previous != to ";
            }
        }
        else
        {
            if (_tail != to)
            {
                osg::notify(osg::NOTICE)<<"Error _trail != to"<<std::endl;
                throw "Error _trail != to";
            }
        }

        to = to->_next;
    }

    unsigned int totalNumber = numInList + _orphanedGLBufferObjects.size();
    if (totalNumber != _numOfGLBufferObjects)
    {
        osg::notify(osg::NOTICE)<<"Error numInList + _orphanedGLBufferObjects.size() != _numOfGLBufferObjects"<<std::endl;
        osg::notify(osg::NOTICE)<<"    numInList = "<<numInList<<std::endl;
        osg::notify(osg::NOTICE)<<"    _orphanedGLBufferObjects.size() = "<<_orphanedGLBufferObjects.size()<<std::endl;
        osg::notify(osg::NOTICE)<<"    _pendingOrphanedGLBufferObjects.size() = "<<_pendingOrphanedGLBufferObjects.size()<<std::endl;
        osg::notify(osg::NOTICE)<<"    _numOfGLBufferObjects = "<<_numOfGLBufferObjects<<std::endl;
        throw "Error numInList + _orphanedGLBufferObjects.size() != _numOfGLBufferObjects";
    }

    return true;
}

void GLBufferObjectSet::handlePendingOrphandedGLBufferObjects()
{
//    osg::notify(osg::NOTICE)<<"handlePendingOrphandedGLBufferObjects()"<<_pendingOrphanedGLBufferObjects.size()<<std::endl;

    if (_pendingOrphanedGLBufferObjects.empty()) return;

    unsigned int numOrphaned = _pendingOrphanedGLBufferObjects.size();

    for(GLBufferObjectList::iterator itr = _pendingOrphanedGLBufferObjects.begin();
        itr != _pendingOrphanedGLBufferObjects.end();
        ++itr)
    {
        GLBufferObject* to = itr->get();

        _orphanedGLBufferObjects.push_back(to);

        remove(to);

#if 0
        osg::notify(osg::NOTICE)<<"  HPOTO  after  _head = "<<_head<<std::endl;
        osg::notify(osg::NOTICE)<<"  HPOTO  after _tail = "<<_tail<<std::endl;
        osg::notify(osg::NOTICE)<<"  HPOTO  after to->_previous = "<<to->_previous<<std::endl;
        osg::notify(osg::NOTICE)<<"  HPOTO  after to->_next = "<<to->_next<<std::endl;
#endif

    }


    // update the GLBufferObjectManager's running total of active + orphaned GLBufferObjects
    _parent->getNumberOrphanedGLBufferObjects() += numOrphaned;
    _parent->getNumberActiveGLBufferObjects() -= numOrphaned;

    _pendingOrphanedGLBufferObjects.clear();

    checkConsistency();
}

void GLBufferObjectSet::flushAllDeletedGLBufferObjects()
{
    for(GLBufferObjectList::iterator itr = _orphanedGLBufferObjects.begin();
        itr != _orphanedGLBufferObjects.end();
        ++itr)
    {

        (*itr)->deleteGLObject();

        osg::notify(osg::NOTICE)<<"Deleting textureobject id="<<(*itr)->getGLObjectID()<<std::endl;
    }

    unsigned int numDeleted = _orphanedGLBufferObjects.size();
    _numOfGLBufferObjects -= numDeleted;

    // update the GLBufferObjectManager's running total of current pool size
    _parent->getCurrGLBufferObjectPoolSize() -= numDeleted*_profile._size;
    _parent->getNumberOrphanedGLBufferObjects() -= numDeleted;
    _parent->getNumberDeleted() += numDeleted;

    _orphanedGLBufferObjects.clear();
}

void GLBufferObjectSet::discardAllDeletedGLBufferObjects()
{
    unsigned int numDiscarded = _orphanedGLBufferObjects.size();

    _numOfGLBufferObjects -= numDiscarded;

    // update the GLBufferObjectManager's running total of current pool size
    _parent->setCurrGLBufferObjectPoolSize( _parent->getCurrGLBufferObjectPoolSize() - numDiscarded*_profile._size );

    // update the number of active and orphaned TextureOjects
    _parent->getNumberOrphanedGLBufferObjects() -= 1;
    _parent->getNumberActiveGLBufferObjects() += 1;
    _parent->getNumberDeleted() += 1;


    // just clear the list as there is nothing else we can do with them when discarding them
    _orphanedGLBufferObjects.clear();
}

void GLBufferObjectSet::flushDeletedGLBufferObjects(double currentTime, double& availableTime)
{
    // if nothing to delete return
    if (_orphanedGLBufferObjects.empty()) return;

    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    // if we don't have too many orphaned texture objects then don't bother deleting them, as we can potentially reuse them later.
    if (_parent->getNumberOrphanedGLBufferObjects()<=s_minimumNumberOfGLBufferObjectsToRetainInCache) return;

    unsigned int numDeleted = 0;
    unsigned int maxNumObjectsToDelete = _parent->getNumberOrphanedGLBufferObjects()-s_minimumNumberOfGLBufferObjectsToRetainInCache;
    if (maxNumObjectsToDelete>4) maxNumObjectsToDelete = 4;

    ElapsedTime timer;

    GLBufferObjectList::iterator itr = _orphanedGLBufferObjects.begin();
    for(;
        itr != _orphanedGLBufferObjects.end() && timer.elapsedTime()<availableTime && numDeleted<maxNumObjectsToDelete;
        ++itr)
    {

        osg::notify(osg::NOTICE)<<"Deleting textureobject id="<<itr->get()<<std::endl;

         (*itr)->deleteGLObject();

        ++numDeleted;
    }

    // osg::notify(osg::NOTICE)<<"Size before = "<<_orphanedGLBufferObjects.size();
    _orphanedGLBufferObjects.erase(_orphanedGLBufferObjects.begin(), itr);
    //osg::notify(osg::NOTICE)<<", after = "<<_orphanedGLBufferObjects.size()<<" numDeleted = "<<numDeleted<<std::endl;

    // update the number of TO's in this GLBufferObjectSet
    _numOfGLBufferObjects -= numDeleted;

    _parent->setCurrGLBufferObjectPoolSize( _parent->getCurrGLBufferObjectPoolSize() - numDeleted*_profile._size );

    // update the number of active and orphaned TextureOjects
    _parent->getNumberOrphanedGLBufferObjects() -= numDeleted;
    _parent->getNumberActiveGLBufferObjects() += numDeleted;
    _parent->getNumberDeleted() += numDeleted;

    availableTime -= timer.elapsedTime();
}

bool GLBufferObjectSet::makeSpace(unsigned int& size)
{
    if (!_orphanedGLBufferObjects.empty())
    {
        unsigned int sizeAvailable = _orphanedGLBufferObjects.size() * _profile._size;
        if (size>sizeAvailable) size -= sizeAvailable;
        else size = 0;

        flushAllDeletedGLBufferObjects();
    }

    return size==0;
}

GLBufferObject* GLBufferObjectSet::takeFromOrphans(BufferObject* bufferObject)
{
    // take front of orphaned list.
    ref_ptr<GLBufferObject> glbo = _orphanedGLBufferObjects.front();

    // remove from orphan list.
    _orphanedGLBufferObjects.pop_front();

    // assign to new texture
    glbo->assign(bufferObject);
    glbo->setProfile(_profile);

    // update the number of active and orphaned TextureOjects
    _parent->getNumberOrphanedGLBufferObjects() -= 1;
    _parent->getNumberActiveGLBufferObjects() += 1;

    // place at back of active list
    addToBack(glbo.get());

    // osg::notify(osg::NOTICE)<<"Reusing orhpahned GLBufferObject, _numOfGLBufferObjects="<<_numOfGLBufferObjects<<std::endl;

    return glbo.release();
}


GLBufferObject* GLBufferObjectSet::takeOrGenerate(BufferObject* bufferObject)
{
    // see if we can recyle GLBufferObject from the orphane list
    if (!_pendingOrphanedGLBufferObjects.empty())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        handlePendingOrphandedGLBufferObjects();
        return takeFromOrphans(bufferObject);
    }
    else if (!_orphanedGLBufferObjects.empty())
    {
        return takeFromOrphans(bufferObject);
    }

    unsigned int minFrameNumber = _parent->getFrameNumber();

    // see if we can reuse GLBufferObject by taking the least recently used active GLBufferObject
    if ((_parent->getMaxGLBufferObjectPoolSize()!=0) &&
        (!_parent->hasSpace(_profile._size)) &&
        (_numOfGLBufferObjects>1) &&
        (_head != 0) &&
        (_head->_frameLastUsed<minFrameNumber))
    {

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        ref_ptr<GLBufferObject> glbo = _head;

        ref_ptr<BufferObject> original_BufferObject = glbo->getBufferObject();

        if (original_BufferObject.valid())
        {
            original_BufferObject->setGLBufferObject(_contextID,0);
            // osg::notify(osg::NOTICE)<<"GLBufferObjectSet="<<this<<": Reusing an active GLBufferObject "<<glbo.get()<<" _numOfGLBufferObjects="<<_numOfGLBufferObjects<<" size="<<_profile._size<<std::endl;
        }
        else
        {
            // osg::notify(osg::NOTICE)<<"Reusing a recently orphaned active GLBufferObject "<<glbo.get()<<std::endl;
        }

        moveToBack(glbo.get());

        // assign to new texture
        glbo->setBufferObject(bufferObject);
        glbo->setProfile(_profile);

        return glbo.release();
    }

    //
    GLBufferObject* glbo = new GLBufferObject(_contextID, const_cast<BufferObject*>(bufferObject));
    glbo->setProfile(_profile);
    glbo->_set = this;
    ++_numOfGLBufferObjects;

    // update the current texture pool size
    _parent->getCurrGLBufferObjectPoolSize() += _profile._size;
    _parent->getNumberActiveGLBufferObjects() += 1;

    addToBack(glbo);

    // osg::notify(osg::NOTICE)<<"Created new GLBufferObject, _numOfGLBufferObjects "<<_numOfGLBufferObjects<<std::endl;

    return glbo;
}

void GLBufferObjectSet::moveToBack(GLBufferObject* to)
{
#if 0
    osg::notify(osg::NOTICE)<<"GLBufferObjectSet::moveToBack("<<to<<")"<<std::endl;
    osg::notify(osg::NOTICE)<<"    before _head = "<<_head<<std::endl;
    osg::notify(osg::NOTICE)<<"    before _tail = "<<_tail<<std::endl;
    osg::notify(osg::NOTICE)<<"    before to->_previous = "<<to->_previous<<std::endl;
    osg::notify(osg::NOTICE)<<"    before to->_next = "<<to->_next<<std::endl;
#endif

    to->_frameLastUsed = _parent->getFrameNumber();

    // nothing to do if we are already tail
    if (to==_tail) return;

    // if no tail exists then assign 'to' as tail and head
    if (_tail==0)
    {
        osg::notify(osg::NOTICE)<<"Error ***************** Should not get here !!!!!!!!!"<<std::endl;
        _head = to;
        _tail = to;
        return;
    }

    if (to->_next==0)
    {
        osg::notify(osg::NOTICE)<<"Error ***************** Should not get here either !!!!!!!!!"<<std::endl;
        return;
    }


    if (to->_previous)
    {
        (to->_previous)->_next = to->_next;
    }
    else
    {
        // 'to' is the head, so moving it to the back will mean we need a new head
        if (to->_next)
        {
            _head = to->_next;
        }
    }

    (to->_next)->_previous = to->_previous;

    _tail->_next = to;

    to->_previous = _tail;
    to->_next = 0;

    _tail = to;

#if 0
    osg::notify(osg::NOTICE)<<"  m2B   after  _head = "<<_head<<std::endl;
    osg::notify(osg::NOTICE)<<"  m2B   after _tail = "<<_tail<<std::endl;
    osg::notify(osg::NOTICE)<<"  m2B   after to->_previous = "<<to->_previous<<std::endl;
    osg::notify(osg::NOTICE)<<"  m2B   after to->_next = "<<to->_next<<std::endl;
#endif
    checkConsistency();
}

void GLBufferObjectSet::addToBack(GLBufferObject* to)
{
#if 0
    osg::notify(osg::NOTICE)<<"GLBufferObjectSet::addToBack("<<to<<")"<<std::endl;
    osg::notify(osg::NOTICE)<<"    before _head = "<<_head<<std::endl;
    osg::notify(osg::NOTICE)<<"    before _tail = "<<_tail<<std::endl;
    osg::notify(osg::NOTICE)<<"    before to->_previous = "<<to->_previous<<std::endl;
    osg::notify(osg::NOTICE)<<"    before to->_next = "<<to->_next<<std::endl;
#endif

    if (to->_previous !=0 || to->_next !=0)
    {
        moveToBack(to);
    }
    else
    {
        to->_frameLastUsed = _parent->getFrameNumber();

        if (_tail) _tail->_next = to;
        to->_previous = _tail;

        if (!_head) _head = to;
        _tail = to;
    }
#if 0
    osg::notify(osg::NOTICE)<<"  a2B  after  _head = "<<_head<<std::endl;
    osg::notify(osg::NOTICE)<<"  a2B   after _tail = "<<_tail<<std::endl;
    osg::notify(osg::NOTICE)<<"  a2B   after to->_previous = "<<to->_previous<<std::endl;
    osg::notify(osg::NOTICE)<<"  a2B   after to->_next = "<<to->_next<<std::endl;
#endif
    checkConsistency();
}

void GLBufferObjectSet::orphan(GLBufferObject* to)
{
    // osg::notify(osg::NOTICE)<<"GLBufferObjectSet::orphan("<<to<<")"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    // disconnect from original texture
    to->setBufferObject(0);

    // add orphan 'to' to the pending list of orphans, these will then be
    // handled in the handlePendingOrphandedGLBufferObjects() where the TO's
    // will be removed from the active list, and then placed in the orhpanGLBufferObject
    // list.  This double buffered approach to handling orphaned TO's is used
    // to avoid having to mutex the process of appling active TO's.
    _pendingOrphanedGLBufferObjects.push_back(to);

#if 0
    osg::notify(osg::NOTICE)<<"GLBufferObjectSet::orphan("<<to<<")  _pendingOrphanedGLBufferObjects.size()="<<_pendingOrphanedGLBufferObjects.size()<<std::endl;
    osg::notify(osg::NOTICE)<<"                                    _orphanedGLBufferObjects.size()="<<_orphanedGLBufferObjects.size()<<std::endl;
#endif
}

void GLBufferObjectSet::remove(GLBufferObject* to)
{
    if (to->_previous!=0)
    {
        (to->_previous)->_next = to->_next;
    }
    else
    {
        // 'to' was head so assign _head to the next in list
        _head = to->_next;
    }

    if (to->_next!=0)
    {
        (to->_next)->_previous = to->_previous;
    }
    else
    {
        // 'to' was tail so assing tail to the previous in list
        _tail = to->_previous;
    }

    // reset the 'to' list pointers as it's no longer in the active list.
    to->_next = 0;
    to->_previous = 0;
}


void GLBufferObjectSet::moveToSet(GLBufferObject* to, GLBufferObjectSet* set)
{
    if (set==this) return;
    if (!set) return;

    // remove 'to' from original set
    --_numOfGLBufferObjects;
    remove(to);

    // register 'to' with new set.
    to->_set = set;
    ++set->_numOfGLBufferObjects;
    set->addToBack(to);
}



GLBufferObjectManager::GLBufferObjectManager(unsigned int contextID):
    _contextID(contextID),
    _numActiveGLBufferObjects(0),
    _numOrphanedGLBufferObjects(0),
    _currGLBufferObjectPoolSize(0),
    _maxGLBufferObjectPoolSize(0),
    _frameNumber(0),
    _numFrames(0),
    _numDeleted(0),
    _deleteTime(0.0),
    _numGenerated(0),
    _generateTime(0.0),
    _numApplied(0),
    _applyTime(0.0)
{
}

void GLBufferObjectManager::setMaxGLBufferObjectPoolSize(unsigned int size)
{
    if (_maxGLBufferObjectPoolSize == size) return;

    if (size<_currGLBufferObjectPoolSize)
    {
        osg::notify(osg::NOTICE)<<"Warning: new MaxGLBufferObjectPoolSize="<<size<<" is smaller than current GLBufferObjectPoolSize="<<_currGLBufferObjectPoolSize<<std::endl;
    }

    _maxGLBufferObjectPoolSize = size;
}

bool GLBufferObjectManager::makeSpace(unsigned int size)
{
    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end() && size>0;
        ++itr)
    {
        if ((*itr).second->makeSpace(size)) return true;
    }

    return size==0;
}


GLBufferObject* GLBufferObjectManager::generateGLBufferObject(const BufferObject* bufferObject)
{
    ElapsedTime elapsedTime(&(getGenerateTime()));
    ++getNumberGenerated();

    BufferObjectProfile profile(bufferObject->getTarget(), bufferObject->getUsage(), bufferObject->computeRequiredBufferSize());

    // osg::notify(osg::NOTICE)<<"GLBufferObjectManager::generateGLBufferObject size="<<bufferObject->computeRequiredBufferSize()<<std::endl;

    GLBufferObjectSet* glbos = getGLBufferObjectSet(profile);
    return glbos->takeOrGenerate(const_cast<BufferObject*>(bufferObject));
}

GLBufferObjectSet* GLBufferObjectManager::getGLBufferObjectSet(const BufferObjectProfile& profile)
{
    osg::ref_ptr<GLBufferObjectSet>& tos = _glBufferObjectSetMap[profile];
    if (!tos) tos = new GLBufferObjectSet(this, profile);
    return tos.get();
}

void GLBufferObjectManager::handlePendingOrphandedGLBufferObjects()
{
    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
        (*itr).second->handlePendingOrphandedGLBufferObjects();
    }
}

void GLBufferObjectManager::flushAllDeletedGLBufferObjects()
{
    ElapsedTime elapsedTime(&(getDeleteTime()));

    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
        (*itr).second->flushAllDeletedGLBufferObjects();
    }
}

void GLBufferObjectManager::discardAllDeletedGLBufferObjects()
{
    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
        (*itr).second->discardAllDeletedGLBufferObjects();
    }
}

void GLBufferObjectManager::flushDeletedGLBufferObjects(double currentTime, double& availableTime)
{
    ElapsedTime elapsedTime(&(getDeleteTime()));

    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        (itr != _glBufferObjectSetMap.end()) && (availableTime > 0.0);
        ++itr)
    {
        (*itr).second->flushDeletedGLBufferObjects(currentTime, availableTime);
    }
}

void GLBufferObjectManager::releaseGLBufferObject(GLBufferObject* to)
{
    if (to->_set) to->_set->orphan(to);
    else osg::notify(osg::NOTICE)<<"GLBufferObjectManager::releaseGLBufferObject(GLBufferObject* to) Not implemented yet"<<std::endl;
}


void GLBufferObjectManager::newFrame(osg::FrameStamp* fs)
{
    if (fs) _frameNumber = fs->getFrameNumber();
    else ++_frameNumber;

    ++_numFrames;
}

void GLBufferObjectManager::reportStats()
{
    double numFrames(_numFrames==0 ? 1.0 : _numFrames);
    osg::notify(osg::NOTICE)<<"GLBufferObjectMananger::reportStats()"<<std::endl;
    osg::notify(osg::NOTICE)<<"   total _numOfGLBufferObjects="<<_numActiveGLBufferObjects<<", _numOrphanedGLBufferObjects="<<_numOrphanedGLBufferObjects<<" _currGLBufferObjectPoolSize="<<_currGLBufferObjectPoolSize<<std::endl;
    osg::notify(osg::NOTICE)<<"   total _numGenerated="<<_numGenerated<<", _generateTime="<<_generateTime<<", averagePerFrame="<<_generateTime/numFrames*1000.0<<"ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"   total _numDeleted="<<_numDeleted<<", _deleteTime="<<_deleteTime<<", averagePerFrame="<<_deleteTime/numFrames*1000.0<<"ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"   total _numApplied="<<_numApplied<<", _applyTime="<<_applyTime<<", averagePerFrame="<<_applyTime/numFrames*1000.0<<"ms"<<std::endl;
}

void GLBufferObjectManager::resetStats()
{
    _numFrames = 0;
    _numDeleted = 0;
    _deleteTime = 0;

    _numGenerated = 0;
    _generateTime = 0;

    _numApplied = 0;
    _applyTime = 0;
}



osg::ref_ptr<GLBufferObjectManager>& GLBufferObjectManager::getGLBufferObjectManager(unsigned int contextID)
{
    typedef osg::buffered_object< ref_ptr<GLBufferObjectManager> > GLBufferObjectManagerBuffer;
    static GLBufferObjectManagerBuffer s_GLBufferObjectManager;
    if (!s_GLBufferObjectManager[contextID]) s_GLBufferObjectManager[contextID] = new GLBufferObjectManager(contextID);
    return s_GLBufferObjectManager[contextID];
}

GLBufferObject* GLBufferObject::createGLBufferObject(unsigned int contextID, const BufferObject* bufferObject)
{
    return GLBufferObjectManager::getGLBufferObjectManager(contextID)->generateGLBufferObject(bufferObject);
}

void GLBufferObject::flushAllDeletedBufferObjects(unsigned int contextID)
{
    GLBufferObjectManager::getGLBufferObjectManager(contextID)->flushAllDeletedGLBufferObjects();
}

void GLBufferObject::discardAllDeletedBufferObjects(unsigned int contextID)
{
    GLBufferObjectManager::getGLBufferObjectManager(contextID)->discardAllDeletedGLBufferObjects();
}

void GLBufferObject::flushDeletedBufferObjects(unsigned int contextID,double currentTime, double& availbleTime)
{
    GLBufferObjectManager::getGLBufferObjectManager(contextID)->flushDeletedGLBufferObjects(currentTime, availbleTime);
}

void GLBufferObject::releaseGLBufferObject(unsigned int contextID, GLBufferObject* to)
{
    GLBufferObjectManager::getGLBufferObjectManager(contextID)->releaseGLBufferObject(to);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BufferObject
//
BufferObject::BufferObject()
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
        unsigned int contextID = state->getContextID();
        if (_glBufferObjects[contextID].valid())
        {
            GLBufferObject::releaseGLBufferObject(contextID, _glBufferObjects[contextID].get());
            _glBufferObjects[contextID] = 0;
        }
    }
    else
    {
        for(unsigned int i=0; i<_glBufferObjects.size();++i)
        {
            if (_glBufferObjects[i].valid())
            {
                GLBufferObject::releaseGLBufferObject(i, _glBufferObjects[i].get());
                _glBufferObjects[i] = 0;
            }
        }
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

unsigned int BufferObject::computeRequiredBufferSize() const
{
    unsigned int newTotalSize = 0;
    for(BufferDataList::const_iterator itr = _bufferDataList.begin();
        itr != _bufferDataList.end();
        ++itr)
    {
        BufferData* bd = *itr;
        newTotalSize += bd->getTotalDataSize();
    }
    return newTotalSize;
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
    setTarget(GL_ARRAY_BUFFER_ARB);
    setUsage(GL_STATIC_DRAW_ARB);
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

//////////////////////////////////////////////////////////////////////////////////
//
//  ElementBufferObject
//
ElementBufferObject::ElementBufferObject()
{
    setTarget(GL_ELEMENT_ARRAY_BUFFER_ARB);
    setUsage(GL_STATIC_DRAW_ARB);
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


//////////////////////////////////////////////////////////////////////////////////
//
//  PixelBufferObject
//
PixelBufferObject::PixelBufferObject(osg::Image* image):
    BufferObject()
{
    setTarget(GL_PIXEL_UNPACK_BUFFER_ARB);
    setUsage(GL_STREAM_DRAW_ARB);

    osg::notify(osg::INFO)<<"Constructing PixelBufferObject for image="<<image<<std::endl;

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


//////////////////////////////////////////////////////////////////////////////////
//
//  PixelDataBufferObject
//
//--------------------------------------------------------------------------------
PixelDataBufferObject::PixelDataBufferObject()
{
    setTarget(GL_ARRAY_BUFFER_ARB);
    setUsage(GL_DYNAMIC_DRAW_ARB);
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
    if ( _profile._size == 0) return;

    GLBufferObject* bo = getOrCreateGLBufferObject(contextID);
    if (!bo || !bo->isDirty()) return;

    bo->_extensions->glBindBuffer(_profile._target, bo->getGLObjectID());
    bo->_extensions->glBufferData(_profile._target, _profile._size, NULL, _profile._usage);
    bo->_extensions->glBindBuffer(_profile._target, 0);
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
            extensions->glBindBuffer(_profile._target,0);
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
