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

#include <osg/Group>
#include <osg/Object>
#include <osg/Node>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#include "trpage_geom.h"
#include "trpage_read.h"
#include "trpage_write.h"
#include "trpage_scene.h"
#include "trpage_managers.h"
#include "WaitBlock.h"
#include "TrPageArchive.h"

#include <string>

// Dec 2002, Robert Osfield -> comment out now, as we actually do want to delete in the main thread
// as the deletion of the display and texture object isn't thread safe.
// Jan 2002, Robert osfield -> comment back in now, as I've changed the way the
// glDelete part is handled so that its much less likely to hit a race condtion,
// in theory its still not thread safe, but a point swap should be much faster
// and less likely to encounter a race condition on the static caches of display
// and texture object lists.
// #define USE_THREADLOOP_DELETE

namespace txp
{
	/* Thread Identifier
		Fill this in for your specific platform.
		Should be water ID you use for threads.
	 */
	typedef int ThreadID;
	typedef OpenThreads::Mutex  ThreadMutex;
	typedef osgTXP::WaitBlock ThreadEvent;

	class OSGPageManager;
	class PagingThread: public OpenThreads::Thread{
	public:
		typedef OpenThreads::Thread super;
		PagingThread():pager(NULL)
		{
		}

		virtual void run();

		OSGPageManager *pager;
	private:
		volatile bool canceled;
	};

	/* OSG Page Manager
		This class handles the paging into 
	 */ 
	class OSGPageManager{
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
		bool ThreadLoop(PagingThread* t);
		
		// Load One tile
		// @param tile managed tile
		// @return tileGroup osg::Group representing managed tile
		void LoadOneTile(trpgManagedTile* tile);

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
		PagingThread pagingThread;

		// Used to notify the paging thread when the location changes
		ThreadEvent locationChangeEvent;

		// Lock for the location and location itself
		ThreadMutex locationMutex;
		bool positionValid;
		double locX,locY;

		// Lock for the change lists (merge, unhook, delete)
		ThreadMutex changeListMutex;
		// Merge list is filled in by the paging thread.
                typedef std::pair< osg::ref_ptr<osg::Group> , osg::ref_ptr<osg::Group> > MergePair;
		std::vector< MergePair > toMerge;
		// no need for that 
		// std::vector<osg::Group *> toMergeParent;
		// Unhook list is filled in by the paging thread
		std::vector<osg::Group *> toUnhook;

#ifdef USE_THREADLOOP_DELETE
		// Main thread moves groups to the delete list as soon as they are unhooked
		std::vector<osg::Group *> toDelete;
#endif            
	};

};

#endif
