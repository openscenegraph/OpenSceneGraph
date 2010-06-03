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


// SharedArena.h - Facilities for creating/destroying shared arenas
// ~~~~~~~~~~~~~

#ifndef _SHAREDARENA_H_
#define _SHAREDARENA_H_

#include <unistd.h>
#include <ulocks.h>
#include <errno.h>
#include <assert.h>

namespace OpenThreads {

#define OT_USESHAREDONLY

class SharedArena {

    friend class Mutex;

    friend class Barrier;

    friend class Condition;

private:

    static void initArena();  

    static void removeArena(); 
 
    static ulock_t allocLock();

    static barrier_t *allocBarrier();

    static usema_t *allocSema();

    static int getSemaFd(usema_t *sema);

    static int pSema(usema_t *sema);

    static int vSema(usema_t *sema);

    static int testSema(usema_t *sema);

    static int closeSemaFd(usema_t *sema);
    
    static int freeSema(usema_t *sema);

    static int lock(ulock_t lock);

    static int unlock(ulock_t lock);

    static int trylock(ulock_t lock);

    static void deleteLock(ulock_t lock);

    static void initBarrier(barrier_t *b);

    static void freeBarrier(barrier_t *b);
    
    static void block(barrier_t *b, unsigned int n);
      
private:

    static usptr_t *_arena;

    static int _numLocks;

    static char *_arenaName;
};

}

#endif // !_SHAREDARENA_H_
