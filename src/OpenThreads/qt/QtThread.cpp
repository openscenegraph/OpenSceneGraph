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
// QtThread.cpp - C++ Thread class built on top of Qt threads.
// ~~~~~~~~~~~

#include "QtThreadPrivateData.h"
#include <QCoreApplication>
#include <iostream>

using namespace OpenThreads;

//-----------------------------------------------------------------------------
// Initialize thread master priority level
//
Thread::ThreadPriority Thread::s_masterThreadPriority = Thread::THREAD_PRIORITY_DEFAULT;
bool Thread::s_isInitialized = false;

//----------------------------------------------------------------------------
//
// Description: Set the concurrency level (no-op)
//
// Use static public
//
int Thread::SetConcurrency(int concurrencyLevel)
{ return -1; }

//----------------------------------------------------------------------------
//
// Description: Get the concurrency level
//
// Use static public
//
int Thread::GetConcurrency()
{ return -1; }

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Thread::Thread()
{
    if (!s_isInitialized) Init();
    
    QtThreadPrivateData* pd = new QtThreadPrivateData(this);
    pd->isRunning = false;
    pd->detached = false;
    pd->cancelled = false;
    pd->cancelMode = 0;
    pd->uniqueId = QtThreadPrivateData::createUniqueID();
    pd->cpunum = -1;
    pd->stackSize = 0;
    pd->threadPolicy = Thread::THREAD_SCHEDULE_DEFAULT;
    pd->threadPriority = Thread::THREAD_PRIORITY_DEFAULT;
    
    _prvData = static_cast<void*>(pd);
}

//----------------------------------------------------------------------------
//
// Description: Destructor
//
// Use: public.
//
Thread::~Thread()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    if (pd->isRunning)
    {
        std::cout<<"Error: Thread "<< this <<" still running in destructor"<<std::endl;
        cancel();
    }
    delete pd;
    _prvData = 0;
}

Thread* Thread::CurrentThread()
{
    if (!s_isInitialized) Thread::Init();
    
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(QThread::currentThread());
    return (pd ? pd->getMasterThread() : 0);
}

//-----------------------------------------------------------------------------
//
// Description: Initialize Threading
//
// Use: public.
//
void Thread::Init()
{
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
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
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
    return (size_t)QCoreApplication::applicationPid();
}

//-----------------------------------------------------------------------------
//
// Description: Determine if the thread is running
//
// Use: public
//
bool Thread::isRunning()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    return pd->isRunning;
}

//-----------------------------------------------------------------------------
//
// Description: Start the thread.
//
// Use: public
//
int Thread::start()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->threadStartedBlock.reset();
    
    pd->setStackSize( pd->stackSize );
    pd->start();
    
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
int Thread::detach()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->detached = true;
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Join the thread.
//
// Use: public
//
int Thread::join()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    if (pd->detached) return -1;
    return pd->wait() ? 0 : -1;
}

//-----------------------------------------------------------------------------
//
// Description: test the cancel state of the thread.
//
// Use: public
//
int Thread::testCancel()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    if (!pd->cancelled)
        return 0;
    
    if (pd->cancelMode == 2)
        return -1;
    
    if (pd!=QThread::currentThread())
        return -1;
    
    throw QtThreadCanceled();
}

//-----------------------------------------------------------------------------
//
// Description: Cancel the thread.
//
// Use: public
//
int Thread::cancel()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    if (pd->isRunning)
    {
        if (pd->cancelMode == 2)
            return -1;
        
        pd->cancelled = true;
        if (pd->cancelMode == 1)
        {
            pd->isRunning = false;
            pd->terminate();
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel at the next convenient point.
//
// Use: public
//
int Thread::setCancelModeDeferred()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->cancelMode = 0;
    pd->setAsynchronousTermination(false);
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: set the thread to cancel immediately
//
// Use: public
//
int Thread::setCancelModeAsynchronous()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->cancelMode = 1;
    pd->setAsynchronousTermination(true);
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Disable cancelibility
//
// Use: public
//
int Thread::setCancelModeDisable()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->cancelMode = 2;
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's schedule priority (if able)
//
// Use: public
//
int Thread::setSchedulePriority(ThreadPriority priority)
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->threadPriority = priority;
    
    if (pd->isRunning)
        pd->applyPriority();
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Get the thread's schedule priority (if able)
//
// Use: public
//
int Thread::getSchedulePriority()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
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
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->threadPolicy = policy;
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's scheduling policy (if able)
//
// Use: public
//
int Thread::getSchedulePolicy()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    return pd->threadPolicy;
}

//-----------------------------------------------------------------------------
//
// Description: Set the thread's desired stack size
//
// Use: public
//
int Thread::setStackSize(size_t stackSize)
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    if (pd->isRunning) return 13;  // return EACESS
    else pd->stackSize = stackSize;
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
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    return pd->stackSize;
}

//-----------------------------------------------------------------------------
//
// Description:  set processor affinity for the thread
//
// Use: public
//
int Thread::setProcessorAffinity(unsigned int cpunum)
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    pd->cpunum = cpunum;
    if (!pd->isRunning) return 0;
    
    // FIXME:
    // Qt doesn't have a platform-independent thread affinity method at present.
    // Does it automatically configure threads on different processors, or we have to do it ourselves?
    return -1;
}

//-----------------------------------------------------------------------------
//
// Description:  Print the thread's scheduling information to stdout.
//
// Use: public
//
void Thread::printSchedulingInfo()
{
    QtThreadPrivateData* pd = static_cast<QtThreadPrivateData*>(_prvData);
    std::cout << "Thread "<< pd->getMasterThread() <<" priority: ";
    
    switch (pd->threadPriority)
    {
    case Thread::THREAD_PRIORITY_MAX:
        std::cout << "MAXIMAL" << std::endl;
        break;
    case Thread::THREAD_PRIORITY_HIGH:
        std::cout << "HIGH" << std::endl;
        break;
    case Thread::THREAD_PRIORITY_DEFAULT:
    case Thread::THREAD_PRIORITY_NOMINAL:
        std::cout << "NORMAL" << std::endl;
        break;
    case Thread::THREAD_PRIORITY_LOW:
        std::cout << "LOW" << std::endl;
        break;
    case Thread::THREAD_PRIORITY_MIN:
        std::cout << "MINIMAL" << std::endl;
        break;
    }
}

//-----------------------------------------------------------------------------
//
// Description:  Yield the processor
//
// Use: protected
//
int Thread::YieldCurrentThread()
{
    QThread::yieldCurrentThread();
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description:  sleep
//
// Use: public
//
int Thread::microSleep(unsigned int microsec)
{
    QtThreadPrivateData::microSleep(microsec);
    return 0;
}

//-----------------------------------------------------------------------------
//
// Description:  Get the number of processors
//
int OpenThreads::GetNumberOfProcessors()
{
    return QThread::idealThreadCount();
}

//-----------------------------------------------------------------------------
//
// Description:  set processor affinity for current thread
//
int OpenThreads::SetProcessorAffinityOfCurrentThread(unsigned int cpunum)
{
    if (cpunum<0) return -1;
    
    Thread::Init();
    Thread* thread = Thread::CurrentThread();
    if (thread)
        return thread->setProcessorAffinity(cpunum);
    else
        return -1;
}
