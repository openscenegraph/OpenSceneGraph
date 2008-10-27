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
// Win32BarrierPrivateData.h - private data structure for barrier
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
#ifndef _Win32BARRIERPRIVATEDATA_H_
#define _Win32BARRIERPRIVATEDATA_H_

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400
#include <windows.h>
#endif

#include <OpenThreads/Mutex>
#include <OpenThreads/Condition>

namespace OpenThreads {

class Barrier;

class Win32BarrierPrivateData {
    friend class Barrier;
private:
    Win32BarrierPrivateData() {};
    ~Win32BarrierPrivateData();

    Condition cond;            // cv for waiters at barrier

    Mutex    lock;            // mutex for waiters at barrier

    volatile int       maxcnt;          // number of threads to wait for

    volatile int       cnt;             // number of waiting threads

    volatile int       phase;           // flag to seperate two barriers


};







}







#endif //_Win32BARRIERPRIVATEDATA_H_



