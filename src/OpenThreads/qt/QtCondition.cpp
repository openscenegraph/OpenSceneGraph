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
// QtCondition.cpp - C++ Condition class built on top of Qt threads.
// ~~~~~~~~~~~

#include "QtMutexPrivateData.h"
#include "QtConditionPrivateData.h"
#include <iostream>

using namespace OpenThreads;

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Condition::Condition()
{
    QtConditionPrivateData* pd = new QtConditionPrivateData;
    _prvData = static_cast<void *>(pd);
}

//----------------------------------------------------------------------------
//
// Decription: Destructor
//
// Use: public.
//
Condition::~Condition()
{
    QtConditionPrivateData* pd = static_cast<QtConditionPrivateData*>(_prvData);
    delete pd;
    _prvData = 0;
}

//----------------------------------------------------------------------------
//
// Decription: wait on a condition
//
// Use: public.
//
int Condition::wait(Mutex *mutex)
{
    QtMutexPrivateData* mpd = static_cast<QtMutexPrivateData*>(mutex->_prvData);
    QtConditionPrivateData* pd = static_cast<QtConditionPrivateData*>(_prvData);
    return pd->wait(mpd) ? 0 : 1;
}

//----------------------------------------------------------------------------
//
// Decription: wait on a condition, for a specified period of time
//
// Use: public.
//
int Condition::wait(Mutex *mutex, unsigned long int ms)
{
    QtMutexPrivateData* mpd = static_cast<QtMutexPrivateData*>(mutex->_prvData);
    QtConditionPrivateData* pd = static_cast<QtConditionPrivateData*>(_prvData);
    return pd->wait(mpd, ms) ? 0 : 1;
}

//----------------------------------------------------------------------------
//
// Decription: signal a thread to wake up.
//
// Use: public.
//
int Condition::signal()
{
    QtConditionPrivateData* pd = static_cast<QtConditionPrivateData*>(_prvData);
    pd->wakeOne();
    return 0;
}

//----------------------------------------------------------------------------
//
// Decription: signal many threads to wake up.
//
// Use: public.
//
int Condition::broadcast()
{
    QtConditionPrivateData* pd = static_cast<QtConditionPrivateData*>(_prvData);
    pd->wakeAll();
    return 0;
}
