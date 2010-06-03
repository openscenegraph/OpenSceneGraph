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
// SprocMutex.c++ - C++ Mutex class built on top of posix threads.
// ~~~~~~~~~~~~~~

#include <unistd.h>
#include <ulocks.h>
#include <errno.h>
#include <assert.h>

#include <OpenThreads/Mutex>
#include "SharedArena.h"
#include "SprocMutexPrivateData.h"
#include "SprocThreadPrivateActions.h"

using namespace OpenThreads;

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Mutex::Mutex(MutexType type):
    _mutexType(type)
{

    SprocMutexPrivateData *pd = new SprocMutexPrivateData();

    pd->mutex = SharedArena::allocLock();

    _prvData = static_cast<void *>(pd);

}

//----------------------------------------------------------------------------
//
// Decription: Destructor
//
// Use: public.
//
Mutex::~Mutex() {

    SprocMutexPrivateData *pd =
        static_cast<SprocMutexPrivateData*>(_prvData);

    SharedArena::deleteLock(pd->mutex);

    delete pd;

}

//----------------------------------------------------------------------------
//
// Decription: lock the mutex
//
// Use: public.
//
int Mutex::lock() {
    
    ThreadPrivateActions::ThreadCancelTest();

    SprocMutexPrivateData *pd =
        static_cast<SprocMutexPrivateData*>(_prvData);

    int status = SharedArena::lock(pd->mutex);
    
    if(status == -1) {
	return status;
    } 

    if(status == 1) {
	return 0;
    } else {
	return 1;
    } 
    return 0;
}

//----------------------------------------------------------------------------
//
// Decription: unlock the mutex
//
// Use: public.
//
int Mutex::unlock() {

    SprocMutexPrivateData *pd =
        static_cast<SprocMutexPrivateData*>(_prvData);

    int status = SharedArena::unlock(pd->mutex);

    ThreadPrivateActions::ThreadCancelTest();

    return status;

}

//----------------------------------------------------------------------------
//
// Decription: test if the mutex may be locked
//
// Use: public.
//
int Mutex::trylock() {

    SprocMutexPrivateData *pd =
        static_cast<SprocMutexPrivateData*>(_prvData);

    int status = SharedArena::trylock(pd->mutex);

    return status;
    
}

