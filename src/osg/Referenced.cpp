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
#include <osg/ApplicationUsage>
#include <osg/observer_ptr>

#include <typeinfo>
#include <memory>
#include <set>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#ifndef OSG_JAVA_BUILD       

namespace osg
{

typedef std::set<Observer*> ObserverSet;

static bool s_useThreadSafeReferenceCounting = getenv("OSG_THREAD_SAFE_REF_UNREF")!=0;
static std::auto_ptr<DeleteHandler> s_deleteHandler(0);
static ApplicationUsageProxy Referenced_e0(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_THREAD_SAFE_REF_UNREF","");

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

Referenced::Referenced():
    _refMutex(0),
    _refCount(0),
    _observers(0)
{
    if (s_useThreadSafeReferenceCounting) _refMutex = new OpenThreads::Mutex;
}

Referenced::Referenced(const Referenced&):
    _refMutex(0),
    _refCount(0),
    _observers(0)
{
    if (s_useThreadSafeReferenceCounting) _refMutex = new OpenThreads::Mutex;
}

Referenced::~Referenced()
{
    if (_refCount>0)
    {
        notify(WARN)<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
        notify(WARN)<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
    }

    if (_observers)
    {
        ObserverSet* os = static_cast<ObserverSet*>(_observers);
        for(ObserverSet::iterator itr = os->begin();
            itr != os->end();
            ++itr)
        {
            (*itr)->objectDeleted(this);
        }
        delete os;
        _observers = 0;
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

void Referenced::addObserver(Observer* observer_ptr)
{
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex); 

        if (!_observers) _observers = new ObserverSet;
        if (_observers) static_cast<ObserverSet*>(_observers)->insert(observer_ptr);
    }
    else
    {
        if (!_observers) _observers = new ObserverSet;
        if (_observers) static_cast<ObserverSet*>(_observers)->insert(observer_ptr);
    }
}

void Referenced::removeObserver(Observer* observer_ptr)
{
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex); 

        if (_observers) static_cast<ObserverSet*>(_observers)->erase(observer_ptr);
    }
    else
    {
        if (_observers) static_cast<ObserverSet*>(_observers)->erase(observer_ptr);
    }
}

}; // end of namespace osg

#endif //OSG_JAVA_BUILD        
