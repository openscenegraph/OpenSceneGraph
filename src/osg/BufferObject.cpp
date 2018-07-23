/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/BufferObject>
#include <osg/Notify>
#include <osg/GLExtensions>
#include <osg/GLExtensions>
#include <osg/Timer>
#include <osg/Image>
#include <osg/State>
#include <osg/PrimitiveSet>
#include <osg/Array>
#include <osg/ContextData>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#if 0
    #define CHECK_CONSISTENCY checkConsistency();
#else
    #define CHECK_CONSISTENCY
#endif

using namespace osg;

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GLBufferObject::BufferEntry
//
unsigned int GLBufferObject::BufferEntry::getNumClients() const
{
    return dataSource->getNumClients();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GLBufferObject
//
GLBufferObject::GLBufferObject(unsigned int contextID, BufferObject* bufferObject, unsigned int glObjectID):
    _contextID(contextID),
    _glObjectID(glObjectID),
    _profile(0,0,0),
    _allocatedSize(0),
    _dirty(true),
    _bufferObject(0),
    _set(0),
    _previous(0),
    _next(0),
    _frameLastUsed(0),
    _extensions(0)
{
    assign(bufferObject);

    _extensions = GLExtensions::Get(contextID, true);

    if (glObjectID==0)
    {
        _extensions->glGenBuffers(1, &_glObjectID);
    }

    // OSG_NOTICE<<"Constructing BufferObject "<<this<<std::endl;
}

GLBufferObject::~GLBufferObject()
{
    //OSG_NOTICE<<"Destructing BufferObject "<<this<<std::endl;
}

void GLBufferObject::release()
{
    if (_set) _set->orphan(this);
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

    unsigned int bufferAlignment = 4;

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
                unsigned int previousEndOfBufferDataMarker = osg::computeBufferAlignment(entry.offset + entry.dataSize, bufferAlignment);

                // OSG_NOTICE<<"GLBufferObject::compileBuffer(..) updating BufferEntry"<<std::endl;

                entry.numRead = 0;
                entry.modifiedCount = 0xffffff;
                entry.offset = newTotalSize;
                entry.dataSize = bd->getTotalDataSize();
                entry.dataSource = bd;

                newTotalSize += entry.dataSize;
                if (previousEndOfBufferDataMarker!=newTotalSize)
                {
                    offsetChanged = true;
                }
            }
            else
            {
                newTotalSize = osg::computeBufferAlignment(newTotalSize + entry.dataSize, bufferAlignment);
            }
        }
        else
        {
            BufferEntry entry;
            entry.offset = newTotalSize;
            entry.modifiedCount = 0xffffff;
            entry.dataSize = bd ? bd->getTotalDataSize() : 0;
            entry.dataSource = bd;
#if 0
            OSG_NOTICE<<"entry"<<std::endl;
            OSG_NOTICE<<"   offset "<<entry.offset<<std::endl;
            OSG_NOTICE<<"   dataSize "<<entry.dataSize<<std::endl;
            OSG_NOTICE<<"   dataSource "<<entry.dataSource<<std::endl;
            OSG_NOTICE<<"   modifiedCount "<<entry.modifiedCount<<std::endl;
#endif
            newTotalSize = computeBufferAlignment(newTotalSize + entry.dataSize, bufferAlignment);

            _bufferEntries.push_back(entry);
        }
    }


    if (i<_bufferEntries.size())
    {
        // triming end of bufferEntries as the source data is has less entries than the originally held.
        _bufferEntries.erase(_bufferEntries.begin()+i, _bufferEntries.end());
    }

    _extensions->glBindBuffer(_profile._target, _glObjectID);

    _extensions->debugObjectLabel(GL_BUFFER, _glObjectID, _bufferObject->getName());

    if (newTotalSize > _profile._size)
    {
        OSG_INFO<<"newTotalSize="<<newTotalSize<<", _profile._size="<<_profile._size<<std::endl;

        unsigned int sizeDifference = newTotalSize - _profile._size;
        _profile._size = newTotalSize;

        if (_set)
        {
            _set->moveToSet(this, _set->getParent()->getGLBufferObjectSet(_profile));
            _set->getParent()->getCurrGLBufferObjectPoolSize() += sizeDifference;
        }

    }

    if (_allocatedSize != _profile._size)
    {
        _allocatedSize = _profile._size;
        OSG_INFO<<"    Allocating new glBufferData(), _allocatedSize="<<_allocatedSize<<std::endl;
        _extensions->glBufferData(_profile._target, _profile._size, NULL, _profile._usage);
        compileAll = true;
    }

    for(BufferEntries::iterator itr = _bufferEntries.begin();
        itr != _bufferEntries.end();
        ++itr)
    {
        BufferEntry& entry = *itr;
        if (entry.dataSource && (compileAll || entry.modifiedCount != entry.dataSource->getModifiedCount()))
        {
            // OSG_NOTICE<<"GLBufferObject::compileBuffer(..) downloading BufferEntry "<<&entry<<std::endl;
            entry.numRead = 0;
            entry.modifiedCount = entry.dataSource->getModifiedCount();

            const osg::Image* image = entry.dataSource->asImage();
            if (image && !(image->isDataContiguous()))
            {
                unsigned int offset = entry.offset;
                for(osg::Image::DataIterator img_itr(image); img_itr.valid(); ++img_itr)
                {
                    _extensions->glBufferSubData(_profile._target, (GLintptr)offset, (GLsizeiptr)img_itr.size(), img_itr.data());
                    offset += img_itr.size();
                }
            }
            else
            {
                _extensions->glBufferSubData(_profile._target, (GLintptr)entry.offset, (GLsizeiptr)entry.dataSize, entry.dataSource->getDataPointer());
            }
        }
    }
}

void GLBufferObject::deleteGLObject()
{
    OSG_DEBUG<<"GLBufferObject::deleteGLObject() "<<_glObjectID<<std::endl;
    if (_glObjectID!=0)
    {
        _extensions->glDeleteBuffers(1, &_glObjectID);
        _glObjectID = 0;

        _allocatedSize = 0;
        _bufferEntries.clear();
    }
}

bool GLBufferObject::hasAllBufferDataBeenRead() const
{
    for (BufferEntries::const_iterator it=_bufferEntries.begin(); it!=_bufferEntries.end(); ++it)
    {
        if (it->numRead < it->getNumClients())
            return false;
    }

    return true;
}

void GLBufferObject::setBufferDataHasBeenRead(const osg::BufferData* bd)
{
    BufferEntry &entry = _bufferEntries[bd->getBufferIndex()];
    ++entry.numRead;
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
    OSG_INFO<<"GLBufferObjectSet::GLBufferObjectSet _profile._size="<<_profile._size<<std::endl;
}

GLBufferObjectSet::~GLBufferObjectSet()
{
#if 0
    OSG_NOTICE<<"GLBufferObjectSet::~GLBufferObjectSet(), _numOfGLBufferObjects="<<_numOfGLBufferObjects<<std::endl;
    OSG_NOTICE<<"     _orphanedGLBufferObjects = "<<_orphanedGLBufferObjects.size()<<std::endl;
    OSG_NOTICE<<"     _head = "<<_head<<std::endl;
    OSG_NOTICE<<"     _tail = "<<_tail<<std::endl;
#endif
}

bool GLBufferObjectSet::checkConsistency() const
{
    OSG_NOTICE<<"GLBufferObjectSet::checkConsistency()"<<std::endl;
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
                OSG_NOTICE<<"GLBufferObjectSet::checkConsistency() : Error (to->_next)->_previous != to "<<std::endl;
                return false;
            }
        }
        else
        {
            if (_tail != to)
            {
                OSG_NOTICE<<"GLBufferObjectSet::checkConsistency() : Error _trail != to"<<std::endl;
                return false;
            }
        }

        to = to->_next;
    }

    unsigned int totalNumber = numInList + _orphanedGLBufferObjects.size();
    if (totalNumber != _numOfGLBufferObjects)
    {
        OSG_NOTICE<<"Error numInList + _orphanedGLBufferObjects.size() != _numOfGLBufferObjects"<<std::endl;
        OSG_NOTICE<<"    numInList = "<<numInList<<std::endl;
        OSG_NOTICE<<"    _orphanedGLBufferObjects.size() = "<<_orphanedGLBufferObjects.size()<<std::endl;
        OSG_NOTICE<<"    _pendingOrphanedGLBufferObjects.size() = "<<_pendingOrphanedGLBufferObjects.size()<<std::endl;
        OSG_NOTICE<<"    _numOfGLBufferObjects = "<<_numOfGLBufferObjects<<std::endl;
        return false;
    }

    return true;
}

void GLBufferObjectSet::handlePendingOrphandedGLBufferObjects()
{
//    OSG_NOTICE<<"handlePendingOrphandedGLBufferObjects()"<<_pendingOrphanedGLBufferObjects.size()<<std::endl;

    if (_pendingOrphanedGLBufferObjects.empty()) return;

    unsigned int numOrphaned = _pendingOrphanedGLBufferObjects.size();

    for(GLBufferObjectList::iterator itr = _pendingOrphanedGLBufferObjects.begin();
        itr != _pendingOrphanedGLBufferObjects.end();
        ++itr)
    {
        GLBufferObject* to = itr->get();

        _orphanedGLBufferObjects.push_back(to);

        remove(to);
    }


    // update the GLBufferObjectManager's running total of active + orphaned GLBufferObjects
    _parent->getNumberOrphanedGLBufferObjects() += numOrphaned;
    _parent->getNumberActiveGLBufferObjects() -= numOrphaned;

    _pendingOrphanedGLBufferObjects.clear();

    CHECK_CONSISTENCY
}

void GLBufferObjectSet::deleteAllGLBufferObjects()
{
    // OSG_NOTICE<<"GLBufferObjectSet::deleteAllGLBufferObjects()"<<std::endl;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_pendingOrphanedGLBufferObjects.empty())
        {
            // OSG_NOTICE<<"GLBufferObjectSet::flushDeletedGLBufferObjects(..) handling orphans"<<std::endl;
            handlePendingOrphandedGLBufferObjects();
        }
    }

    CHECK_CONSISTENCY

    unsigned int numOrphaned = 0;
    GLBufferObject* to = _head;
    while(to!=0)
    {
        ref_ptr<GLBufferObject> glbo = to;

        to = to->_next;

        _orphanedGLBufferObjects.push_back(glbo.get());

        remove(glbo.get());

        ++numOrphaned;

        ref_ptr<BufferObject> original_BufferObject = glbo->getBufferObject();
        if (original_BufferObject.valid())
        {
            // detect the GLBufferObject from the BufferObject
            original_BufferObject->setGLBufferObject(_contextID,0);
        }
    }

    _parent->getNumberOrphanedGLBufferObjects() += numOrphaned;
    _parent->getNumberActiveGLBufferObjects() -= numOrphaned;

    // do the actual delete.
    flushAllDeletedGLBufferObjects();

    // OSG_NOTICE<<"done GLBufferObjectSet::deleteAllGLBufferObjects()"<<std::endl;
}

void GLBufferObjectSet::discardAllGLBufferObjects()
{
    // OSG_NOTICE<<"GLBufferObjectSet::discardAllGLBufferObjects()"<<std::endl;

    GLBufferObject* to = _head;
    while(to!=0)
    {
        ref_ptr<GLBufferObject> glbo = to;

        to = to->_next;

        ref_ptr<BufferObject> original_BufferObject = glbo->getBufferObject();
        if (original_BufferObject.valid())
        {
            // detect the GLBufferObject from the BufferObject
            original_BufferObject->setGLBufferObject(_contextID,0);
        }
    }

    // the linked list should now be empty
    _head = 0;
    _tail = 0;

    _pendingOrphanedGLBufferObjects.clear();
    _orphanedGLBufferObjects.clear();

    unsigned int numDeleted = _numOfGLBufferObjects;
    _numOfGLBufferObjects = 0;

    // update the GLBufferObjectManager's running total of current pool size
    _parent->getCurrGLBufferObjectPoolSize() -= numDeleted*_profile._size;
    _parent->getNumberOrphanedGLBufferObjects() -= numDeleted;
    _parent->getNumberDeleted() += numDeleted;
}

void GLBufferObjectSet::flushAllDeletedGLBufferObjects()
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_pendingOrphanedGLBufferObjects.empty())
        {
            // OSG_NOTICE<<"GLBufferObjectSet::flushDeletedGLBufferObjects(..) handling orphans"<<std::endl;
            handlePendingOrphandedGLBufferObjects();
        }
    }

    for(GLBufferObjectList::iterator itr = _orphanedGLBufferObjects.begin();
        itr != _orphanedGLBufferObjects.end();
        ++itr)
    {
        (*itr)->deleteGLObject();
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
    // OSG_NOTICE<<"GLBufferObjectSet::discardAllDeletedGLBufferObjects()"<<std::endl;

    // clean up the pending orphans.
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_pendingOrphanedGLBufferObjects.empty())
        {
            // OSG_NOTICE<<"GLBufferObjectSet::flushDeletedGLBufferObjects(..) handling orphans"<<std::endl;
            handlePendingOrphandedGLBufferObjects();
        }
    }

    unsigned int numDiscarded = _orphanedGLBufferObjects.size();

    _numOfGLBufferObjects -= numDiscarded;

    // update the GLBufferObjectManager's running total of current pool size
    _parent->setCurrGLBufferObjectPoolSize( _parent->getCurrGLBufferObjectPoolSize() - numDiscarded*_profile._size );

    // update the number of active and orphaned GLBufferObjects
    _parent->getNumberOrphanedGLBufferObjects() -= numDiscarded;
    _parent->getNumberDeleted() += numDiscarded;


    // just clear the list as there is nothing else we can do with them when discarding them
    _orphanedGLBufferObjects.clear();
}

void GLBufferObjectSet::flushDeletedGLBufferObjects(double /*currentTime*/, double& availableTime)
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_pendingOrphanedGLBufferObjects.empty())
        {
            // OSG_NOTICE<<"GLBufferObjectSet::flushDeletedGLBufferObjects(..) handling orphans"<<std::endl;
            handlePendingOrphandedGLBufferObjects();
        }
    }

    if (_parent->getCurrGLBufferObjectPoolSize()<=_parent->getMaxGLBufferObjectPoolSize())
    {
        OSG_INFO<<"Plenty of space in GLBufferObject pool"<<std::endl;
        return;
    }

    // if nothing to delete return
    if (_orphanedGLBufferObjects.empty()) return;

    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    unsigned int numDeleted = 0;
    unsigned int sizeRequired = _parent->getCurrGLBufferObjectPoolSize() - _parent->getMaxGLBufferObjectPoolSize();
    unsigned int maxNumObjectsToDelete = static_cast<unsigned int>(ceil(double(sizeRequired) / double(_profile._size)));
    OSG_INFO<<"_parent->getCurrGLBufferObjectPoolSize()="<<_parent->getCurrGLBufferObjectPoolSize() <<" _parent->getMaxGLBufferObjectPoolSize()="<< _parent->getMaxGLBufferObjectPoolSize()<<std::endl;
    OSG_INFO<<"Looking to reclaim "<<sizeRequired<<", going to look to remove "<<maxNumObjectsToDelete<<" from "<<_orphanedGLBufferObjects.size()<<" orphans"<<std::endl;

    ElapsedTime timer;

    GLBufferObjectList::iterator itr = _orphanedGLBufferObjects.begin();
    for(;
        itr != _orphanedGLBufferObjects.end() && timer.elapsedTime()<availableTime && numDeleted<maxNumObjectsToDelete;
        ++itr)
    {

         (*itr)->deleteGLObject();

        ++numDeleted;
    }

    // OSG_NOTICE<<"Size before = "<<_orphanedGLBufferObjects.size();
    _orphanedGLBufferObjects.erase(_orphanedGLBufferObjects.begin(), itr);
    // OSG_NOTICE<<", after = "<<_orphanedGLBufferObjects.size()<<" numDeleted = "<<numDeleted<<std::endl;

    // update the number of TO's in this GLBufferObjectSet
    _numOfGLBufferObjects -= numDeleted;

    _parent->setCurrGLBufferObjectPoolSize( _parent->getCurrGLBufferObjectPoolSize() - numDeleted*_profile._size );

    // update the number of active and orphaned TextureObjects
    _parent->getNumberOrphanedGLBufferObjects() -= numDeleted;
    _parent->getNumberDeleted() += numDeleted;

    availableTime -= timer.elapsedTime();
}

bool GLBufferObjectSet::makeSpace(unsigned int& size)
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_pendingOrphanedGLBufferObjects.empty())
        {
            // OSG_NOTICE<<"GLBufferSet::::makeSpace(..) handling orphans"<<std::endl;
            handlePendingOrphandedGLBufferObjects();
        }
    }

    if (!_orphanedGLBufferObjects.empty())
    {
        unsigned int sizeAvailable = _orphanedGLBufferObjects.size() * _profile._size;
        if (size>sizeAvailable) size -= sizeAvailable;
        else size = 0;

        flushAllDeletedGLBufferObjects();
    }

    return size==0;
}

osg::ref_ptr<GLBufferObject> GLBufferObjectSet::takeFromOrphans(BufferObject* bufferObject)
{
    // take front of orphaned list.
    ref_ptr<GLBufferObject> glbo = _orphanedGLBufferObjects.front();

    // remove from orphan list.
    _orphanedGLBufferObjects.pop_front();

    // assign to new GLBufferObject
    glbo->assign(bufferObject);
    glbo->setProfile(_profile);

    // update the number of active and orphaned GLBufferObjects
    _parent->getNumberOrphanedGLBufferObjects() -= 1;
    _parent->getNumberActiveGLBufferObjects() += 1;

    // place at back of active list
    addToBack(glbo.get());

    //OSG_NOTICE<<"Reusing orphaned GLBufferObject, _numOfGLBufferObjects="<<_numOfGLBufferObjects<<" target="<<std::hex<<_profile._target<<std::dec<<std::endl;

    return glbo;
}


osg::ref_ptr<GLBufferObject> GLBufferObjectSet::takeOrGenerate(BufferObject* bufferObject)
{
    // see if we can recycle GLBufferObject from the orphan list
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_pendingOrphanedGLBufferObjects.empty())
        {
            handlePendingOrphandedGLBufferObjects();
            return takeFromOrphans(bufferObject);
        }
    }

    if (!_orphanedGLBufferObjects.empty())
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
            OSG_INFO<<"GLBufferObjectSet="<<this<<": Reusing an active GLBufferObject "<<glbo.get()<<" _numOfGLBufferObjects="<<_numOfGLBufferObjects<<" size="<<_profile._size<<std::endl;
        }
        else
        {
            OSG_INFO<<"Reusing a recently orphaned active GLBufferObject "<<glbo.get()<<std::endl;
        }

        moveToBack(glbo.get());

        // assign
        glbo->setBufferObject(bufferObject);
        glbo->setProfile(_profile);

        return glbo;
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

    // OSG_NOTICE<<"Created new GLBufferObject, _numOfGLBufferObjects "<<_numOfGLBufferObjects<<std::endl;

    return glbo;
}

void GLBufferObjectSet::moveToBack(GLBufferObject* to)
{
#if 0
    OSG_NOTICE<<"GLBufferObjectSet::moveToBack("<<to<<")"<<std::endl;
    OSG_NOTICE<<"    before _head = "<<_head<<std::endl;
    OSG_NOTICE<<"    before _tail = "<<_tail<<std::endl;
    OSG_NOTICE<<"    before to->_previous = "<<to->_previous<<std::endl;
    OSG_NOTICE<<"    before to->_next = "<<to->_next<<std::endl;
#endif

    to->_frameLastUsed = _parent->getFrameNumber();

    // nothing to do if we are already tail
    if (to==_tail) return;

    // if no tail exists then assign 'to' as tail and head
    if (_tail==0)
    {
        OSG_NOTICE<<"Error ***************** Should not get here !!!!!!!!!"<<std::endl;
        _head = to;
        _tail = to;
        return;
    }

    if (to->_next==0)
    {
        OSG_NOTICE<<"Error ***************** Should not get here either !!!!!!!!!"<<std::endl;
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
    OSG_NOTICE<<"  m2B   after  _head = "<<_head<<std::endl;
    OSG_NOTICE<<"  m2B   after _tail = "<<_tail<<std::endl;
    OSG_NOTICE<<"  m2B   after to->_previous = "<<to->_previous<<std::endl;
    OSG_NOTICE<<"  m2B   after to->_next = "<<to->_next<<std::endl;
#endif
    CHECK_CONSISTENCY
}

void GLBufferObjectSet::addToBack(GLBufferObject* to)
{
#if 0
    OSG_NOTICE<<"GLBufferObjectSet::addToBack("<<to<<")"<<std::endl;
    OSG_NOTICE<<"    before _head = "<<_head<<std::endl;
    OSG_NOTICE<<"    before _tail = "<<_tail<<std::endl;
    OSG_NOTICE<<"    before to->_previous = "<<to->_previous<<std::endl;
    OSG_NOTICE<<"    before to->_next = "<<to->_next<<std::endl;
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
    OSG_NOTICE<<"  a2B  after  _head = "<<_head<<std::endl;
    OSG_NOTICE<<"  a2B   after _tail = "<<_tail<<std::endl;
    OSG_NOTICE<<"  a2B   after to->_previous = "<<to->_previous<<std::endl;
    OSG_NOTICE<<"  a2B   after to->_next = "<<to->_next<<std::endl;
#endif
    CHECK_CONSISTENCY
}

void GLBufferObjectSet::orphan(GLBufferObject* to)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    // disconnect from original texture
    to->setBufferObject(0);

    // add orphan 'to' to the pending list of orphans, these will then be
    // handled in the handlePendingOrphandedGLBufferObjects() where the TO's
    // will be removed from the active list, and then placed in the orhpanGLBufferObject
    // list.  This double buffered approach to handling orphaned TO's is used
    // to avoid having to mutex the process of appling active TO's.
    _pendingOrphanedGLBufferObjects.push_back(to);
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

unsigned int GLBufferObjectSet::computeNumGLBufferObjectsInList() const
{
    unsigned int num=0;
    GLBufferObject* obj = _head;
    while(obj!=NULL)
    {
        ++num;
        obj = obj->_next;
    }
    return num;
}


GLBufferObjectManager::GLBufferObjectManager(unsigned int contextID):
    GraphicsObjectManager("GLBufferObjectManager", contextID),
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

GLBufferObjectManager::~GLBufferObjectManager()
{
}

void GLBufferObjectManager::setMaxGLBufferObjectPoolSize(unsigned int size)
{
    if (_maxGLBufferObjectPoolSize == size) return;

    if (size<_currGLBufferObjectPoolSize)
    {
        OSG_NOTICE<<"Warning: new MaxGLBufferObjectPoolSize="<<size<<" is smaller than current GLBufferObjectPoolSize="<<_currGLBufferObjectPoolSize<<std::endl;
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


osg::ref_ptr<GLBufferObject> GLBufferObjectManager::generateGLBufferObject(const BufferObject* bufferObject)
{
    ElapsedTime elapsedTime(&(getGenerateTime()));
    ++getNumberGenerated();

    unsigned int requiredBufferSize = osg::maximum(bufferObject->computeRequiredBufferSize(), bufferObject->getProfile()._size);

    BufferObjectProfile profile(bufferObject->getTarget(), bufferObject->getUsage(), requiredBufferSize);

    // OSG_NOTICE<<"GLBufferObjectManager::generateGLBufferObject size="<<requiredBufferSize<<std::endl;

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

void GLBufferObjectManager::deleteAllGLObjects()
{
    ElapsedTime elapsedTime(&(getDeleteTime()));

    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
        (*itr).second->deleteAllGLBufferObjects();
    }
}

void GLBufferObjectManager::discardAllGLObjects()
{
    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
        (*itr).second->discardAllGLBufferObjects();
    }
}

void GLBufferObjectManager::flushAllDeletedGLObjects()
{
    ElapsedTime elapsedTime(&(getDeleteTime()));

    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
        (*itr).second->flushAllDeletedGLBufferObjects();
    }
}

void GLBufferObjectManager::discardAllDeletedGLObjects()
{
    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
        (*itr).second->discardAllDeletedGLBufferObjects();
    }
}

void GLBufferObjectManager::flushDeletedGLObjects(double currentTime, double& availableTime)
{
    ElapsedTime elapsedTime(&(getDeleteTime()));

    for(GLBufferObjectSetMap::iterator itr = _glBufferObjectSetMap.begin();
        (itr != _glBufferObjectSetMap.end()) && (availableTime > 0.0);
        ++itr)
    {
        (*itr).second->flushDeletedGLBufferObjects(currentTime, availableTime);
    }
}

void GLBufferObjectManager::newFrame(osg::FrameStamp* fs)
{
    if (fs) _frameNumber = fs->getFrameNumber();
    else ++_frameNumber;

    ++_numFrames;
}

void GLBufferObjectManager::reportStats(std::ostream& out)
{
    double numFrames(_numFrames==0 ? 1.0 : _numFrames);
    out<<"GLBufferObjectMananger::reportStats()"<<std::endl;
    out<<"   total _numOfGLBufferObjects="<<_numActiveGLBufferObjects<<", _numOrphanedGLBufferObjects="<<_numOrphanedGLBufferObjects<<" _currGLBufferObjectPoolSize="<<_currGLBufferObjectPoolSize<<std::endl;
    out<<"   total _numGenerated="<<_numGenerated<<", _generateTime="<<_generateTime<<", averagePerFrame="<<_generateTime/numFrames*1000.0<<"ms"<<std::endl;
    out<<"   total _numDeleted="<<_numDeleted<<", _deleteTime="<<_deleteTime<<", averagePerFrame="<<_deleteTime/numFrames*1000.0<<"ms"<<std::endl;
    out<<"   total _numApplied="<<_numApplied<<", _applyTime="<<_applyTime<<", averagePerFrame="<<_applyTime/numFrames*1000.0<<"ms"<<std::endl;
    out<<"   getMaxGLBufferObjectPoolSize()="<<getMaxGLBufferObjectPoolSize()<<" current/max size = "<<double(_currGLBufferObjectPoolSize)/double(getMaxGLBufferObjectPoolSize())<<std::endl;;

    recomputeStats(out);

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

void GLBufferObjectManager::recomputeStats(std::ostream& out) const
{
    out<<"GLBufferObjectMananger::recomputeStats()"<<std::endl;
    unsigned int numObjectsInLists = 0;
    unsigned int numActive = 0;
    unsigned int numOrphans = 0;
    unsigned int numPendingOrphans = 0;
    unsigned int currentSize = 0;
    for(GLBufferObjectSetMap::const_iterator itr = _glBufferObjectSetMap.begin();
        itr != _glBufferObjectSetMap.end();
        ++itr)
    {
         const GLBufferObjectSet* os = itr->second.get();
         numObjectsInLists += os->computeNumGLBufferObjectsInList();
         numActive += os->getNumOfGLBufferObjects();
         numOrphans += os->getNumOrphans();
         numPendingOrphans += os->getNumPendingOrphans();
         currentSize += os->getProfile()._size * (os->computeNumGLBufferObjectsInList()+os->getNumOrphans());
         out<<"   size="<<os->getProfile()._size
           <<", os->computeNumGLBufferObjectsInList()"<<os->computeNumGLBufferObjectsInList()
           <<", os->getNumOfGLBufferObjects()"<<os->getNumOfGLBufferObjects()
           <<", os->getNumOrphans()"<<os->getNumOrphans()
           <<", os->getNumPendingOrphans()"<<os->getNumPendingOrphans()
           <<std::endl;
    }
    out<<"   numObjectsInLists="<<numObjectsInLists<<", numActive="<<numActive<<", numOrphans="<<numOrphans<<" currentSize="<<currentSize<<std::endl;
    out<<"   getMaxGLBufferObjectPoolSize()="<<getMaxGLBufferObjectPoolSize()<<" current/max size = "<<double(currentSize)/double(getMaxGLBufferObjectPoolSize())<<std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BufferObject
//
BufferObject::BufferObject():
    _copyDataAndReleaseGLBufferObject(false)
{
}

BufferObject::BufferObject(const BufferObject& bo,const CopyOp& copyop):
    Object(bo,copyop),
    _copyDataAndReleaseGLBufferObject(bo._copyDataAndReleaseGLBufferObject)
{
}

BufferObject::~BufferObject()
{
    releaseGLObjects(0);
}

GLBufferObject* BufferObject::getOrCreateGLBufferObject(unsigned int contextID) const
{
    if (!_glBufferObjects[contextID]) _glBufferObjects[contextID] = osg::get<GLBufferObjectManager>(contextID)->generateGLBufferObject(this);

    // OSG_NOTICE<<"BufferObject::getOrCreateGLBufferObject() _glBufferObjects[contextID]->getProfile()._size() = "<<_glBufferObjects[contextID]->getProfile()._size<<std::endl;

    return _glBufferObjects[contextID].get();
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
        if (_glBufferObjects[i].valid())
        {
            _glBufferObjects[i]->dirty();
        }
    }
}

void BufferObject::resizeGLObjectBuffers(unsigned int maxSize)
{
    _glBufferObjects.resize(maxSize);
}

void BufferObject::releaseGLObjects(State* state) const
{
    OSG_DEBUG<<"BufferObject::releaseGLObjects("<<state<<")"<<std::endl;
    if (state)
    {
        unsigned int contextID = state->getContextID();
        if (_glBufferObjects[contextID].valid())
        {
            _glBufferObjects[contextID]->release();
            _glBufferObjects[contextID] = 0;
        }
    }
    else
    {
        for(unsigned int i=0; i<_glBufferObjects.size();++i)
        {
            if (_glBufferObjects[i].valid())
            {
                // OSG_NOTICE<<"  GLBufferObject::releaseGLBufferObject("<<i<<", _glBufferObjects["<<i<<"]="<<_glBufferObjects[i].get()<<")"<<std::endl;
                _glBufferObjects[i]->release();
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

    dirty();

//OSG_NOTICE<<"BufferObject "<<this<<":"<<className()<<"::addBufferData("<<bd<<"), bufferIndex= "<<_bufferDataList.size()-1<<std::endl;

    return _bufferDataList.size()-1;
}

void BufferObject::removeBufferData(unsigned int index)
{
    if (index>=_bufferDataList.size())
    {
        OSG_WARN<<"Error "<<className()<<"::removeBufferData("<<index<<") out of range."<<std::endl;
        return;
    }

    //OSG_NOTICE<<"BufferObject::"<<this<<":"<<className()<<"::removeBufferData("<<index<<"), size= "<<_bufferDataList.size()<<std::endl;

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
    //OSG_NOTICE<<"BufferObject::"<<this<<":"<<className()<<"::removeBufferData("<<bd<<"), index="<<bd->getBufferIndex()<<" size= "<<_bufferDataList.size()<<std::endl;

    if (!bd || bd->getBufferObject()!=this) return;

    removeBufferData(bd->getBufferIndex());
}

unsigned int BufferObject::computeRequiredBufferSize() const
{
    unsigned int bufferAlignment = 4;
    unsigned int newTotalSize = 0;
    for(BufferDataList::const_iterator itr = _bufferDataList.begin();
        itr != _bufferDataList.end();
        ++itr)
    {
        BufferData* bd = *itr;
        if (bd) newTotalSize = osg::computeBufferAlignment(newTotalSize + bd->getTotalDataSize(), bufferAlignment);
        else
        {
            OSG_NOTICE<<"BufferObject::"<<this<<":"<<className()<<"::BufferObject::computeRequiredBufferSize() error, BufferData is 0x0"<<std::endl;
        }
    }
    //OSG_NOTICE<<"BufferObject::"<<this<<":"<<className()<<"::BufferObject::computeRequiredBufferSize() size="<<newTotalSize<<std::endl;
    return newTotalSize;
}

void BufferObject::deleteBufferObject(unsigned int contextID,GLuint globj)
{
    // implement deleteBufferObject for backwards compatibility by adding
    // a GLBufferObject for the globj id to BufferObjectManager/Set for the specified context.

    GLBufferObjectManager* bufferObjectManager = osg::get<GLBufferObjectManager>(contextID);
    if (!bufferObjectManager)
    {
        OSG_NOTICE<<"Warning::BufferObject::deleteBufferObject("<<contextID<<", "<<globj<<") unable to get GLBufferObjectManager for context."<<std::endl;
        return;
    }
    osg::ref_ptr<GLBufferObject> glBufferObject = new GLBufferObject(contextID, 0, globj);

    GLBufferObjectSet* bufferObjectSet = bufferObjectManager->getGLBufferObjectSet(glBufferObject->getProfile());
    if (!bufferObjectSet)
    {
        OSG_NOTICE<<"Warning::BufferObject::deleteBufferObject("<<contextID<<", "<<globj<<") unable to get GLBufferObjectSet for context."<<std::endl;
        return;
    }

    // do the adding of the wrapper buffer object.
    bufferObjectSet->orphan(glBufferObject.get());
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

void BufferData::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_bufferObject.valid())
    {
        _bufferObject->resizeGLObjectBuffers(maxSize);
    }
}

void BufferData::releaseGLObjects(State* state) const
{
    OSG_DEBUG<<"BufferData::releaseGLObjects("<<state<<")"<<std::endl;
    if (_bufferObject.valid())
    {
        _bufferObject->releaseGLObjects(state);
    }
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
//  DrawIndirectBufferObject
//
DrawIndirectBufferObject::DrawIndirectBufferObject()
{
    setTarget(GL_DRAW_INDIRECT_BUFFER);
    setUsage(GL_STATIC_DRAW_ARB);
//    setUsage(GL_STREAM_DRAW_ARB);
}

DrawIndirectBufferObject::DrawIndirectBufferObject(const DrawIndirectBufferObject& vbo,const CopyOp& copyop):
    BufferObject(vbo,copyop)
{
}

DrawIndirectBufferObject::~DrawIndirectBufferObject()
{
}

unsigned int DrawIndirectBufferObject::addArray(osg::Array* array)
{
    return addBufferData(array);
}

void DrawIndirectBufferObject::removeArray(osg::Array* array)
{
    removeBufferData(array);
}

void DrawIndirectBufferObject::setArray(unsigned int i, Array* array)
{
    setBufferData(i,array);
}

Array* DrawIndirectBufferObject::getArray(unsigned int i)
{
    return dynamic_cast<osg::Array*>(getBufferData(i));
}

const Array* DrawIndirectBufferObject::getArray(unsigned int i) const
{
    return dynamic_cast<const osg::Array*>(getBufferData(i));
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

    OSG_DEBUG<<"Constructing PixelBufferObject for image="<<image<<std::endl;

    if (image) setBufferData(0, image);
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
    GLExtensions* extensions = GLExtensions::Get(contextID, true);

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


//////////////////////////////////////////////////////////////////////////////////
//
//  UniformBufferObject
//
UniformBufferObject::UniformBufferObject()
{
    setTarget(GL_UNIFORM_BUFFER);
    setUsage(GL_STREAM_DRAW_ARB);
}

UniformBufferObject::UniformBufferObject(const UniformBufferObject& ubo, const CopyOp& copyop)
    : BufferObject(ubo, copyop)
{
}

UniformBufferObject::~UniformBufferObject()
{
}



//////////////////////////////////////////////////////////////////////////////////
//
//  AtomicCounterBufferObject
//
AtomicCounterBufferObject::AtomicCounterBufferObject()
{
    setTarget(GL_ATOMIC_COUNTER_BUFFER);
    setUsage(GL_DYNAMIC_DRAW);
}

AtomicCounterBufferObject::AtomicCounterBufferObject(const AtomicCounterBufferObject& ubo, const CopyOp& copyop)
    : BufferObject(ubo, copyop)
{
}

AtomicCounterBufferObject::~AtomicCounterBufferObject()
{
}


//////////////////////////////////////////////////////////////////////////////////
//
//  ShaderStorageBufferObject
//
ShaderStorageBufferObject::ShaderStorageBufferObject()
{
    setTarget(GL_SHADER_STORAGE_BUFFER);
    setUsage(GL_STATIC_DRAW);
}

ShaderStorageBufferObject::ShaderStorageBufferObject(const ShaderStorageBufferObject& ubo, const CopyOp& copyop)
    : BufferObject(ubo, copyop)
{
}

ShaderStorageBufferObject::~ShaderStorageBufferObject()
{
}


