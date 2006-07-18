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

#include <osgDB/ReentrantMutex>


using namespace osgDB;
using namespace OpenThreads;

ReentrantMutex::ReentrantMutex():
    _threadHoldingMutex(0),
    _lockCount(0)
{
}

ReentrantMutex::~ReentrantMutex()
{
}

int ReentrantMutex::lock()
{
    if (_threadHoldingMutex==OpenThreads::Thread::CurrentThread() && _lockCount>0)
    {
        ++_lockCount;
        return 0;
    }
    else
    {
        int result = Mutex::lock();
        if (result==0)
        {
            _threadHoldingMutex = OpenThreads::Thread::CurrentThread();
            _lockCount = 1;
        }
        return result;
    }
}

int ReentrantMutex::unlock()
{
    if (_threadHoldingMutex==OpenThreads::Thread::CurrentThread() && _lockCount>0)
    {
        --_lockCount;
        if (_lockCount<=0) return Mutex::unlock();
    }
    return 0;
}

int ReentrantMutex::trylock()
{
    if (_threadHoldingMutex==OpenThreads::Thread::CurrentThread() && _lockCount>0)
    {
        ++_lockCount;
        return 0;
    }
    else
    {
        int result = Mutex::trylock();
        if (result==0)
        {
            _threadHoldingMutex = OpenThreads::Thread::CurrentThread();
            _lockCount = 1;
        }
        return result;
    }
}
