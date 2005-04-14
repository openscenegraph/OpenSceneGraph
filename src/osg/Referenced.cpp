/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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
#include <osg/Referenced>
#include <osg/Notify>

#include <typeinfo>
#include <memory>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#ifndef OSG_JAVA_BUILD       

namespace osg
{

static bool s_useThreadSafeReferenceCounting = getenv("OSG_THREAD_SAFE_REF_UNREF")!=0;
static std::auto_ptr<DeleteHandler> s_deleteHandler(0);

void Referenced::setThreadSafeReferenceCounting(bool enableThreadSafeReferenceCounting)
{
    s_useThreadSafeReferenceCounting = enableThreadSafeReferenceCounting;
}

bool Referenced::getThreadSafeReferenceCounting()
{
    return s_useThreadSafeReferenceCounting;
}


void Referenced::setDeleteHandler(DeleteHandler* handler)
{
    s_deleteHandler.reset(handler);
}

DeleteHandler* Referenced::getDeleteHandler()
{
    return s_deleteHandler.get();
}

Referenced::Referenced()
{
    if (s_useThreadSafeReferenceCounting) _refMutex = new OpenThreads::Mutex;
    else _refMutex = 0;
   _refCount=0;
}

Referenced::Referenced(const Referenced&)
{
    if (s_useThreadSafeReferenceCounting) _refMutex = new OpenThreads::Mutex;
    else _refMutex = 0;
    _refCount=0;
}

Referenced::~Referenced()
{
    if (_refCount>0)
    {
        notify(WARN)<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
        notify(WARN)<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
    }

    if (_refMutex)
    {
        OpenThreads::Mutex* tmpMutexPtr = _refMutex;
        _refMutex = 0;
        delete tmpMutexPtr;
    }
}

void Referenced::setThreadSafeRefUnref(bool threadSafe)
{
    if (threadSafe)
    {
        if (!_refMutex)
        {
            // we want thread safe ref()/unref() so assing a mutex
            _refMutex = new OpenThreads::Mutex;
        }
    }
    else
    {
        if (_refMutex)
        {
            // we don't want thread safe ref()/unref() so remove any assigned mutex
            OpenThreads::Mutex* tmpMutexPtr = _refMutex;
            _refMutex = 0;
            delete tmpMutexPtr;
        }
    }
}

/*
void Referenced::ref() const
{ 
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex); 
        ++_refCount;
    }
    else
    {
        ++_refCount;
    }

}

void Referenced::unref() const
{
    bool needDelete = false;
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex); 
        --_refCount;
        needDelete = _refCount<=0;
    }
    else
    {
        --_refCount;
        needDelete = _refCount<=0;
    }
    
    if (needDelete)
    {
        if (getDeleteHandler()) getDeleteHandler()->requestDelete(this);
        else delete this;
    }
}
*/
void Referenced::unref_nodelete() const
{
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex); 
        --_refCount;
    }
    else
    {
        --_refCount;
    }
}
        

}; // end of namespace osg

#endif //OSG_JAVA_BUILD        
