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
// Win32PrivateData.h - Private data structure for Thread
// ~~~~~~~~~~~~~~~~~~~~~
#ifndef _Win32PRIVATEDATA_H_
#define _Win32PRIVATEDATA_H_

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400
#include <windows.h>
#endif

#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include "HandleHolder.h"

namespace OpenThreads {

    class Win32ThreadPrivateData {
    //-------------------------------------------------------------------------
    // We're friendly to Thread, so it can use our data.
    //
    friend class Thread;
    //-------------------------------------------------------------------------
    // We're friendly to Win32PrivateActions, so it can get at some 
    // variables.
    //
    friend class ThreadPrivateActions;

private:

    Win32ThreadPrivateData() {};
    ~Win32ThreadPrivateData();

    size_t stackSize;
    bool isRunning;

    Block threadStartedBlock;

    int  cancelMode; // 0 - deffered (default) 1-asynch 2-disabled  

    bool detached;

    Thread::ThreadPriority threadPriority;

    Thread::ThreadPolicy threadPolicy;

    HandleHolder tid;

    int uniqueId;

    int cpunum;

public:

    HandleHolder cancelEvent;

    struct TlsHolder{ // thread local storage slot
        DWORD getId()
        {
            static bool initialized = false;
            if (!initialized) {
                ID = TlsAlloc();
                initialized = true;
            }
            return ID;
        }
        ~TlsHolder(){
            TlsFree(ID);
        }
    private:
        DWORD ID;
    };

    static TlsHolder TLS;

};

DWORD cooperativeWait(HANDLE waitHandle, unsigned long timeout);


}






#endif // !_PTHREADPRIVATEDATA_H_



