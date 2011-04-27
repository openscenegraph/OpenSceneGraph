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
// Win32Barrier.c++ - C++ Barrier class built on top of POSIX threads.
// ~~~~~~~~~~~~~~~~~~
//

#include  <OpenThreads/Barrier>
#include  <OpenThreads/Thread>
#include  <OpenThreads/ScopedLock>
#include "Win32BarrierPrivateData.h"
using namespace OpenThreads;

// so compiler can place it somewhere
Win32BarrierPrivateData::~Win32BarrierPrivateData()
{
}

//----------------------------------------------------------------------------
//
// Description: Constructor
//
// Use: public.
//
Barrier::Barrier(int numThreads) {
    Win32BarrierPrivateData *pd = new Win32BarrierPrivateData(numThreads, 0, 0);
    _valid = true;
    _prvData = static_cast<void *>(pd);
}
//----------------------------------------------------------------------------
//
// Description: Destructor
//
// Use: public.
//
Barrier::~Barrier() {
    Win32BarrierPrivateData *pd =
        static_cast<Win32BarrierPrivateData*>(_prvData);
    delete pd;
}
//----------------------------------------------------------------------------
//
// Description: Reset the barrier to its original state
//
// Use: public.
//
void Barrier::reset() {
    Win32BarrierPrivateData *pd =
        static_cast<Win32BarrierPrivateData*>(_prvData);
    pd->cnt = 0;
    pd->phase = 0;
}
//----------------------------------------------------------------------------
//
// Description: Block until numThreads threads have entered the barrier.
//
// Use: public.
//
void Barrier::block(unsigned int numThreads) {

    Win32BarrierPrivateData *pd =
        static_cast<Win32BarrierPrivateData*>(_prvData);

    if(numThreads != 0) pd->maxcnt = numThreads;
    int my_phase;

    ScopedLock<Mutex> lock(pd->lock);
    if( _valid )
    {
        my_phase = pd->phase;
        ++pd->cnt;

        if (pd->cnt == pd->maxcnt) {             // I am the last one
            pd->cnt = 0;                         // reset for next use
            pd->phase = 1 - my_phase;            // toggle phase
            pd->cond.broadcast();
        }else{ 
            while (pd->phase == my_phase ) {
                pd->cond.wait(&pd->lock);
            }
        }
    }
}

void Barrier::invalidate()
{
    Win32BarrierPrivateData *pd =
            static_cast<Win32BarrierPrivateData*>(_prvData);

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
void Barrier::release() {

    Win32BarrierPrivateData *pd =
        static_cast<Win32BarrierPrivateData*>(_prvData);

    int my_phase;

    ScopedLock<Mutex> lock(pd->lock);
    my_phase = pd->phase;
    
    pd->cnt = 0;                         // reset for next use
    pd->phase = 1 - my_phase;            // toggle phase
    pd->cond.broadcast();
    
}

//----------------------------------------------------------------------------
//
// Description: Return the number of threads currently blocked in the barrier
//
// Use: public
//
int Barrier::numThreadsCurrentlyBlocked() {
    
    Win32BarrierPrivateData *pd =
        static_cast<Win32BarrierPrivateData*>(_prvData);

    int numBlocked = -1;
    ScopedLock<Mutex> lock(pd->lock);
    numBlocked = pd->cnt;
    return numBlocked;

}
