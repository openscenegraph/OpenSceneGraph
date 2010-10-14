/* -*-c++-*- OpenThreads library, Copyright (C) 2002 - 2007  The Open Thread Group
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

//
// QtBarrier.cpp - C++ Barrier class built on top of Qt threads.
// Borrowed from Win32ThreadBarrier.cpp implementation.
// ~~~~~~~~~~~

#include "QtBarrierPrivateData.h"
#include <OpenThreads/Barrier>
#include <OpenThreads/Thread>
#include <OpenThreads/ScopedLock>
#include <iostream>

using namespace OpenThreads;

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Barrier::Barrier(int numThreads)
{
    QtBarrierPrivateData* pd = new QtBarrierPrivateData;
    pd->cnt = 0;
    pd->phase = 0;
    pd->maxcnt = numThreads;
    _valid = true;
    
    _prvData = static_cast<void *>(pd);
}

//----------------------------------------------------------------------------
//
// Decription: Destructor
//
// Use: public.
//
Barrier::~Barrier()
{
    QtBarrierPrivateData* pd = static_cast<QtBarrierPrivateData*>(_prvData);
    delete pd;
    _prvData = 0;
}

//----------------------------------------------------------------------------
//
// Description: Reset the barrier to its original state
//
// Use: public.
//
void Barrier::reset()
{
    QtBarrierPrivateData* pd = static_cast<QtBarrierPrivateData*>(_prvData);
    pd->cnt = 0;
    pd->phase = 0;
}

//----------------------------------------------------------------------------
//
// Description: Block until numThreads threads have entered the barrier.
//
// Use: public.
//
void Barrier::block(unsigned int numThreads)
{
    QtBarrierPrivateData* pd = static_cast<QtBarrierPrivateData*>(_prvData);
    if (numThreads != 0) pd->maxcnt = numThreads;
    int my_phase;
    
    ScopedLock<Mutex> lock(pd->lock);
    if ( _valid )
    {
        my_phase = pd->phase;
        ++pd->cnt;
        
        if (pd->cnt == pd->maxcnt)
        {
            pd->cnt = 0;
            pd->phase = 1 - my_phase;
            pd->cond.broadcast();
        }
        else
        { 
            while (pd->phase == my_phase )
                pd->cond.wait(&pd->lock);
        }
    }
}

void Barrier::invalidate()
{
    QtBarrierPrivateData* pd = static_cast<QtBarrierPrivateData*>(_prvData);
    pd->lock.lock();
    _valid = false;
    
    pd->lock.unlock();
    release();
}

//----------------------------------------------------------------------------
//
// Description: Release the barrier, now.
//
// Use: public.
//
void Barrier::release()
{
    QtBarrierPrivateData* pd = static_cast<QtBarrierPrivateData*>(_prvData);
    int my_phase;
    
    ScopedLock<Mutex> lock(pd->lock);
    my_phase = pd->phase;
    
    pd->cnt = 0;
    pd->phase = 1 - my_phase;
    pd->cond.broadcast();
}

//----------------------------------------------------------------------------
//
// Description: Return the number of threads currently blocked in the barrier
//
// Use: public
//
int Barrier::numThreadsCurrentlyBlocked()
{
    QtBarrierPrivateData* pd = static_cast<QtBarrierPrivateData*>(_prvData);
    
    int numBlocked = -1;
    ScopedLock<Mutex> lock(pd->lock);
    numBlocked = pd->cnt;
    return numBlocked;
}
