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

#ifndef _WIN32VODITIONPRODUCER_CONDITION
#define PRODUCER_CONDITION

#ifdef WIN32

#include "Mutex.h"

namespace OpenThreads {

class Win32ConditionImpl
{
public:
	/// number of waiters.
	long waiters_;

	Condition(long max = 0L)
	{
		waiters_ = 0;
		sema_ = CreateSemaphore(NULL,0,0x7fffffff,NULL);
		waiters_done_ = CreateEvent(NULL,FALSE,FALSE,NULL);
	}

	~Condition()
	{
	//	CloseHandle(sema_);
	//	CloseHandle(waiters_done_);
	}

	inline int broadcast ()
	{
		 waiters_lock_.lock();
		 int have_waiters = 0;

		if (waiters_ > 0)
		{
	      // We are broadcasting, even if there is just one waiter...
		  // Record the fact that we are broadcasting.  This helps the
		  // wait() method know how to optimize itself.  Be sure to
		  // set this with the <waiters_lock_> held.
	      was_broadcast_ = 1;
		  have_waiters = 1;
		}
		waiters_lock_.unlock();
		
		int result = 0;
		if (have_waiters)
	    {
			// Wake up all the waiters.
			ReleaseSemaphore(sema_,waiters_,NULL);
			WaitForSingleObject(waiters_done_,INFINITE) ;
			// This is okay, even without the <waiters_lock_> held because
			// no other waiter threads can wake up to access it.
			was_broadcast_ = 0;
	    }
		return result;
	}

	inline int wait (Mutex& external_mutex)
	{
		// Prevent race conditions on the <waiters_> count.
		waiters_lock_.lock();
		waiters_++;
		waiters_lock_.unlock();
		
		int result = 0;
		
        external_mutex.unlock();
		
		DWORD dwResult = WaitForSingleObject(sema_,INFINITE);
		if(dwResult != WAIT_OBJECT_0)
			result = (int)dwResult;
		
		// Reacquire lock to avoid race conditions on the <waiters_> count.
		waiters_lock_.lock();
		
		// We're ready to return, so there's one less waiter.
		waiters_--;
		
		int last_waiter = was_broadcast_ && waiters_ == 0;
		
		// Release the lock so that other collaborating threads can make
		// progress.
		waiters_lock_.unlock();
		
		if (result != -1 && last_waiter)
			SetEvent(waiters_done_);
		
		external_mutex.lock();
		
		return result;
	}


protected:

  /// Serialize access to the waiters count.
  Mutex waiters_lock_;

  /// Queue up threads waiting for the condition to become signaled.
  HANDLE sema_;
  /**
   * An auto reset event used by the broadcast/signal thread to wait
   * for the waiting thread(s) to wake up and get a chance at the
   * semaphore.
   */
  HANDLE waiters_done_;

  /// Keeps track of whether we were broadcasting or just signaling.
  size_t was_broadcast_;

};
#else
#include <pthread.h>

namespace Producer {

class PR_EXPORT Condition
{
public:
	/// number of waiters.
	Condition(long max)
	{
        pthread_cond_init( &_cond, 0L );
	}

	~Condition()
	{
	}

	inline int broadcast ()
	{
		return pthread_cond_broadcast(&_cond);
	}

	inline int wait (Mutex& external_mutex)
	{
		return pthread_cond_wait(&_cond);
	}


protected:
   pthread_cond_t _cond;
};
#endif
}
#endif