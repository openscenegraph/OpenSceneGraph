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
// SharedArena.c++ - Facilities for creating/destroying shared arenas
// ~~~~~~~~~~~~~~~

#include <unistd.h>
#include <sys/types.h>
#include <poll.h>
#include <bstring.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdlib.h>

#include "SharedArena.h"

using namespace OpenThreads;

#ifdef DEBUG
# define DPRINTF(arg) printf arg
#else
# define DPRINTF(arg)
#endif

#define MAX_PROCS 1024

#define SEMAPHORE_ARENA_SIZE 500000

//----------------------------------------------------------------------------
// Static arena initialization
//
usptr_t *SharedArena::_arena = 0; 

int SharedArena::_numLocks = 0;

char *SharedArena::_arenaName = 0;


void SharedArena::initArena() { 

    _arenaName = tmpnam(0);
    DPRINTF(("Creating arena: %s\n", _arenaName));

    if (unlink(_arenaName) < 0) { 
	if (errno != ENOENT) { 
	    perror("unlink"); 
	    exit(1);
	} 
    } 

    if (usconfig(CONF_INITUSERS, MAX_PROCS) < 0) { 
	perror("usconfig1"); 
	exit(1); 
    }

#ifdef OT_USESHAREDONLY
    if (usconfig(CONF_ARENATYPE, US_SHAREDONLY) < 0) { 
	perror("usconfig2"); 
	exit(1); 
    }
#endif

    char *sema_arena_size_string = getenv("OPENTHREADS_SEMAPHORE_ARENA_SIZE");
    unsigned long int sema_arena_size;
    if(sema_arena_size_string != 0L) {
	sema_arena_size = atol(sema_arena_size_string);
    } else {
	sema_arena_size = SEMAPHORE_ARENA_SIZE;
    }
    
    if (usconfig(CONF_INITSIZE,sema_arena_size) < 0) { 
	perror("usconfig3"); 
	exit(1); 
    } 
	

    if ((_arena = usinit(_arenaName)) == 0) { 
	perror("usinit"); 
	exit(1); 
    } 
} 

void SharedArena::removeArena() { 

    DPRINTF(("Removing arena: %s\n", _arenaName));
#ifndef OT_USESHAREDONLY	

    if (unlink(_arenaName) < 0) { 
	perror("unlink"); 
	exit(1); 
    } 
#endif 

}

ulock_t SharedArena::allocLock() {
    
    if(_numLocks == 0) initArena();

    assert(_arena != 0);

    ulock_t lock; 
    if ((lock = usnewlock(_arena)) == 0) { 
	perror("usnewlock"); 
	printf("Num Locks: %d\n", _numLocks);
	exit(1); 
    }
    ++_numLocks;
    return lock;
}

barrier_t *SharedArena::allocBarrier() {

    if(_numLocks == 0) initArena();

    assert(_arena != 0);
    barrier_t *bar;
    if ((bar= new_barrier(_arena)) == 0) { 
	perror("new_barrier"); 
	exit(1); 
    }
    ++_numLocks;
    return bar;
	
}

int SharedArena::lock(ulock_t lock) {

    return ussetlock(lock);
}
 
int SharedArena::unlock(ulock_t lock) {

    return usunsetlock(lock);
}

int SharedArena::trylock(ulock_t lock) {

    return ustestlock(lock);
}

void SharedArena::deleteLock(ulock_t lock) {

    assert(_arena != 0);
    usfreelock(lock, _arena);
    --_numLocks;
    if(_numLocks == 0) {
	removeArena();
    }
}

void SharedArena::initBarrier(barrier_t *b) {
    init_barrier(b);
}

void SharedArena::freeBarrier(barrier_t *b) {
    assert(_arena != 0);
    free_barrier(b);
    b = 0;
    --_numLocks;
    if(_numLocks == 0) {
	removeArena();
    }
}
    
void SharedArena::block(barrier_t *b, unsigned int n) {
    barrier(b, n);
}

usema_t *SharedArena::allocSema() {

    if(_numLocks == 0) initArena();
    assert(_arena != 0);

    usema_t *sema;
    sema = usnewpollsema(_arena, 0);
    if(sema == 0) {
	perror("usnewpollsema");
	printf("NUM SEMAS: %d\n", _numLocks);
	exit(1);
    }
    ++_numLocks;
    return sema;

}

int SharedArena::getSemaFd(usema_t *sema) {
    
    int returnval;
    returnval = usopenpollsema(sema, S_IRWXU | S_IRWXG | S_IRWXO);
    if(0 > returnval) {
	perror("usopenpollsema");
	exit(1);
    }
    return returnval;

}



int SharedArena::pSema(usema_t *sema) {

    return uspsema(sema);

}

int SharedArena::vSema(usema_t *sema) {

    return usvsema(sema);

}

int SharedArena::testSema(usema_t *sema) {
    
    return ustestsema(sema);
    
}

int SharedArena::closeSemaFd(usema_t *sema) {

    int returnval;
    returnval = usclosepollsema(sema);
    if(returnval != 0) {
	perror("usclosepollsema");
	exit(1);
    }

    return returnval;

}

int SharedArena::freeSema(usema_t *sema) {

    assert(_arena != 0);
    usfreepollsema(sema, _arena);
    --_numLocks;
    if(_numLocks == 0) {
	removeArena();
    }

    return 0;
}

#undef OT_USESHAREDONLY
