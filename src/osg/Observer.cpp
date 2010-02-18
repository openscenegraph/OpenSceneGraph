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

#include <osg/ObserverNodePath>
#include <osg/Notify>

using namespace osg;

ObserverSet::ObserverSet()
{
    //OSG_NOTICE<<"ObserverSet::ObserverSet() "<<this<<std::endl;
}

ObserverSet::~ObserverSet()
{
    //OSG_NOTICE<<"ObserverSet::~ObserverSet() "<<this<<", _observers.size()="<<_observers.size()<<std::endl;
}

void ObserverSet::addObserver(Observer* observer)
{
    //OSG_NOTICE<<"ObserverSet::addObserver("<<observer<<") "<<this<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*getObserverSetMutex());
    _observers.insert(observer);
}

void ObserverSet::removeObserver(Observer* observer)
{
    //OSG_NOTICE<<"ObserverSet::removeObserver("<<observer<<") "<<this<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*getObserverSetMutex());
    _observers.erase(observer);
}

void ObserverSet::signalObjectUnreferenced(void* ptr)
{
    for(Observers::iterator itr = _observers.begin();
        itr != _observers.end();
        )
    {
        if ((*itr)->objectUnreferenced(ptr))
        {
            Observers::iterator orig_itr = itr;
            ++itr;
            _observers.erase(orig_itr);
        }
        else
        {
            ++itr;
        }
    }
}

void ObserverSet::signalObjectDeleted(void* ptr)
{
    for(Observers::iterator itr = _observers.begin();
        itr != _observers.end();
        ++itr)
    {
        (*itr)->objectDeleted(ptr);
    }
    _observers.clear();
}