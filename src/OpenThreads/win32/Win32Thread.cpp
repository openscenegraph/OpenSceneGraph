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
// Win32Thread.c++
// ~~~~~~~~~~~
#include <memory>
#include <string>
#include <iostream>
#include <process.h>
#include <stdlib.h>

#if defined(_MSC_VER) && (_MSC_VER < 1300)
#ifdef __SGI_STL
using std::size_t;
#endif
#else
using std::size_t;
#endif

#if defined(_MSC_VER)
    #pragma warning( disable : 4996 )
#endif

#include "Win32ThreadPrivateData.h"

struct Win32ThreadCanceled{};

using namespace OpenThreads;

DWORD OpenThreads::cooperativeWait(HANDLE waitHandle, unsigned long timeout){
    Thread* current = Thread::CurrentThread();
    DWORD dwResult ;
    if(current)
    {
        HANDLE cancelHandle = static_cast<Win32ThreadPrivateData*>(current->getImplementation())->cancelEvent.get();
        HANDLE handleSet[2] = {waitHandle, cancelHandle};

        dwResult = WaitForMultipleObjects(2,handleSet,FALSE,timeout);
        if(dwResult == WAIT_OBJECT_0 + 1 ) throw Win32ThreadCanceled();
    }
    else
    {
        dwResult = WaitForSingleObject(waitHandle,timeout);
    }

    return dwResult;
}

Win32ThreadPrivateData::TlsHolder Win32ThreadPrivateData::TLS;

Win32ThreadPrivateData::~Win32ThreadPrivateData()
{
}

//-----------------------------------------------------------------------------
// Initialize thread master priority level
//
Thread::ThreadPriority Thread::s_masterThreadPriority =  Thread::THREAD_PRIORITY_DEFAULT;

bool Thread::s_isInitialized = false;
//-----------------------------------------------------------------------------
// Class to support some static methods necessary for pthread's to work
// correctly.
//
namespace OpenThreads {

    class ThreadPrivateActions {
        //-------------------------------------------------------------------------
        // We're friendly to Thread, so it can issue the methods.
        //
        friend class Thread;
    private:

        //-------------------------------------------------------------------------
        // Win32Threads standard start routine.
        //
        static unsigned int __stdcall StartThread(void *data) {

            Thread *thread = static_cast<Thread *>(data);

            Win32ThreadPrivateData *pd =
                static_cast<Win32ThreadPrivateData *>(thread->_prvData);

            if (thread->_prvData==0) return 0;

            TlsSetValue(Win32ThreadPrivateData::TLS.getId(), data);
            //---------------------------------------------------------------------
            // Set the proper scheduling priorities
            //
            SetThreadSchedulingParams(thread);

            pd->isRunning = true;

            pd->uniqueId = Thread::CurrentThreadId();

            // release the thread that created this thread.
            pd->threadStartedBlock.release();

            thread->setProcessorAffinity(pd->affinity);

            try{
                thread->run();
            }
            catch(Win32ThreadCanceled&)
            {
                // thread is canceled do cleanup
                try {
                    thread->cancelCleanup();
                } catch(...) { }
            }
            catch(...)
            {
                // abnormal termination but must be caught in win32 anyway
            }

            TlsSetValue(Win32ThreadPrivateData::TLS.getId(), 0);
            pd->isRunning = false;

            return 0;
        };

        //-------------------------------------------------------------------------
        // Print information related to thread schduling parameters.
        //
        static void PrintThreadSchedulingInfo(Thread *thread) {

            // Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *>(thread->_prvData);

            std::cout<<"Thread "<< thread <<" priority : ";

            switch(thread->getSchedulePriority()) {
            case Thread::THREAD_PRIORITY_MAX:
                std::cout<<"MAXIMAL"<<std::endl;
                break;
            case Thread::THREAD_PRIORITY_HIGH:
                std::cout<<"HIGH"<<std::endl;
                break;
            case Thread::THREAD_PRIORITY_DEFAULT:
            case Thread::THREAD_PRIORITY_NOMINAL:
                std::cout<<"NORMAL"<<std::endl;
                break;
            case Thread::THREAD_PRIORITY_LOW:
                std::cout<<"LOW"<<std::endl;
                break;
            case Thread::THREAD_PRIORITY_MIN:
                std::cout<<"MINIMAL"<<std::endl;
                break;
            }
        }

        //--------------------------------------------------------------------------
        // Set thread scheduling parameters.
        // Note that time-critical priority is omitted :
        // 1) It's not sensible thing to do
        // 2) there's no enum for that in Thread interface
        // Also, on Windows, effective thread priority is :
        // process priority (manipulated with Get/SetProrityClass) + thread priority (here).
        //
        //
        static int SetThreadSchedulingParams(Thread *thread) {

            Win32ThreadPrivateData *pd =
                static_cast<Win32ThreadPrivateData *>(thread->_prvData);

            int prio = THREAD_PRIORITY_NORMAL;

            switch(thread->getSchedulePriority()) {
            case Thread::THREAD_PRIORITY_MAX:
                prio = THREAD_PRIORITY_HIGHEST;
                break;
            case Thread::THREAD_PRIORITY_HIGH:
                prio = THREAD_PRIORITY_ABOVE_NORMAL;
                break;
            case Thread::THREAD_PRIORITY_NOMINAL:
                prio = THREAD_PRIORITY_NORMAL;
                break;
            case Thread::THREAD_PRIORITY_LOW:
                prio = THREAD_PRIORITY_BELOW_NORMAL;
                break;
            case Thread::THREAD_PRIORITY_MIN:
                prio = THREAD_PRIORITY_IDLE;
                break;
            }

            int status = SetThreadPriority( pd->tid.get(), prio);

            if(getenv("OUTPUT_THREADLIB_SCHEDULING_INFO") != 0)
                PrintThreadSchedulingInfo(thread);

            return status!=0;
        };
    };
}

Thread* Thread::CurrentThread()
{
    DWORD ID = Win32ThreadPrivateData::TLS.getId();
    if (ID == TLS_OUT_OF_INDEXES)
        return 0;
    return (Thread* )TlsGetValue(ID);
}

size_t Thread::CurrentThreadId()
{
    return (size_t)::GetCurrentThreadId();
}

//----------------------------------------------------------------------------
//
// Description: Set the concurrency level (no-op)
//
// Use static public
//
int Thread::SetConcurrency(int) {
    return -1;
}

//----------------------------------------------------------------------------
//
// Description: Get the concurrency level
//
// Use static public
//
int Thread::GetConcurrency() {
    return -1;
}

Win32ThreadPrivateData::Win32ThreadPrivateData()
{
    stackSize = 0;
    isRunning = false;
    cancelMode = 0;
    uniqueId = 0;
    threadPriority = Thread::THREAD_PRIORITY_DEFAULT;
    threadPolicy = Thread::THREAD_SCHEDULE_DEFAULT;
    detached = false;
    cancelEvent.set(CreateEvent(NULL,TRUE,FALSE,NULL));
}

//----------------------------------------------------------------------------
//
// Description: Constructor
//
// Use: public.
//
Thread::Thread() {

    // there's no need for this
    //    if(!s_isInitialized) Init();

    Win32ThreadPrivateData *pd = new Win32ThreadPrivateData();

    _prvData = static_cast<void *>(pd);
}


//----------------------------------------------------------------------------
//
// Description: Destructor
//
// Use: public.
//
Thread::~Thread()
{
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *>(_prvData);

    if(pd->isRunning)
    {
        std::cout<<"Error: Thread "<<this<<" still running in destructor"<<std::endl;
        pd->cancelMode = 0;
        cancel();

        join();
    }

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
//    if(s_isInitialized) return;
//        s_masterThreadPriority = Thread::THREAD_PRIORITY_DEFAULT;
    s_isInitialized = true;
}

//-----------------------------------------------------------------------------
//
// Description: Get a unique identifier for this thread.
//
// Use: public
//
size_t Thread::getThreadId() {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    return pd->uniqueId;
}
//-----------------------------------------------------------------------------
//
// Description: Get the thread's process id
//
// Use: public
//
size_t Thread::getProcessId() {

    return (size_t) GetCurrentProcessId();

}
//-----------------------------------------------------------------------------
//
// Description: Determine if the thread is running
//
// Use: public
//
bool Thread::isRunning() {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    return pd->isRunning;
}
//-----------------------------------------------------------------------------
//
// Description: Start the thread.
//
// Use: public
//
int Thread::start() {

    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    if (pd->isRunning)
    {
        return 0;
    }

    //-------------------------------------------------------------------------
    // Prohibit the stack size from being changed.
    // (bb 5/13/2005) it actually doesn't matter.
    // 1) usually setStackSize()/start() sequence is serialized.
    // 2) if not than we're in trouble anyway - nothing is protected
    // pd->stackSizeLocked = true;
    unsigned int ID;

    pd->threadStartedBlock.reset();

    pd->tid.set( (void*)_beginthreadex(NULL,static_cast<unsigned>(pd->stackSize),ThreadPrivateActions::StartThread,static_cast<void *>(this),CREATE_SUSPENDED,&ID));
    ResumeThread(pd->tid.get());

    pd->uniqueId = (size_t)ID;

    if(!pd->tid) {
        return -1;
    }

    // wait till the thread has actually started.
    pd->threadStartedBlock.block();

    return 0;

}

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
int Thread::join() {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    if( pd->detached )
        return -1; // cannot wait for detached ;

    if( WaitForSingleObject(pd->tid.get(),INFINITE) != WAIT_OBJECT_0)
        return -1 ;

    return 0;
}



int Thread::detach()
{
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    pd->detached = true;
    return 0;
}


//-----------------------------------------------------------------------------
//
// Description: Cancel the thread.
//
// Use: public
//
int Thread::cancel()
{
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);

    if (pd->isRunning)
    {
        if( pd->cancelMode == 2 )
            return -1;

        // signal all interested parties that we are going to exit
        SetEvent(pd->cancelEvent.get());

        // cancelMode == 1 (asynch)-> kill em
        // cancelMode == 0 (deffered) -> wait a little then kill em

    //    if( (pd->cancelMode == 1) || (WaitForSingleObject(pd->tid,INFINITE)!=WAIT_OBJECT_0) )
        if( pd->cancelMode == 1 )
        {
            // did not terminate cleanly force termination
            pd->isRunning = false;
            return TerminateThread(pd->tid.get(),(DWORD)-1);
        }
    }

    return 0;
}



int Thread::testCancel()
{
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);

    if(WaitForSingleObject(pd->cancelEvent.get(),0) != WAIT_OBJECT_0) return 0;

    if(pd->cancelMode == 2)
        return 0;

    size_t curr = Thread::CurrentThreadId();

    if( pd->uniqueId != curr )
        return -1;

//    pd->isRunning = false;
//    ExitThread(0);
    throw Win32ThreadCanceled();

//    return 0;
}



//-----------------------------------------------------------------------------
//
// Description: Disable cancelibility
//
// Use: public
//
int Thread::setCancelModeDisable() {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    pd->cancelMode = 2;
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel immediately
//
// Use: public
//
int Thread::setCancelModeAsynchronous() {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    pd->cancelMode  = 1;
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel at the next convenient point.
//
// Use: public
//
int Thread::setCancelModeDeferred() {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    pd->cancelMode = 0;
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's schedule priority (if able)
//
// Use: public
//
int Thread::setSchedulePriority(ThreadPriority priority) {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);

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
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    return pd->threadPriority;
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's scheduling policy (if able)
//
// Use: public
//
int Thread::setSchedulePolicy(ThreadPolicy policy) {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);

    pd->threadPolicy = policy;

    if(pd->isRunning)
        return ThreadPrivateActions::SetThreadSchedulingParams(this);
    else
        return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's scheduling policy (if able)
//
// Use: public
//
int Thread::getSchedulePolicy() {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    return pd->threadPolicy;
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's desired stack size
//
// Use: public
//
int Thread::setStackSize(size_t stackSize) {
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    if(pd->isRunning) return 13;  // cannot set stack size of running thread  return EACESS
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
    Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    return pd->stackSize;
}

static int SetThreadAffinity(HANDLE tid, const Affinity& affinity)
{
	unsigned int numprocessors = OpenThreads::GetNumberOfProcessors();
	//std::cout << "SetThreadAffinity() : affinity.activeCPUs.size()=" << affinity.activeCPUs.size() << ", numprocessors=" << numprocessors << std::endl;

    DWORD_PTR affinityMask = 0x0;
    DWORD_PTR maskBit = 0x1;
	if (affinity)
	{
		for (Affinity::ActiveCPUs::const_iterator itr = affinity.activeCPUs.begin();
			itr != affinity.activeCPUs.end();
			++itr)
		{
			unsigned int cpunum = *itr;
			if (cpunum<numprocessors)
			{
				affinityMask |= (maskBit << cpunum);
			}
		}
        //std::cout << "   Setting affinityMask : 0x" << std::hex << affinityMask << std::dec << std::endl;
	}
	else
	{
		for (unsigned int cpunum = 0; cpunum < numprocessors; ++cpunum)
		{

			affinityMask |= (maskBit << cpunum);
		}
		//std::cout << "   Fallback setting affinityMask : 0x" << std::hex << affinityMask << std::dec << std::endl;
	}

	DWORD_PTR res = SetThreadAffinityMask ( tid, affinityMask );

	// return value 1 means call is ignored ( 9x/ME/SE )
	if (res == 1) return -1;
	// return value 0 is failure
	return (res == 0) ? GetLastError() : 0;
}

//-----------------------------------------------------------------------------
//
// Description:  set processor affinity for the thread
//
// Use: public
//
int Thread::setProcessorAffinity( const Affinity& affinity )
{
	Win32ThreadPrivateData *pd = static_cast<Win32ThreadPrivateData *> (_prvData);
    pd->affinity = affinity;
    if (!pd->isRunning)
       return 0;

    if (pd->tid.get() == INVALID_HANDLE_VALUE)
       return -1;

	return SetThreadAffinity(pd->tid.get(), affinity);
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
#if _WIN32_WINNT < 0x0400 // simulate
int SwitchToThread (void)
{
    ::Sleep(10);
    return 0;
};
#endif

int Thread::YieldCurrentThread()
{
    return SwitchToThread();
}

int Thread::microSleep(unsigned int microsec)
{
#if _WIN32_WINNT < 0x0400 // simulate
    ::Sleep(microsec/1000);
    return 0;
#else
    HandleHolder sleepTimer(CreateWaitableTimer(NULL, TRUE, NULL));

    if( !sleepTimer )
      return -1;

    LARGE_INTEGER t;

    t.QuadPart= -(LONGLONG)microsec*10; // in 100ns units
                 // negative sign means relative,

    if (!SetWaitableTimer(sleepTimer.get(), &t, 0, NULL, NULL, 0))
    {
        return -1;
    }

    // Wait for the timer.
    if (WaitForSingleObject(sleepTimer.get(), INFINITE) != WAIT_OBJECT_0)
    {
        return -1;
    }
    return 0;
#endif
}


//-----------------------------------------------------------------------------
//
// Description:  Get the number of processors
//
int OpenThreads::GetNumberOfProcessors()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    return sysInfo.dwNumberOfProcessors;
}

int OpenThreads::SetProcessorAffinityOfCurrentThread(const Affinity& affinity)
{
    Thread::Init();

    Thread* thread = Thread::CurrentThread();
    if (thread)
    {
        return thread->setProcessorAffinity(affinity);
    }
    else
    {
		return SetThreadAffinity(GetCurrentThread(), affinity);
    }
}
