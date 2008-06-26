/* -*-c++-*- OpenThreads library, Copyright (C) 2008  The Open Thread Group
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

#include <OpenThreads/Atomic>

#if defined(_OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED)
# include <windows.h>
#endif

namespace OpenThreads {

#if defined(_OPENTHREADS_ATOMIC_USE_LIBRARY_ROUTINES)

// Non inline implementations for two special cases:
// * win32
// * i386 gcc
//
// On win32 we do not want to pull windows.h in the Atomic header.
// windows.h just pulls too much stuff that disturbs sources.
//
// On i386 gcc, we have that nice builtins available but only with some compiler
// architectural compiler flags enabled. This way we can make sure that the
// compiler flags are like we had them in the configure test. If the functions
// are inline, we do not need what compiler flags the Atomic header will see.

unsigned
Atomic::operator++()
{
#if defined(_OPENTHREADS_ATOMIC_USE_GCC_BUILTINS)
    return __sync_add_and_fetch(&_value, 1);
#elif defined(_OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED)
    return InterlockedIncrement(&_value);
#else
# error This implementation should happen inline in the include file
#endif
}

unsigned
Atomic::operator--()
{
#if defined(_OPENTHREADS_ATOMIC_USE_GCC_BUILTINS)
    return __sync_sub_and_fetch(&_value, 1);
#elif defined(_OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED)
    return InterlockedDecrement(&_value);
#else
# error This implementation should happen inline in the include file
#endif
}

Atomic::operator unsigned() const
{
#if defined(_OPENTHREADS_ATOMIC_USE_GCC_BUILTINS)
    __sync_synchronize();
    return _value;
#elif defined(_OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED)
    MemoryBarrier();
    return static_cast<unsigned const volatile &>(_value);
#else
# error This implementation should happen inline in the include file
#endif
}

bool
AtomicPtr::assign(void* ptrNew, const void* const ptrOld)
{
#if defined(_OPENTHREADS_ATOMIC_USE_GCC_BUILTINS)
    return __sync_bool_compare_and_swap(&_ptr, ptrOld, ptrNew);
#elif defined(_OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED)
    return ptrOld == InterlockedCompareExchangePointer((PVOID volatile*)&_ptr, (PVOID)ptrNew, (PVOID)ptrOld);
#else
# error This implementation should happen inline in the include file
#endif
}

void*
AtomicPtr::get() const
{
#if defined(_OPENTHREADS_ATOMIC_USE_GCC_BUILTINS)
    __sync_synchronize();
    return _ptr;
#elif defined(_OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED)
    MemoryBarrier();
    return _ptr;
#else
# error This implementation should happen inline in the include file
#endif
}

#endif // defined(_OPENTHREADS_ATOMIC_USE_LIBRARY_ROUTINES)

}
