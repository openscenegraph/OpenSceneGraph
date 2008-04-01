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
// Win32MutexPrivateData.h - Private data structure for Mutex
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//

#ifndef _Win32MUTEXPRIVATEDATA_H_
#define _Win32MUTEXPRIVATEDATA_H_


#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400  // was missing : adegli
#include <windows.h>
#endif
namespace OpenThreads {

class Win32MutexPrivateData {
    friend class Mutex;
    friend class Condition;

private:

    Win32MutexPrivateData();

    ~Win32MutexPrivateData();
#define USE_CRITICAL_SECTION
#ifdef USE_CRITICAL_SECTION
    CRITICAL_SECTION _cs;
#else
    volatile unsigned long mutex;
#endif

};

}

#endif // !_Win32MUTEXPRIVATEDATA_H_





