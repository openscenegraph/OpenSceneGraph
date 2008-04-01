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
// SprocCondition.c++ - C++ Condition class built on sprocs.
// ~~~~~~~~~~~~~~~~~~

#include <OpenThreads/Condition>
#include "SprocConditionPrivateData.h"
#include <OpenThreads/Mutex>
#include "SharedArena.h"
#include "SprocThreadPrivateActions.h"
#include <errno.h>
#include <signal.h>

using namespace OpenThreads;

#ifdef DEBUG

#define DPRINTF(arg) printf arg; fflush(stdout);
#define DPRINTLIST(arg) ConditionDebug::printList arg; fflush(stdout);

namespace OpenThreads {

class ConditionDebug {

    friend class Condition;

private:

    static void printList(std::list<pid_t> &pid_list) {

	std::list<pid_t>::iterator iter;
	int counter = 0;
	printf("(SPROC CONDITION %d) ", getpid());
	for(iter=pid_list.begin(); iter!=pid_list.end();++iter) {
	    printf("Pid [%d]=%d, ", counter, *iter);
	    ++counter;
	}
	printf("\b\n");

    }

};

}

#else

#define DPRINTF(arg)
#define DPRINTLIST(arg)

#endif

void condition_alarm_handler(int signal) {

    //DPRINTF(("(SPROC CONDITION) signal alarm handler called.\n"));
    
    sigset(SIGALRM, SIG_DFL);
    
    unblockproc(getpid());

}

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Condition::Condition() {
    
    SprocConditionPrivateData *pd = 
	new SprocConditionPrivateData();
    
    _prvData = static_cast<void *>(pd);
    
}

//----------------------------------------------------------------------------
//
// Decription: Destructor
//
// Use: public.
//
Condition::~Condition() {

    SprocConditionPrivateData *pd = 
	static_cast<SprocConditionPrivateData *>(_prvData);

    pd->mutex.lock();
    DPRINTF(("(SPROC CONDITION) :  In destructor\n"));
    DPRINTLIST((pd->pid_list));

    //-------------------------------------------------------------------------
    // Destroy all remaining in the linked-list of waiters (pids).
    //
    pd->pid_list.clear();

    delete pd;
    _prvData = 0;
    
}

//----------------------------------------------------------------------------
//
// Decription: wait on a condition
//
// Use: public.
//
int Condition::wait(Mutex *mutex) {

    return wait(mutex, 0);

}

//----------------------------------------------------------------------------
//
// Decription: wait on a condition
//
// Use: public.
//
int Condition::wait(Mutex *mutex, unsigned long int ms) {

    unsigned int sec;
    unsigned int usec;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    pid_t mypid = getpid();

    SprocConditionPrivateData *pd = 
	static_cast<SprocConditionPrivateData *>(_prvData);

    if(ms != 0) {

	// Wait for ms milliseconds
	sec = ms / 1000;
	usec = (ms % 1000) * 1000;
	tv.tv_sec = sec;
	tv.tv_usec = usec;

	DPRINTF(("(SPROC CONDITION) :  PID %d timeout values [%d | %d]\n", 
		 mypid, tv.tv_sec, tv.tv_usec));
	
    }

    pd->mutex.lock();

    pd->pid_list.push_front(mypid);

    pd->mutex.unlock();

    DPRINTF(("(SPROC CONDITION) :  PID %d going to blockproc\n", 
	     mypid));

    int status = 0;
    
    status = setblockproccnt(mypid, 0);

    // If we're doing a timout, setup the signal handler to deal with it.
    if(tv.tv_sec != 0 || tv.tv_usec != 0) {

	 DPRINTF(("(SPROC CONDITION) :  PID %d setting timeout condition\n", 
	     mypid));

	sigset(SIGALRM, condition_alarm_handler);
	
	struct timeval recur;
	recur.tv_sec = 0;
	recur.tv_usec = 0;

	itimerval itv;
	itv.it_interval = recur;
	itv.it_value = tv;

	setitimer(ITIMER_REAL, &itv, NULL);

    }

    mutex->unlock();
    
    ThreadPrivateActions::ThreadCancelTest();

    status = blockproc(mypid);
   
    ThreadPrivateActions::ThreadCancelTest();

    mutex->lock();

    DPRINTF(("(SPROC CONDITION) :  PID %d, returned from blockproc %d\n",
	     mypid, status));

    //-------------------------------------------------------------------------
    // Pull the pid from the list
    //
    pd->mutex.lock();

    DPRINTLIST((pd->pid_list));

#ifndef DEBUG

    // KLUDGE - can optimized this by just doing -remove()-
    std::list<pid_t>::iterator iter;
    iter = pd->pid_list.begin();
    while(iter != pd->pid_list.end()) {
    	
	if(*iter == mypid) {
	    DPRINTF(("(SPROC CONDITION) : PID %d removed itself from the list\n", 
		     mypid));
	    
	    pd->pid_list.remove(mypid);
	    iter = pd->pid_list.begin();
	} else {
	    ++iter;
	}
	
    }
    
#else
    pd->pid_list.remove(mypid);
#endif

    DPRINTLIST((pd->pid_list));

    pd->mutex.unlock();
    
    if(status == -1) {
	return status;
    }

    return 0;
}

//----------------------------------------------------------------------------
//
// Decription: signal a thread to wake up.
//
// Use: public.
//
int Condition::signal() {
   
    ThreadPrivateActions::ThreadCancelTest();

    SprocConditionPrivateData *pd =
        static_cast<SprocConditionPrivateData *>(_prvData);
    
    pd->mutex.lock();
    if(pd->pid_list.empty()) {
	DPRINTF(("(SPROC CONDITION) :  No threads to signal\n"));
	pd->mutex.unlock(); // Remember to release the mutex.
	return 0;
    }
    //-------------------------------------------------------------------------
    // Perform an unblockproc on the first pid in the list.
    //
    DPRINTF(("(SPROC CONDITION) :  PID %d signaling pid %d\n", 
	     getpid(), pd->pid_list.front()));
    int status = unblockproc(pd->pid_list.front());
    pd->mutex.unlock();
    return status;
}

//----------------------------------------------------------------------------
//
// Decription: signal all threads to wake up.
//
// Use: public.
//
int Condition::broadcast() {

    
    ThreadPrivateActions::ThreadCancelTest();

    SprocConditionPrivateData *pd =
        static_cast<SprocConditionPrivateData *>(_prvData);
    
    pd->mutex.lock();

    std::list<pid_t>::iterator iter;
    for(iter = pd->pid_list.begin();
	iter != pd->pid_list.end();
	++iter) {

	DPRINTF(("(SPROC CONDITION) Broadcast to pid[%d]\n", *iter));
	unblockproc(*iter);
    }

    pd->mutex.unlock();

    return 0;
}


