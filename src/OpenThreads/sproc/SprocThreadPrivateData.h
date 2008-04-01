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
//
// SprocThreadPrivateData.h - private data for sproc thread
// ~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef _SPROCTHREADPRIVATEDATA_H_
#define _SPROCTHREADPRIVATEDATA_H_

#include <sys/types.h>
#include <ulocks.h>
#include <stack>

#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include "SprocThreadPrivateActions.h"

namespace OpenThreads {

class SprocThreadPrivateData {
    
    friend class Thread;

    friend class ThreadPrivateActions;

private:
    
    struct CancelFuncStruct {
	
	void (*routine)(void *);
	void *arg;
    };

    SprocThreadPrivateData() {};

    virtual ~SprocThreadPrivateData() {};

    volatile unsigned int stackSize;

    volatile bool stackSizeLocked;

    volatile bool isRunning;

    Block threadStartedBlock;

    volatile bool isCanceled;

    volatile bool idSet;

    volatile bool cancelActive;

    volatile bool detached;

    volatile bool dieFlag;

    volatile Thread::ThreadPriority threadPriority;
    
    volatile Thread::ThreadPolicy threadPolicy;

    volatile pid_t pid;

    volatile int uniqueId;

    std::stack<CancelFuncStruct> cancelFuncStack; 

    static int nextId;

};

}

#endif // !_SPROCTHREADPRIVATEDATA_H_
