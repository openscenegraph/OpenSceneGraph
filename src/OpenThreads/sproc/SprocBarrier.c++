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
// SprocBarrier.c++ - C++ Barrier class built on top of IRIX process threads.
// ~~~~~~~~~~~~~~~~

#include <ulocks.h>
#include <OpenThreads/Barrier>
#include "SprocBarrierPrivateData.h"
#include "SharedArena.h"
#include "SprocThreadPrivateActions.h"

#ifndef USE_IRIX_NATIVE_BARRIER

#include <OpenThreads/Condition>
#include <OpenThreads/Mutex>

#endif

#ifdef DEBUG
#define DPRINTF(arg) printf arg; fflush(stdout);
#else
#define DPRINTF(arg)
#endif

using namespace OpenThreads;

//----------------------------------------------------------------------------
// This cancel cleanup handler is necessary to ensure that the barrier's
// mutex gets unlocked on cancel. Otherwise deadlocks could occur with 
// later joins.
//
void barrier_cleanup_handler(void *arg) {

    DPRINTF(("(SPROC BARRIER) cleanup handler called on pid %d\n", getpid()));

    Mutex *mutex = static_cast<Mutex *>(arg);
    
    if(mutex->trylock() == 1) 
	mutex->unlock();

}

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Barrier::Barrier(int numThreads) {

    SprocBarrierPrivateData *pd = new SprocBarrierPrivateData();

#ifdef USE_IRIX_NATIVE_BARRIER

    pd->barrier = SharedArena::allocBarrier();
    pd->numThreads = numThreads;

#else

    pd->cnt = 0;
    pd->phase = 0;
    pd->maxcnt = numThreads;

#endif

    _prvData = static_cast<void *>(pd);

}

//----------------------------------------------------------------------------
//
// Decription: Destructor
//
// Use: public.
//
Barrier::~Barrier() {

    SprocBarrierPrivateData *pd =
        static_cast<SprocBarrierPrivateData*>(_prvData);

#ifdef USE_IRIX_NATIVE_BARRIER

    SharedArena::freeBarrier(pd->barrier);

#endif

    delete pd;
}

//----------------------------------------------------------------------------
//
// Decription: Reset the barrier to its original state
//
// Use: public.
//
void Barrier::reset() {
    
    SprocBarrierPrivateData *pd =
        static_cast<SprocBarrierPrivateData*>(_prvData);

#ifdef USE_IRIX_NATIVE_BARRIER

    SharedArena::initBarrier(pd->barrier);

#else
    
    pd->cnt = 0;
    pd->phase = 0;

#endif

}

//----------------------------------------------------------------------------
//
// Decription: Block until numThreads threads have entered the barrier.
//
// Use: public.
//
void Barrier::block(unsigned int numThreads) {

    SprocBarrierPrivateData *pd =
        static_cast<SprocBarrierPrivateData*>(_prvData);

#ifdef USE_IRIX_NATIVE_BARRIER

    if(numThreads == 0) {
	SharedArena::block(pd->barrier, pd->numThreads);
    } else {
	SharedArena::block(pd->barrier, numThreads);
    }

#else

    pd->_mutex.lock();

    if(numThreads != 0) pd->maxcnt = numThreads;

    int my_phase;

    my_phase = pd->phase;
    ++pd->cnt;

    DPRINTF(("(SPROC BARRIER %d) block, count=%d, maxThreads=%d, phase=%d\n",
	     getpid(), pd->cnt, pd->maxcnt, pd->phase));
    
    if(pd->cnt == pd->maxcnt) {             // I am the last one
	pd->cnt = 0;                         // reset for next use
	pd->phase = 1 - my_phase;            // toggle phase
	pd->_cond.broadcast();
    } 

    while (pd->phase == my_phase) {
	ThreadPrivateActions::PushCancelFunction(barrier_cleanup_handler, 
						 &pd->_mutex);
	pd->_cond.wait(&pd->_mutex);

	ThreadPrivateActions::PopCancelFunction();
    }

    pd->_mutex.unlock();

#endif

}

//----------------------------------------------------------------------------
//
// Description: Release the barrier, now.
//
// Use: public.
//
void Barrier::release() {

    SprocBarrierPrivateData *pd =
        static_cast<SprocBarrierPrivateData*>(_prvData);

#ifdef USE_IRIX_NATIVE_BARRIER

    printf("ERROR >>>>> Barrier::release() cannot be implemented using native IRIX Barriers !!!\n");

#else

    int my_phase;
    pd->_mutex.lock();
    
    my_phase = pd->phase;
    
    pd->cnt = 0;                         // reset for next use
    pd->phase = 1 - my_phase;            // toggle phase
    pd->_cond.broadcast();

    pd->_mutex.unlock();

#endif

}

//----------------------------------------------------------------------------
//
// Description: Return the number of threads currently blocked in the barrier
//
// Use: public
//
int Barrier::numThreadsCurrentlyBlocked() {
    
    SprocBarrierPrivateData *pd =
        static_cast<SprocBarrierPrivateData*>(_prvData);

    int numBlocked = -1;
    pd->_mutex.lock();
    numBlocked = pd->cnt;
    pd->_cond.broadcast();
    return numBlocked;

}
