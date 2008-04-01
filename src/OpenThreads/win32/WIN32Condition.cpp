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
// Win32Condition.c++ - C++ Condition class built on top of posix threads.
// ~~~~~~~~~~~~~~~~~~~~
//
#include <OpenThreads/Condition>
#include  <OpenThreads/Thread>
#include "Win32ConditionPrivateData.h"

using namespace OpenThreads;
Win32ConditionPrivateData::~Win32ConditionPrivateData()
{
}

//----------------------------------------------------------------------------
//
// Description: Constructor
//
// Use: public.
//
Condition::Condition() {
    Win32ConditionPrivateData *pd =
        new Win32ConditionPrivateData();
    _prvData = static_cast<void *>(pd);
}
//----------------------------------------------------------------------------
//
// Description: Destructor
//
// Use: public.
//
Condition::~Condition() {
    Win32ConditionPrivateData *pd =
       static_cast<Win32ConditionPrivateData *>(_prvData);

    delete pd;
}

//----------------------------------------------------------------------------
//
// Description: wait on a condition
//
// Use: public.
//
int Condition::wait(Mutex *mutex) {

    Win32ConditionPrivateData *pd =
        static_cast<Win32ConditionPrivateData *>(_prvData);

    return pd->wait(*mutex, INFINITE);
}
//----------------------------------------------------------------------------
//
// Description: wait on a condition, for a specified period of time
//
// Use: public.
//
int Condition::wait(Mutex *mutex, unsigned long ms) {

    Win32ConditionPrivateData *pd =
        static_cast<Win32ConditionPrivateData *>(_prvData);

    return pd->wait(*mutex, ms);
}
//----------------------------------------------------------------------------
//
// Description: signal a thread to wake up.
//
// Use: public.
//
int Condition::signal() {

    Win32ConditionPrivateData *pd =
        static_cast<Win32ConditionPrivateData *>(_prvData);
    return pd->signal();
}
//----------------------------------------------------------------------------
//
// Description: signal many threads to wake up.
//
// Use: public.
//
int Condition::broadcast() {

    Win32ConditionPrivateData *pd =
        static_cast<Win32ConditionPrivateData *>(_prvData);
    return pd->broadcast();
}
