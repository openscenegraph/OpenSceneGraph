# Check for availability of atomic operations 
# This module defines
# OPENTHREADS_HAVE_ATOMIC_OPS

INCLUDE(CheckCXXSourceRuns)

# Do step by step checking, 
CHECK_CXX_SOURCE_RUNS("
#include <cstdlib>

int main()
{
   unsigned value = 0;
   void* ptr = &value;
   __sync_add_and_fetch(&value, 1);
   __sync_synchronize();
   __sync_sub_and_fetch(&value, 1);
   if (!__sync_bool_compare_and_swap(&value, 0, 1))
      return EXIT_FAILURE;
   
   if (!__sync_bool_compare_and_swap(&ptr, ptr, ptr))
      return EXIT_FAILURE;

   return EXIT_SUCCESS;
}
" _OPENTHREADS_ATOMIC_USE_GCC_BUILTINS)

CHECK_CXX_SOURCE_RUNS("
#include <stdlib.h>

int main(int, const char**)
{
   unsigned value = 0;
   void* ptr = &value;
   __add_and_fetch(&value, 1);
   __synchronize(value);
   __sub_and_fetch(&value, 1);
   if (!__compare_and_swap(&value, 0, 1))
      return EXIT_FAILURE;
   
   if (!__compare_and_swap((unsigned long*)&ptr, (unsigned long)ptr, (unsigned long)ptr))
      return EXIT_FAILURE;

   return EXIT_SUCCESS;
}
" _OPENTHREADS_ATOMIC_USE_MIPOSPRO_BUILTINS)

CHECK_CXX_SOURCE_RUNS("
#include <atomic.h>
#include <cstdlib>

int main(int, const char**)
{
   uint_t value = 0;
   void* ptr = &value;
   atomic_inc_uint_nv(&value);
   membar_consumer();
   atomic_dec_uint_nv(&value);
   if (0 != atomic_cas_uint(&value, 0, 1))
      return EXIT_FAILURE;
   
   if (ptr != atomic_cas_ptr(&ptr, ptr, ptr))
      return EXIT_FAILURE;

   return EXIT_SUCCESS;
}
" _OPENTHREADS_ATOMIC_USE_SUN)

CHECK_CXX_SOURCE_RUNS("
#include <windows.h>
#include <intrin.h>
#include <cstdlib>

#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedXor)

int main(int, const char**)
{
   volatile long value = 0;
   long data = 0;
   long* volatile ptr = &data;

   InterlockedIncrement(&value);
   MemoryBarrier();
   InterlockedDecrement(&value);

   if (0 != InterlockedCompareExchange(&value, 1, 0))
      return EXIT_FAILURE;

   if (ptr != InterlockedCompareExchangePointer((PVOID volatile*)&ptr, (PVOID)ptr, (PVOID)ptr))
      return EXIT_FAILURE;

   return EXIT_SUCCESS;
}
" _OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED)

CHECK_CXX_SOURCE_RUNS("
#include <libkern/OSAtomic.h>

int main()
{
  volatile int32_t value = 0;
  long data = 0;
  long * volatile ptr = &data;
  
  OSAtomicIncrement32(&value);
  OSMemoryBarrier();
  OSAtomicDecrement32(&value);
  OSAtomicCompareAndSwapInt(value, 1, &value);
  OSAtomicCompareAndSwapPtr(ptr, ptr, (void * volatile *)&ptr);
}
" _OPENTHREADS_ATOMIC_USE_BSD_ATOMIC)


IF(NOT _OPENTHREADS_ATOMIC_USE_GCC_BUILTINS AND NOT _OPENTHREADS_ATOMIC_USE_MIPOSPRO_BUILTINS AND NOT _OPENTHREADS_ATOMIC_USE_SUN AND NOT _OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED AND NOT _OPENTHREADS_ATOMIC_USE_BSD_ATOMIC)
  SET(_OPENTHREADS_ATOMIC_USE_MUTEX 1)
ENDIF(NOT _OPENTHREADS_ATOMIC_USE_GCC_BUILTINS AND NOT _OPENTHREADS_ATOMIC_USE_MIPOSPRO_BUILTINS AND NOT _OPENTHREADS_ATOMIC_USE_SUN AND NOT _OPENTHREADS_ATOMIC_USE_WIN32_INTERLOCKED AND NOT _OPENTHREADS_ATOMIC_USE_BSD_ATOMIC)
