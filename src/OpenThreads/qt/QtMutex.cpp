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
// QtMutex.cpp - C++ Mutex class built on top of Qt threads.
// ~~~~~~~~~~~

#include "QtMutexPrivateData.h"
#include <iostream>

using namespace OpenThreads;

//----------------------------------------------------------------------------
//
// Decription: Constructor
//
// Use: public.
//
Mutex::Mutex(MutexType type)
:   _mutexType(type)
{
    QMutex::RecursionMode mode = QMutex::NonRecursive;
    if (type == MUTEX_RECURSIVE) mode = QMutex::Recursive;
    
    QtMutexPrivateData* pd = new QtMutexPrivateData(mode);
    _prvData = static_cast<void *>(pd);
}

//----------------------------------------------------------------------------
//
// Decription: Destructor
//
// Use: public.
//
Mutex::~Mutex()
{
    QtMutexPrivateData* pd = static_cast<QtMutexPrivateData*>(_prvData);
    delete pd;
    _prvData = 0;
}

//----------------------------------------------------------------------------
//
// Decription: lock the mutex
//
// Use: public.
//
int Mutex::lock()
{
    QtMutexPrivateData* pd = static_cast<QtMutexPrivateData*>(_prvData);
    pd->lock();
    return 0;
}

//----------------------------------------------------------------------------
//
// Decription: unlock the mutex
//
// Use: public.
//
int Mutex::unlock()
{
    QtMutexPrivateData* pd = static_cast<QtMutexPrivateData*>(_prvData);
    pd->unlock();
    return 0;
}

//----------------------------------------------------------------------------
//
// Decription: test if the mutex may be locked
//
// Use: public.
//
int Mutex::trylock()
{
    QtMutexPrivateData* pd = static_cast<QtMutexPrivateData*>(_prvData);
    return pd->tryLock() ? 0 : 1;
}
