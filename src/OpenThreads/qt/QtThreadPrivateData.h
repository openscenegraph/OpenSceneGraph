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
// QtThreadPrivateData.h - Private data structure for Thread
// ~~~~~~~~~~~~~~~~~~~~~

#ifndef _QTTHREADPRIVATEDATA_H_
#define _QTTHREADPRIVATEDATA_H_

#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include <QThread>

struct QtThreadCanceled {};

class QtThreadPrivateData : public QThread
{
public:
    static int createUniqueID()
    {
        static int nextID = 0;
        return nextID++;
    }
    
    static void microSleep(unsigned int microsec)
    {
        usleep(microsec);
    }
    
    QtThreadPrivateData(OpenThreads::Thread* master)
    : QThread(0), _master(master) {}
    
    virtual ~QtThreadPrivateData() {}
    
    OpenThreads::Thread* getMasterThread() { return _master; }
    
    void setAsynchronousTermination( bool enabled )
    { setTerminationEnabled(enabled); }
    
    void applyPriority()
    {
        Priority prio = NormalPriority;
        switch (threadPriority)
        {
        case OpenThreads::Thread::THREAD_PRIORITY_MAX:
            prio = HighestPriority;
            break;
        case OpenThreads::Thread::THREAD_PRIORITY_HIGH:
            prio = HighPriority;
            break;
        case OpenThreads::Thread::THREAD_PRIORITY_NOMINAL:
            prio = NormalPriority;
            break;
        case OpenThreads::Thread::THREAD_PRIORITY_LOW:
            prio = LowPriority;
            break;
        case OpenThreads::Thread::THREAD_PRIORITY_MIN:
            prio = IdlePriority;
            break;
        default:
            break;
        }
        setPriority( prio );
    }
    
    virtual void run()
    {
        applyPriority();
        isRunning = true;
        threadStartedBlock.release();
        
        if (_master)
        {
            try
            {
                _master->run();
            }
            catch (QtThreadCanceled&)
            {
                try { _master->cancelCleanup(); }
                catch (...) {}
            }
            catch (...)
            {}
        }
        isRunning = false;
    }
    
    OpenThreads::Thread::ThreadPolicy threadPolicy;
    OpenThreads::Thread::ThreadPriority threadPriority;
    unsigned int stackSize;
    int cpunum;
    int uniqueId;
    int cancelMode;  // 0 - deffered; 1 - asynchronous; 2 - disabled
    bool cancelled;
    bool detached;
    bool isRunning;
    
    OpenThreads::Block threadStartedBlock;
    
private:
    OpenThreads::Thread* _master;
};

#endif
