/* **************************************************************************
 * OpenSceneGraph loader for Terrapage format database
 *
 * That loader is redistributed under the terms listed on Terrain Experts
 * website (www.terrex.com/www/pages/technology/technologypage.htm)
 *
 * "TerraPage is provided as an Open Source format for use by anyone...
 * We supply the TerraPage C++ source code free of charge.  Anyone
 * can use it and redistribute it as needed (including our competitors).
 * We do, however, ask that you keep the TERREX copyrights intact."
 *
 * Copyright Terrain Experts Inc. 1999.
 * All Rights Reserved.
 *
 *****************************************************************************/

#ifndef _TRPAGEMANAGER_H_
#define _TRPAGEMANAGER_H_

#include <string>

#ifndef WIN32
#include <pthread.h>
#endif

#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osgTXP/Export.h>

#include <osgTXP/trpage_geom.h>
#include <osgTXP/trpage_read.h>
#include <osgTXP/trpage_write.h>
#include <osgTXP/trpage_scene.h>
#include <osgTXP/trpage_managers.h>
#include <osgTXP/WaitBlock.h>

namespace txp
{
	/* Thread Identifier
		Fill this in for your specific platform.
		Should be water ID you use for threads.
	 */
#if defined(_WIN32)
	typedef HANDLE ThreadID;
	typedef HANDLE ThreadMutex;
	typedef HANDLE ThreadEvent;
#endif

#if !defined(ThreadID)
	/*
	// Stubs to make it compile
	typedef int ThreadID;
	typedef int ThreadMutex;
	typedef int ThreadEvent;
	*/

	typedef pthread_t ThreadID;
	typedef pthread_mutex_t ThreadMutex;
	typedef osgTXP::WaitBlock ThreadEvent;

#endif

	/* OSG Page Manager
		This class handles the paging into 
	 */ 
	class OSGTXP_EXPORT OSGPageManager {
	public:
		/* Need a group to put things under and the archive to page.
			Also, optionally, a Page Manager (if you've made changes
			to the default one).
		 */
		OSGPageManager(TrPageArchive *,trpgPageManager *pageManage = NULL);
		~OSGPageManager();

		/* Unthreaded update
			Update viewer position and load a maximum of numTile before
			returning.
			Also unloads everything that needs it.
			Don't call this in threaded mode.
		 */
		bool UpdateNoThread(osg::Group *,double locX,double locY,int numTile=-1);

		/* Thread routines
			The thread will run in and around this object.  It can
			run in one of two modes:
			  ThreadFree - 
			  ThreadSync -
		 */
		typedef enum {ThreadNone,ThreadFree,ThreadSync} ThreadMode;

		// Retrieve the current threading mode
		ThreadMode GetThreadMode() { return threadMode; }

		// ----- Main thread routines -----
		// ----- Only call these from the main thread ----
		// Create a new thread in the given mode.
		bool StartThread(ThreadMode,ThreadID &newThread);

		/* If we're in ThreadFree mode, merge everything the paging
			thread has read in up to this point into the main scenegraph.
		 */
		bool MergeUpdateThread(osg::Group *);

		// Shut down the current paging thread.
		bool EndThread();

		// Update the viewer position
		void UpdatePositionThread(double locX,double locY);

		// ----- Paging Thread Routines ----
		// ----- Only call these from the paging thread ----

		// Called by the thread start function
		// Don't call this yourself
		bool ThreadLoop();

	protected:
		// Page Manager we'll use is ours (i.e. delete it at the end)
		bool pageManageOurs;
		trpgPageManager *pageManage;
		// Archive to page from
		TrPageArchive *archive;
		// Database origin
		double originX,originY;

		/* Thread specific data.
		 */

		// ID of created thread and whether it's valid
		ThreadMode threadMode;
		ThreadID threadID;

		// Used to notify the paging thread when the location changes
		ThreadEvent locationChangeEvent;

		// Lock for the location and location itself
		ThreadMutex locationMutex;
		bool positionValid;
		double locX,locY;

		// Lock for the change lists (merge, unhook, delete)
		ThreadMutex changeListMutex;
		// Merge list is filled in by the paging thread.
		std::vector<osg::Group *> toMerge;
		std::vector<osg::Group *> toMergeParent;
		// Unhook list is filled in by the paging thread
		std::vector<osg::Group *> toUnhook;
		// Main thread moves groups to the delete list as soon as they are unhooked
		std::vector<osg::Group *> toDelete;
	};
};

#endif
