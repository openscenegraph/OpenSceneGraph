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
// SprocThread.c++ - C++ Thread class built on top of IRIX sproc.
// ~~~~~~~~~~~~~~~

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/sysmp.h>
#include <signal.h>
#include <unistd.h>
#include <list>
#include <OpenThreads/Thread>
#include "SprocMutexPrivateData.h"
#include "SprocThreadPrivateData.h"
#include "SprocThreadPrivateActions.h"

using namespace OpenThreads;

extern int errno;

#ifdef DEBUG
#define DPRINTF(arg) printf arg; fflush(stdout);
#else
#define DPRINTF(ARG)
#endif

static void sproc_dead_child_sig_handler(int sigid);

//-----------------------------------------------------------------------------
// Initialize the static unique ids.
//
int SprocThreadPrivateData::nextId = 0;

//-----------------------------------------------------------------------------
// Initialize thread master priority level
//
Thread::ThreadPriority Thread::s_masterThreadPriority =
                                          Thread::THREAD_PRIORITY_MAX;

bool Thread::s_isInitialized = false;

std::list<Thread *> ThreadPrivateActions::s_threadList;

void ThreadPrivateActions::ThreadCancelTest() {

    OpenThreads::Thread *t = GetThread(getpid());

    if(t != 0L) {

	SprocThreadPrivateData *pd =
	    static_cast<SprocThreadPrivateData *>(t->_prvData);

	bool *dieflag = GetDeathFlag(t);

	if(*dieflag==false) return;

	DPRINTF(("(SPROC THREAD) Thread Cancel Test Passed for %d\n",
		 getpid()));

	if(!pd->cancelFuncStack.empty())
	    pd->cancelFuncStack.top().routine(pd->cancelFuncStack.top().arg);

	t->cancelCleanup();
	pd->isRunning = false;

	exit(1);
    }
}

bool *ThreadPrivateActions::GetDeathFlag(Thread *thread) {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *>(thread->_prvData);

    return (bool *)(&(pd->dieFlag));
}

Thread *ThreadPrivateActions::GetThread(pid_t thread_id) {

    std::list<Thread *>::iterator iter;
    for(iter = s_threadList.begin();
	iter != s_threadList.end();
	++iter) {

	Thread *t = *iter;
	if(t->getProcessId() == thread_id) return t;

    }

    return 0L; // no thread found;

};

void ThreadPrivateActions::ThreadCancelHandler(int sigid) {

    Thread *t = GetThread(getpid());

    if(t != 0L) {

	bool * dieflag = GetDeathFlag(t);

	*dieflag = true;

	sigset(SIGINT, SIG_DFL);
	unblockproc(getpid());
    }
}

//-------------------------------------------------------------------------
// standard start routine.
//
void ThreadPrivateActions::StartThread(void *data)
{

    Thread *thread = static_cast<Thread *>(data);

    if (thread->_prvData==0) return;

    AddThread(thread);

    *((Thread **)&PRDA->usr_prda) = (Thread *)thread;

    SetThreadSchedulingParams(thread);

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *>(thread->_prvData);

    sigset(SIGINT, ThreadCancelHandler);

    size_t defaultStackSize;
    prctl(PR_GETSTACKSIZE, &defaultStackSize);

    if(defaultStackSize < pd->stackSize) {
	prctl(PR_SETSTACKSIZE, pd->stackSize);
    }

    prctl(PR_GETSTACKSIZE, &pd->stackSize);

    pd->stackSizeLocked = true;

    pd->isRunning = true;
    
    // release the thread that created this thread.
    pd->threadStartedBlock.release();
    
    thread->run();

    pd->isRunning = false;

    RemoveThread(thread);

    if(pd->detached == true ) {
	exit(0);
    }

    return;

};

void ThreadPrivateActions::AddThread(Thread *thread) {

    s_threadList.push_front(thread);

    };

void ThreadPrivateActions::RemoveThread(Thread *thread) {
    s_threadList.remove(thread);
};

void ThreadPrivateActions::PrintThreadSchedulingInfo(Thread *thread) {

    int status, my_policy, min_priority, max_priority;
    struct sched_param my_param;

    status = sched_getparam(thread->getProcessId(),
			    &my_param);

    my_policy = sched_getscheduler(thread->getProcessId());

    if(status != 0 || my_policy == -1) {

	printf("THREAD INFO (%d) : Get sched param: %s/%s\n",
	       (unsigned int)(thread->getProcessId()),
	       strerror(status),
	       strerror(errno));
    } else {
	printf(
	    "THREAD INFO (%d) : Thread running at %s / Priority: %d\n",
	    (unsigned int)(thread->getProcessId()),
	    (my_policy == SCHED_FIFO ? "SCHEDULE_FIFO"
	     : (my_policy == SCHED_RR ? "SCHEDULE_ROUND_ROBIN"
		: (my_policy == SCHED_TS ? "SCHEDULE_TIME_SHARE"
		   : (my_policy == SCHED_OTHER ? "SCHEDULE_OTHER"
		      : "UNKNOWN")))),
	    my_param.sched_priority);

	max_priority = sched_get_priority_max(my_policy);
	min_priority = sched_get_priority_min(my_policy);

	printf(
	    "THREAD INFO (%d) : Max priority: %d, Min priority: %d\n",
	    (unsigned int)(thread->getProcessId()),
	    max_priority, min_priority);

    }

}

int ThreadPrivateActions::SetThreadSchedulingParams(Thread *thread) {

    int status;

    int th_priority;
    int max_priority, nominal_priority, min_priority;

    max_priority = 0;  // This is as high as we can regularly go.
    min_priority = 20;
    nominal_priority = (max_priority + min_priority)/2;

    switch(thread->getSchedulePriority()) {

    case Thread::THREAD_PRIORITY_MAX:
	th_priority = max_priority;
	break;

    case Thread::THREAD_PRIORITY_HIGH:
	th_priority = (max_priority + nominal_priority)/2;
	break;

    case Thread::THREAD_PRIORITY_NOMINAL:
	th_priority = nominal_priority;
	break;

    case Thread::THREAD_PRIORITY_LOW:
	th_priority = (min_priority + nominal_priority)/2;
	break;

    case Thread::THREAD_PRIORITY_MIN:
	th_priority =  min_priority;
	break;

    default:
	th_priority = max_priority;
	break;

    }

    status = setpriority(PRIO_PROCESS, thread->getProcessId(),
			 th_priority);

    if(getenv("OUTPUT_THREADLIB_SCHEDULING_INFO") != 0)
	PrintThreadSchedulingInfo(thread);

    return status;
};

void ThreadPrivateActions::PushCancelFunction(void (*routine)(void *), void *arg) {

    Thread *thread = GetThread(getpid());

    if(thread != 0L) {
	SprocThreadPrivateData *pd =
	    static_cast<SprocThreadPrivateData *>(thread->_prvData);

	SprocThreadPrivateData::CancelFuncStruct c;

	pd->cancelFuncStack.push(c);

	SprocThreadPrivateData::CancelFuncStruct *cft = &(pd->cancelFuncStack.top());

	cft->routine = routine;
	cft->arg = arg;
    }
}

void ThreadPrivateActions::PopCancelFunction() {

    Thread *thread = GetThread(getpid());

    if(thread != 0L) {

	SprocThreadPrivateData *pd =
	    static_cast<SprocThreadPrivateData *>(thread->_prvData);

	if(!pd->cancelFuncStack.empty())
	    pd->cancelFuncStack.pop();
    }
}

//----------------------------------------------------------------------------
//
// Description: Set the concurrency level (no-op)
//
// Use static public
//
int Thread::SetConcurrency(int concurrencyLevel) {

    return -1;

};

//----------------------------------------------------------------------------
//
// Description: Get the concurrency level
//
// Use static public
//
int Thread::GetConcurrency() {

    return -1;

};

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Thread::Thread() {

    if(!s_isInitialized) Init();

    SprocThreadPrivateData *pd = new SprocThreadPrivateData();
    pd->stackSize = 128*1024;    // Set a minimum of 128K bytes if possible.
    pd->stackSizeLocked = false;
    pd->isRunning = false;
    pd->isCanceled = false;
    pd->idSet = false;
    pd->cancelActive = true;
    pd->detached = false;
    pd->uniqueId = pd->nextId;
    pd->nextId++;
    pd->threadPriority = Thread::THREAD_PRIORITY_DEFAULT;
    pd->threadPolicy = Thread::THREAD_SCHEDULE_DEFAULT;

    _prvData = static_cast<void *>(pd);
}

//----------------------------------------------------------------------------
//
// Decription: Destructor
//
// Use: public.
//
Thread::~Thread()
{
    DPRINTF(("(SPROC THREAD) %s:%d, In OpenThreads::Thread destructor\n",
	__FILE__, __LINE__));

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *>(_prvData);

    if(pd->isRunning)
    {

	DPRINTF(("(SPROC THREAD) %s:%d, about to kill OpenThreads::Thread\n",
		 __FILE__, __LINE__));


	//-------------------------------------------------------------------
	//  Kill the process when the thread is destroyed.
	//
	cancel();

	while (pd->isRunning == true) {
	    ::usleep(1);
	}

    }


    DPRINTF(("(SPROC THREAD) %s:%d, Thread destroying private data.\n",
	     __FILE__, __LINE__));


    delete pd;

    _prvData = 0;
}

//-----------------------------------------------------------------------------
//
// Description: Initialize Threading
//
// Use: public.
//
void Thread::Init() {

    if(s_isInitialized) return;

#ifdef GP_DEBUG
    fprintf(stderr, "%s\n", OPENTHREAD_VERSION_STRING);
#endif

    s_masterThreadPriority = Thread::THREAD_PRIORITY_MAX;

    s_isInitialized = true;

}

//-----------------------------------------------------------------------------
//
// Description: Return a pointer to the currently executing thread
//
// Use: public
//
Thread *Thread::CurrentThread() {

    return (*(Thread **)&PRDA->usr_prda);

}

//-----------------------------------------------------------------------------
//
// Description: Get a unique identifier for this thread.
//
// Use: public
//
int Thread::getThreadId() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);
    return pd->uniqueId;
}

//-----------------------------------------------------------------------------
//
// Description: Get the thread's process id
//
// Use: public
//
size_t Thread::getProcessId() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    if(pd->idSet == false) return getpid();

    return (size_t)(pd->pid);

}

//-----------------------------------------------------------------------------
//
// Description: Determine if the thread is running
//
// Use: public
//
bool Thread::isRunning() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    return pd->isRunning;

}

//-----------------------------------------------------------------------------
//
// Description: Start the thread.
//
// Use: public
//
int Thread::start() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    pd->threadStartedBlock.reset();

    int pid = sproc(ThreadPrivateActions::StartThread,
		    PR_SALL,
		    static_cast<void *>(this));

    // PR_SADDR | PR_SDIR | PR_SUMASK | PR_SULIMIT | PR_SID,

    if(pid < 0) {
	perror("sproc encountered an error");
	return -1;
    }

    //-----------------------------------------------------------------
    // Make the thread runnable anywhere.
    //
    sysmp(MP_RUNANYWHERE_PID, pid);

    pd->pid = pid;
    pd->idSet = true;

    // wait till the thread has actually started.
    pd->threadStartedBlock.block();

    return 0;

}

//-----------------------------------------------------------------------------
//
// Description: Alternate thread start routine.
//
// Use: public
//
int Thread::startThread()
{
    if (_prvData) return start(); 
    else return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Join the thread.
//
// Use: public
//
int Thread::detach() {

    int status = 0;

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    pd->detached=true;
    sigset(SIGCLD, sproc_dead_child_sig_handler);

    return status;

}

//-----------------------------------------------------------------------------
//
// Description: Join the thread.
//
// Use: public
//
int Thread::join() {

    int status;

    return waitpid((pid_t)getProcessId(), &status, 0);
    //return status;

}

//-----------------------------------------------------------------------------
//
// Description: test the cancel state of the thread.
//
// Use: public
//
int Thread::testCancel() {

    if(getpid() != getProcessId()) return -1;

    ThreadPrivateActions::ThreadCancelTest();

    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Cancel the thread.
//
// Use: public
//
int Thread::cancel() {

    int status = 0;

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    if(pd->cancelActive) {

	status = kill((pid_t)getProcessId(), SIGINT);
    };

    return status;

}

//-----------------------------------------------------------------------------
//
// Description: Disable cancelibility
//
// Use: public
//
int Thread::setCancelModeDisable() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    pd->cancelActive = false;

    return 0;

}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel immediately
//
// Use: public
//
int Thread::setCancelModeAsynchronous() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    pd->cancelActive = true;

    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel at the next convienent point.
//
// Use: public
//
int Thread::setCancelModeDeferred() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    pd->cancelActive = true;

    return 0;

}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's schedule priority (if able)
//
// Use: public
//
int Thread::setSchedulePriority(ThreadPriority priority) {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    pd->threadPriority = priority;

    if(pd->isRunning)
        return ThreadPrivateActions::SetThreadSchedulingParams(this);
    else
        return 0;

}

//-----------------------------------------------------------------------------
//
// Description: Get the thread's schedule priority (if able)
//
// Use: public
//
int Thread::getSchedulePriority() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    return pd->threadPriority;

}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's scheduling policy (if able)
//
// Use: public
//
int Thread::setSchedulePolicy(ThreadPolicy policy) {

    return 0;

}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's scheduling policy (if able)
//
// Use: public
//
int Thread::getSchedulePolicy() {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    return pd->threadPolicy;

}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's desired stack size
//
// Use: public
//
int Thread::setStackSize(size_t stackSize) {

    SprocThreadPrivateData *pd =
	static_cast<SprocThreadPrivateData *> (_prvData);

    if(pd->stackSizeLocked == true) return 13;  // EACESS

    pd->stackSize = stackSize;

    return 0;

}

//-----------------------------------------------------------------------------
//
// Description: Get the thread's stack size.
//
// Use: public
//
size_t Thread::getStackSize() {

   SprocThreadPrivateData *pd =
       static_cast<SprocThreadPrivateData *> (_prvData);

   return pd->stackSize;

}

//-----------------------------------------------------------------------------
//
// Description:  Print the thread's scheduling information to stdout.
//
// Use: public
//
void Thread::printSchedulingInfo() {

    ThreadPrivateActions::PrintThreadSchedulingInfo(this);

}

//-----------------------------------------------------------------------------
//
// Description:  Yield the processor
//
// Use: protected
//
int Thread::YieldCurrentThread() {

    return sched_yield();

}

//-----------------------------------------------------------------------------
// Description:  sleep
//
// Use: public
//
int Thread::microSleep(unsigned int microsec)
{
    return ::usleep(microsec);
}

static void sproc_dead_child_sig_handler(int sigid) {

#ifdef DEBUG
    int pid, status;
    pid = wait(&status);
    DPRINTF(("(SPROC THREAD) Dead Child Handler Caught Signal, Reaped %d\n",
	     pid));
#endif

    sigset(SIGCLD, sproc_dead_child_sig_handler);

}

int Thread::setProcessorAffinity( unsigned int cpunum )
{
    return -1;
}

//-----------------------------------------------------------------------------
//
// Description:  Get the number of processors
//
int OpenThreads::GetNumberOfProcessors()
{
    return 1;
}

int OpenThreads::SetProcessorAffinityOfCurrentThread(unsigned int cpunum)
{
    Thread::Init();

    Thread* thread = Thread::CurrentThread();
    if (thread) 
    {
        return thread->setProcessorAffinity(cpunum);
    }
    else
    {
        // non op right now, needs implementation.
        return -1;
    }
}
