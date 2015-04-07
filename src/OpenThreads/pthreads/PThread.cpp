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
// PThread.c++ - C++ Thread class built on top of posix threads.
// ~~~~~~~~~~~

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>

#if defined __linux__ || defined __sun || defined __APPLE__ || ANDROID
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#if !defined ANDROID
    #include <sys/unistd.h>
#endif
#endif
#if defined(__sgi)
#include <unistd.h>
#endif
#if defined(__hpux)
#include <sys/mpctl.h>
#endif

#if defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY) || defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
#    include <sched.h>
#endif
#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__MACH__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
#endif

#if defined(__ANDROID__)
    #ifndef PAGE_SIZE
        #define PAGE_SIZE 0x400
    #endif
#endif

#include <OpenThreads/Thread>
#include "PThreadPrivateData.h"

#include <iostream>

using namespace OpenThreads;

#ifdef DEBUG
# define DPRINTF(arg) printf arg
#else
# define DPRINTF(arg)
#endif

//-----------------------------------------------------------------------------
// Initialize the static unique ids.
//
int PThreadPrivateData::nextId = 0;

//-----------------------------------------------------------------------------
// Initialize thread master priority level
//
Thread::ThreadPriority Thread::s_masterThreadPriority =
                                          Thread::THREAD_PRIORITY_DEFAULT;

bool Thread::s_isInitialized = false;
pthread_key_t PThreadPrivateData::s_tls_key;

struct ThreadCleanupStruct
{

    OpenThreads::Thread *thread;
    Atomic *runflag;

};

//-----------------------------------------------------------------------------
// This cleanup handler is necessary to ensure that the thread will cleanup
// and set its isRunning flag properly.
//
void thread_cleanup_handler(void *arg)
{

    ThreadCleanupStruct *tcs = static_cast<ThreadCleanupStruct *>(arg);

    tcs->thread->cancelCleanup();
    (*(tcs->runflag)).exchange(0);

}

//-----------------------------------------------------------------------------
// Class to support some static methods necessary for pthread's to work
// correctly.
//

namespace OpenThreads
{

class ThreadPrivateActions
{

    //-------------------------------------------------------------------------
    // We're friendly to Thread, so it can issue the methods.
    //
    friend class Thread;

private:

    //-------------------------------------------------------------------------
    // pthreads standard start routine.
    //
    static void *StartThread(void *data)
    {

        Thread *thread = static_cast<Thread *>(data);

        PThreadPrivateData *pd =
        static_cast<PThreadPrivateData *>(thread->_prvData);


        if (pd->cpunum>=0)
        {
#if defined(__sgi)
            pthread_setrunon_np( pd->cpunum );
#elif defined(HAVE_PTHREAD_SETAFFINITY_NP) || defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY) || defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
            cpu_set_t cpumask;
            CPU_ZERO( &cpumask );
            CPU_SET( pd->cpunum, &cpumask );

#if defined(HAVE_PTHREAD_SETAFFINITY_NP)
            pthread_setaffinity_np( pthread_self(), sizeof(cpumask), &cpumask);
#elif defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY)
            sched_setaffinity( 0, sizeof(cpumask), &cpumask );
#elif defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
            sched_setaffinity( 0, &cpumask );
#endif
#endif
        }
#if defined(HAVE_PTHREAD_SETAFFINITY_NP) || defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY) || defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
        else
        {
            // BUG-fix for linux:
            // Each thread inherits the processor affinity mask from its parent thread.
            // We need to explicitly set it to all CPUs, if no affinity was specified.

            cpu_set_t cpumask;
            CPU_ZERO( &cpumask );

            for (int i = 0; i < OpenThreads::GetNumberOfProcessors(); ++i)
            {
                CPU_SET( i, &cpumask );
            }

#if defined(HAVE_PTHREAD_SETAFFINITY_NP)
            pthread_setaffinity_np( pthread_self(), sizeof(cpumask), &cpumask);
#elif defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY)
            sched_setaffinity( 0, sizeof(cpumask), &cpumask );
#elif defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
            sched_setaffinity( 0, &cpumask );
#endif
        }
#endif

        ThreadCleanupStruct tcs;
        tcs.thread = thread;
        tcs.runflag = &pd->_isRunning;

        // Set local storage so that Thread::CurrentThread() can return the right thing
        int status = pthread_setspecific(PThreadPrivateData::s_tls_key, thread);
        if (status)
        {
            printf("Error: pthread_setspecific(,) returned error status, status = %d\n",status);
        }

        pthread_cleanup_push(thread_cleanup_handler, &tcs);

#ifdef ALLOW_PRIORITY_SCHEDULING

        //---------------------------------------------------------------------
        // Set the proper scheduling priorities
        //
        SetThreadSchedulingParams(thread);

#endif // ] ALLOW_PRIORITY_SCHEDULING

        pd->setRunning(true);

        // release the thread that created this thread.
        pd->threadStartedBlock.release();

        thread->run();

        pd->setRunning(false);

        pthread_cleanup_pop(0);

        return 0;

    };

    //-------------------------------------------------------------------------
    // Print information related to thread schduling parameters.
    //
    static void PrintThreadSchedulingInfo(Thread *thread) {

#ifdef ALLOW_PRIORITY_SCHEDULING // [

        if(sysconf(_POSIX_THREAD_PRIORITY_SCHEDULING))
        {

            int status, my_policy, min_priority, max_priority;
            struct sched_param my_param;

            status = pthread_getschedparam(thread->getProcessId(),
                           &my_policy,
                           &my_param);

            if(status != 0) {
            printf("THREAD INFO (%d) : Get sched: %s\n",
                   thread->getProcessId(),
                   strerror(status));
            } else {
            printf(
                "THREAD INFO (%d) : Thread running at %s / Priority: %d\n",
                thread->getProcessId(),
                (my_policy == SCHED_FIFO ? "SCHEDULE_FIFO"
                 : (my_policy == SCHED_RR ? "SCHEDULE_ROUND_ROBIN"
                : (my_policy == SCHED_OTHER ? "SCHEDULE_OTHER"
                   : "UNKNOWN"))),
                my_param.sched_priority);

            max_priority = sched_get_priority_max(my_policy);
            min_priority = sched_get_priority_min(my_policy);

            printf(
                "THREAD INFO (%d) : Max priority: %d, Min priority: %d\n",
                thread->getProcessId(),
                max_priority, min_priority);

            }

        }
        else
        {
            printf(
            "THREAD INFO (%d) POSIX Priority scheduling not available\n",
            thread->getProcessId());
        }

        fflush(stdout);

#endif // ] ALLOW_PRIORITY_SCHEDULING

    }

    //--------------------------------------------------------------------------
    // Set thread scheduling parameters.  Unfortunately on Linux, there's no
    // good way to set this, as pthread_setschedparam is mostly a no-op.
    //
    static int SetThreadSchedulingParams(Thread *thread)
    {

        int status = 0;

#ifdef ALLOW_PRIORITY_SCHEDULING // [

        if(sysconf(_POSIX_THREAD_PRIORITY_SCHEDULING))
        {

            int th_policy;
            int max_priority, nominal_priority, min_priority;
            sched_param th_param;
            pthread_getschedparam(thread->getProcessId(),
                      &th_policy, &th_param);

#ifndef __linux__

            switch(thread->getSchedulePolicy())
            {

                case Thread::THREAD_SCHEDULE_FIFO:
                th_policy = SCHED_FIFO;
                break;

                case Thread::THREAD_SCHEDULE_ROUND_ROBIN:
                th_policy = SCHED_RR;
                break;

                case Thread::THREAD_SCHEDULE_TIME_SHARE:
                th_policy = SCHED_OTHER;
                break;

                default:
#ifdef __sgi
                th_policy = SCHED_RR;
#else
                th_policy = SCHED_FIFO;
#endif
                break;
            };

#else
            th_policy = SCHED_OTHER;  // Must protect linux from realtime.
#endif

#ifdef __linux__

            max_priority = 0;
            min_priority = 20;
            nominal_priority = (max_priority + min_priority)/2;

#else

            max_priority = sched_get_priority_max(th_policy);
            min_priority = sched_get_priority_min(th_policy);
            nominal_priority = (max_priority + min_priority)/2;

#endif

            switch(thread->getSchedulePriority())
            {

                case Thread::THREAD_PRIORITY_MAX:
                    th_param.sched_priority = max_priority;
                    break;

                case Thread::THREAD_PRIORITY_HIGH:
                    th_param.sched_priority = (max_priority + nominal_priority)/2;
                    break;

                case Thread::THREAD_PRIORITY_NOMINAL:
                    th_param.sched_priority = nominal_priority;
                    break;

                case Thread::THREAD_PRIORITY_LOW:
                    th_param.sched_priority = (min_priority + nominal_priority)/2;
                    break;

                case Thread::THREAD_PRIORITY_MIN:
                    th_param.sched_priority = min_priority;
                    break;

                default:
                    th_param.sched_priority = max_priority;
                    break;

            }

            status = pthread_setschedparam(thread->getProcessId(),
                           th_policy,
                           &th_param);


            if(getenv("OUTPUT_THREADLIB_SCHEDULING_INFO") != 0)
            PrintThreadSchedulingInfo(thread);

        }

#endif // ] ALLOW_PRIORITY_SCHEDULING

    return status;
    };
};

}

//----------------------------------------------------------------------------
//
// Description: Set the concurrency level (no-op)
//
// Use static public
//
int Thread::SetConcurrency(int concurrencyLevel)
{

#if defined (HAVE_PTHREAD_SETCONCURRENCY)
    return pthread_setconcurrency(concurrencyLevel);
#else
    return -1;
#endif

}

//----------------------------------------------------------------------------
//
// Description: Get the concurrency level
//
// Use static public
//
int Thread::GetConcurrency()
{

#if defined (HAVE_PTHREAD_GETCONCURRENCY)
    return pthread_getconcurrency();
#else
    return -1;
#endif

}

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Thread::Thread()
{

    if(!s_isInitialized) Init();

    PThreadPrivateData *pd = new PThreadPrivateData();
    pd->stackSize = 0;
    pd->stackSizeLocked = false;
    pd->idSet = false;
    pd->setRunning(false);
    pd->isCanceled = false;
    pd->uniqueId = pd->nextId;
    pd->nextId++;
    pd->threadPriority = Thread::THREAD_PRIORITY_DEFAULT;
    pd->threadPolicy = Thread::THREAD_SCHEDULE_DEFAULT;
    pd->cpunum = -1;

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
    PThreadPrivateData *pd = static_cast<PThreadPrivateData *>(_prvData);

    if(pd->isRunning())
    {
        std::cout<<"Error: Thread "<<this<<" still running in destructor"<<std::endl;

        //---------------------------------------------------------------------
        // Kill the thread when it is destructed
        //
        cancel();

        // wait till the thread is stopped before finishing.
        join();

    }

    delete pd;

    _prvData = 0;
}

Thread *Thread::CurrentThread()
{
    if(!s_isInitialized) Thread::Init();

    Thread *thread =
    static_cast<Thread *>(pthread_getspecific(PThreadPrivateData::s_tls_key));

    return thread;

}

//-----------------------------------------------------------------------------
//
// Description: Initialize Threading
//
// Use: public.
//
void Thread::Init()
{

    if(s_isInitialized) return;

    // Allocate a key to be used to access thread local storage
    int status = pthread_key_create(&PThreadPrivateData::s_tls_key, NULL);
    if (status)
    {
        printf("Error: pthread_key_create(,) returned error status, status = %d\n",status);
    }

#ifdef ALLOW_PRIORITY_SCHEDULING

    //--------------------------------------------------------------------------
    // If we've got priority scheduling, set things to nominal.
    //
    if(sysconf(_POSIX_THREAD_PRIORITY_SCHEDULING))
    {

        int max_priority, nominal_priority, min_priority;

        int th_policy;
        sched_param th_param;
        pthread_getschedparam(pthread_self(),
                      &th_policy, &th_param);

        max_priority = sched_get_priority_max(th_policy);
        min_priority = sched_get_priority_min(th_policy);
        nominal_priority = (max_priority + min_priority)/2;

        th_param.sched_priority = nominal_priority;

        pthread_setschedparam(pthread_self(),
                      th_policy,
                      &th_param);

        s_masterThreadPriority = Thread::THREAD_PRIORITY_NOMINAL;

    }
    else
    {

        s_masterThreadPriority = Thread::THREAD_PRIORITY_DEFAULT;

    }

#endif // ] ALLOW_PRIORITY_SCHEDULING

    s_isInitialized = true;

}

//-----------------------------------------------------------------------------
//
// Description: Get a unique identifier for this thread.
//
// Use: public
//
int Thread::getThreadId()
{

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);
    return pd->uniqueId;
}

//-----------------------------------------------------------------------------
//
// Description: Get the thread's process id
//
// Use: public
//
size_t Thread::getProcessId()
{

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

    if(pd->idSet == false) return (size_t)(pthread_self());

    return (size_t)(pd->tid);
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's processor affinity
//
// Use: public
//
int Thread::setProcessorAffinity(unsigned int cpunum)
{
    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);
    pd->cpunum = cpunum;
    if (pd->cpunum<0) return -1;

#ifdef __sgi

    int status;
    pthread_attr_t thread_attr;

    status = pthread_attr_init( &thread_attr );
    if(status != 0)
    {
        return status;
    }

    status = pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_BOUND_NP );
    return status;

#elif defined(HAVE_PTHREAD_SETAFFINITY_NP) || defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY) || defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)

    if (pd->isRunning() && Thread::CurrentThread()==this)
    {
        cpu_set_t cpumask;
        CPU_ZERO( &cpumask );
        CPU_SET( pd->cpunum, &cpumask );
#if defined(HAVE_PTHREAD_SETAFFINITY_NP)
        return pthread_setaffinity_np (pthread_self(), sizeof(cpumask), &cpumask);
#elif defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY)
        return sched_setaffinity( 0, sizeof(cpumask), &cpumask );
#elif defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
        return sched_setaffinity( 0, &cpumask );
#endif
    }

    return -1;
#else
    return -1;
#endif

}

//-----------------------------------------------------------------------------
//
// Description: Determine if the thread is running
//
// Use: public
//
bool Thread::isRunning()
{
    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);
    return pd->isRunning();

}

//-----------------------------------------------------------------------------
//
// Description: Start the thread.
//
// Use: public
//
int Thread::start() {

    int status;
    pthread_attr_t thread_attr;

    status = pthread_attr_init( &thread_attr );
    if(status != 0)
    {
        return status;
    }

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

    //-------------------------------------------------------------------------
    // Set the stack size if requested, but not less than a platform reasonable
    // value.
    //
    if(pd->stackSize)
    {
#ifdef PTHREAD_STACK_MIN
        if(pd->stackSize < PTHREAD_STACK_MIN)
            pd->stackSize = PTHREAD_STACK_MIN;
#endif
        pthread_attr_setstacksize( &thread_attr, pd->stackSize);
        if(status != 0)
        {
            return status;
        }
    }

    //-------------------------------------------------------------------------
    // Now get what we actually have...
    //
    size_t size;
    pthread_attr_getstacksize( &thread_attr, &size);
    if(status != 0)
    {
        return status;
    }
    pd->stackSize = size;

    //-------------------------------------------------------------------------
    // Prohibit the stack size from being changed.
    //
    pd->stackSizeLocked = true;

#ifdef ALLOW_PRIORITY_SCHEDULING

    status = pthread_attr_setinheritsched( &thread_attr,
                       PTHREAD_EXPLICIT_SCHED );

    pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);

#endif // ] ALLOW_PRIORITY_SCHEDULING

    if(status != 0)
    {
        return status;
    }

    pd->threadStartedBlock.reset();

    status = pthread_create(&(pd->tid), &thread_attr,
                           ThreadPrivateActions::StartThread,
                           static_cast<void *>(this));

    if(status == 0)
    {
        // wait till the thread has actually started.
        pd->threadStartedBlock.block();

        pd->idSet = true;
    }

    return status;
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
// Description: detach the thread.
//
// Use: public
//
int Thread::detach()
{
    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);
    return pthread_detach(pd->tid);
}

//-----------------------------------------------------------------------------
//
// Description: Join the thread.
//
// Use: public
//
int Thread::join()
{

    void *threadResult = 0; // Dummy var.
    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);
    return pthread_join(pd->tid, &threadResult);

}

//-----------------------------------------------------------------------------
//
// Description: test the cancel state of the thread.
//
// Use: public
//
int Thread::testCancel()
{
#if defined(HAVE_PTHREAD_TESTCANCEL)
    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

    if(pthread_self() != pd->tid)
    return -1;

    pthread_testcancel();

    return 0;
#else
    return 0;
#endif
}


//-----------------------------------------------------------------------------
//
// Description: Cancel the thread.
//
// Use: public
//
int Thread::cancel()
{
#if defined(HAVE_PTHREAD_CANCEL)
    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);
    if (pd->isRunning())
    {
        pd->isCanceled = true;
        int status = pthread_cancel(pd->tid);
        return status;
    }
    return 0;
#else
    return 0;
#endif
}

//-----------------------------------------------------------------------------
//
// Description: Disable cancelibility
//
// Use: public
//
int Thread::setCancelModeDisable()
{
#if defined(HAVE_PTHREAD_SETCANCELSTATE)
    return pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, 0 );
#else
    return 0;
#endif
}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel immediately
//
// Use: public
//
int Thread::setCancelModeAsynchronous() {

#if defined(HAVE_PTHREAD_SETCANCELSTATE)
    int status = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
    if(status != 0) return status;

    return pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, 0);
#else
    return 0;
#endif
}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel at the next convenient point.
//
// Use: public
//
int Thread::setCancelModeDeferred() {

#if defined(HAVE_PTHREAD_SETCANCELSTATE)
    int status = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
    if(status != 0) return status;

    return pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, 0);
#else
    return 0;
#endif
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's schedule priority (if able)
//
// Use: public
//
int Thread::setSchedulePriority(ThreadPriority priority) {

#ifdef ALLOW_PRIORITY_SCHEDULING

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

    pd->threadPriority = priority;

    if(pd->isRunning)
        return ThreadPrivateActions::SetThreadSchedulingParams(this);
    else
        return 0;

#else
    return -1;
#endif

}

//-----------------------------------------------------------------------------
//
// Description: Get the thread's schedule priority (if able)
//
// Use: public
//
int Thread::getSchedulePriority() {

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

    return pd->threadPriority;

}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's scheduling policy (if able)
//
// Use: public
//
int Thread::setSchedulePolicy(ThreadPolicy policy)
{

#ifdef ALLOW_PRIORITY_SCHEDULING

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

    pd->threadPolicy = policy;

    if(pd->isRunning)
    return ThreadPrivateActions::SetThreadSchedulingParams(this);
    else
    return 0;
#else
    return -1;
#endif

}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's scheduling policy (if able)
//
// Use: public
//
int Thread::getSchedulePolicy()
{

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

    return pd->threadPolicy;

}


//-----------------------------------------------------------------------------
//
// Description: Set the thread's desired stack size
//
// Use: public
//
int Thread::setStackSize(size_t stackSize) {

    PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

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
size_t Thread::getStackSize()
{

   PThreadPrivateData *pd = static_cast<PThreadPrivateData *> (_prvData);

   return pd->stackSize;

}

//-----------------------------------------------------------------------------
//
// Description:  Print the thread's scheduling information to stdout.
//
// Use: public
//
void Thread::printSchedulingInfo()
{

    ThreadPrivateActions::PrintThreadSchedulingInfo(this);

}

//-----------------------------------------------------------------------------
//
// Description:  Yield the processor
//
// Use: protected
//
int Thread::YieldCurrentThread()
{
#if defined(HAVE_PTHREAD_YIELD)
    pthread_yield();
    return 0;
#elif defined(HAVE_SCHED_YIELD)
    return sched_yield();
#else
    return -1;
#endif
}

// Description:  sleep
//
// Use: public
//
int Thread::microSleep(unsigned int microsec)
{
#if !defined(__ANDROID__)
    return ::usleep(microsec);
#else
    ::usleep(microsec);
    return 0;
#endif
}



//-----------------------------------------------------------------------------
//
// Description:  Get the number of processors
//
int OpenThreads::GetNumberOfProcessors()
{
#if defined(__linux__)
   long ret = sysconf(_SC_NPROCESSORS_ONLN);
   if (ret == -1)
      return 0;
   return ret;
#elif defined(__sun__)
   long ret = sysconf(_SC_NPROCESSORS_ONLN);
   if (ret == -1)
      return 0;
   return ret;
#elif defined(__sgi)
   long ret = sysconf(_SC_NPROC_ONLN);
   if (ret == -1)
      return 0;
   return ret;
#elif defined(__hpux)
   int ret = mpctl(MPC_GETNUMSPUS, 0, NULL);
   if (ret == -1)
      return 0;
   return ret;
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(__MACH__)
   uint64_t num_cpus = 0;
   size_t num_cpus_length = sizeof(num_cpus);
#if defined(__FreeBSD__)
   sysctlbyname("hw.ncpu", &num_cpus, &num_cpus_length, NULL, 0);
#else
   sysctlbyname("hw.activecpu", &num_cpus, &num_cpus_length, NULL, 0);
#endif
   return num_cpus;
#else
   return 1;
#endif
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
#if defined(HAVE_PTHREAD_SETAFFINITY_NP) || defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY) || defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
        cpu_set_t cpumask;
        CPU_ZERO( &cpumask );
        CPU_SET( cpunum, &cpumask );
#if defined(HAVE_PTHREAD_SETAFFINITY_NP)
        pthread_setaffinity_np( pthread_self(), sizeof(cpumask), &cpumask);
        return 0;
#elif defined(HAVE_THREE_PARAM_SCHED_SETAFFINITY)
        sched_setaffinity( 0, sizeof(cpumask), &cpumask );
        return 0;
#elif defined(HAVE_TWO_PARAM_SCHED_SETAFFINITY)
        sched_setaffinity( 0, &cpumask );
        return 0;
#endif
#endif
    }

    return -1;
}
