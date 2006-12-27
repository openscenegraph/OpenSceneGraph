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
#include <OpenThreads/ScopedLock>

#include <osg/Notify>

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
#if 0
    if (_threadHoldingMutex==OpenThreads::Thread::CurrentThread() && _lockCount>0)
    {
        --_lockCount;
        if (_lockCount<=0)
        {
            _threadHoldingMutex = 0;
            return Mutex::unlock();
        }
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Error: ReentrantMutex::unlock() - unlocking from the wrong thread."<<std::endl;
    }
#else
    if (_lockCount>0)
    {
        --_lockCount;
        if (_lockCount<=0)
        {
            _threadHoldingMutex = 0;
            return Mutex::unlock();
        }
    }
#endif    
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

ReadWriteMutex::ReadWriteMutex():
    _readCount(0)
{
}

ReadWriteMutex::~ReadWriteMutex()
{
}

int ReadWriteMutex::readLock()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_readCountMutex);
    int result = 0;
    if (_readCount==0)
    {
        result = _readWriteMutex.lock();
    }
    ++_readCount;
    return result;
}

int ReadWriteMutex::readUnlock()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_readCountMutex);
    int result = 0;
    if (_readCount>0)
    {
        --_readCount;
        if (_readCount==0)
        {
            result = _readWriteMutex.unlock();
        }
    }
    return result;
}

int ReadWriteMutex::writeLock()
{
    return _readWriteMutex.lock();
}

int ReadWriteMutex::writeUnlock()
{
    return _readWriteMutex.unlock();
}
