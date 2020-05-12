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
// PThreadPrivateData.h - Private data structure for Thread
// ~~~~~~~~~~~~~~~~~~~~~

#ifndef _PTHREADPRIVATEDATA_H_
#define _PTHREADPRIVATEDATA_H_

#include <pthread.h>
#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include <OpenThreads/Atomic>

namespace OpenThreads {

class PThreadPrivateData {

    //-------------------------------------------------------------------------
    // We're friendly to Thread, so it can use our data.
    //
    friend class Thread;

    //-------------------------------------------------------------------------
    // We're friendly to PThreadPrivateActions, so it can get at some
    // variables.
    //
    friend class ThreadPrivateActions;

private:

    PThreadPrivateData()
    {
        stackSize = 0;
        stackSizeLocked = false;
        idSet = false;
        setRunning(false);
        isCanceled = false;
        tid = 0;
        uniqueId = 0;
        threadPriority = Thread::THREAD_PRIORITY_DEFAULT;
        threadPolicy = Thread::THREAD_SCHEDULE_DEFAULT;
    };

    virtual ~PThreadPrivateData() {};

    volatile unsigned int stackSize;

    volatile bool stackSizeLocked;

    void setRunning(bool flag) { _isRunning.exchange(flag); }
    bool isRunning() const { return _isRunning!=0; }

    Atomic _isRunning;

    Block threadStartedBlock;

    volatile bool isCanceled;

    volatile bool idSet;

    volatile Thread::ThreadPriority threadPriority;

    volatile Thread::ThreadPolicy threadPolicy;

    pthread_t tid;

    size_t uniqueId;

    Affinity affinity;

    static pthread_key_t s_tls_key;

};

}

#endif // !_PTHREADPRIVATEDATA_H_
