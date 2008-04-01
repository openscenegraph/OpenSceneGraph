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
// SprocThreadPrivateActions.c++ - Thread private actions for sprocs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <OpenThreads/Thread>
#include <list>

#ifndef SPROC_THREAD_PRIVATE_ACTIONS
#define SPROC_THREAD_PRIVATE_ACTIONS

namespace OpenThreads {

    class ThreadPrivateActions {

	//-------------------------------------------------------------------------
	// We're friendly to Thread, so it can issue the methods.
	//
	friend class Thread;
	
    public:

	static void ThreadCancelTest();
	
	static void PushCancelFunction(void (*routine)(void *), void *arg);

	static void PopCancelFunction();

    private:
	
	static bool *GetDeathFlag(Thread *thread);
	
	static Thread *GetThread(pid_t thread_id);
	
	static void ThreadCancelHandler(int sigid);
	
	//-------------------------------------------------------------------------
	// standard start routine.
	//
	static void StartThread(void *data);
	
	static void AddThread(Thread *thread);
	
	static void RemoveThread(Thread *thread);
	
	static void PrintThreadSchedulingInfo(Thread *thread);
	
	static int SetThreadSchedulingParams(Thread *thread);
	
    private:

	static std::list<Thread *> s_threadList;
	
    };
    
}

#endif // !SPROC_THREAD_PRIVATE_ACTIONS
