#include <stdlib.h>
#include <stdio.h>

#if defined(__CYGWIN__)
#include <windows.h>
#include <unistd.h>
#include <pthread.h>
#elif defined(_WIN32)
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#include "TrPageArchive.h"
#include "trPagePageManager.h"
#include "trpage_print.h"

#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/Texture>

#include <osgDB/Registry>
#include <osgDB/FileUtils>

#include <iostream>

using namespace txp;
using namespace osg;

OSGPageManager::OSGPageManager(TrPageArchive *in_arch,trpgPageManager *in_pageManage)
{
    archive = in_arch;
    pageManage = in_pageManage;

    if (!in_arch)
        throw 1;

    if (pageManage)
        pageManageOurs = false;
    else {
        pageManage = new trpgPageManager();
        pageManage->SetPageDistFactor(1.2);
        pageManage->Init(archive);
        pageManageOurs = true;
    }

    // Location info we'll need later
    const trpgHeader *head = archive->GetHeader();
    trpg2dPoint sw,ne;
    head->GetExtents(sw,ne);
    originX = sw.x;
    originY = sw.y;

    threadMode = ThreadNone;
}

OSGPageManager::~OSGPageManager()
{
    if (threadMode != ThreadNone)
        EndThread();
    if (pageManageOurs)
        delete pageManage;
    pageManage = NULL;
}

/* Update
    Bring the paging up to date based on the given location.
    The location is in TerraPage's coordinate system, but must
    still be adjusted to the SW corner.
 */
bool OSGPageManager::UpdateNoThread(osg::Group *rootNode,double locX,double locY,int numTile)
{
    // Adjust to TerraPage coordinates
    double lx = locX-originX;
    double ly = locY-originY;

    /* Do that paging thing:
        - Tell the manager the new location
        - Iterate over the unloads
        - Iterate over the loads
     */
    trpg2dPoint loc;
    loc.x = lx;
    loc.y = ly;
    if (pageManage->SetLocation(loc)) {
//        printf("Location (%f,%f) resulted in changes.",loc.x,loc.y);
//        trpgFilePrintBuffer printBuf(stdout);
//        pageManage->Print(printBuf);
    }

    // Do the unloads
    trpgManagedTile *tile=NULL;
    while ((tile = pageManage->GetNextUnload())) {
        archive->UnLoadTile(pageManage,tile);
        pageManage->AckUnload();
    };

    // Decide how many loads to do per frame
    int loadCount=0;

    // Do the loads
    while ((tile = pageManage->GetNextLoad())) {
        archive->LoadTile(rootNode,pageManage,tile);
        pageManage->AckLoad();
        loadCount++;
        if (numTile > 0 && loadCount >= numTile)
            break;
    };

    return true;
}

// Mutex lock function
// --- Either thread ---
void osgLockMutex(ThreadMutex &mtx)
{
#if defined (_WIN32)
    WaitForSingleObject( mtx, INFINITE);   
#else
    pthread_mutex_lock( &mtx );
#endif
}

// Mutex unlock function
// --- Either thread ---
void osgUnLockMutex(ThreadMutex &mtx)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    ReleaseMutex(mtx);
#else
    pthread_mutex_unlock( &mtx );
#endif
}

// Wait for Event
// --- Either thread (only used in paging thread) ---
void osgWaitEvent(ThreadEvent &ev)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    WaitForSingleObject( ev, INFINITE);
#else
    ev.wait();
#endif
}

// Set Event (i.e. notify the other thread something's up)
// --- Either thread (only used in main thread) ---
void osgSetEvent(ThreadEvent &ev)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    SetEvent(ev);
#else
    ev.release();
#endif
}

// Windows specific thread function.
// This just fires up our own loop
// --- Paging Thread ---
#if defined(_WIN32) && !defined(__CYGWIN__)
DWORD WINAPI ThreadFunc( LPVOID lpParam ) 
{
    OSGPageManager *myPager = (OSGPageManager *)lpParam;
    myPager->ThreadLoop();

    return 0; 
}
#else // Pthreads, cause Pthreads is a POSIX standard and is EVERYWHERE else

void *ThreadFunc( void *data )
{
    OSGPageManager *myPager = static_cast<OSGPageManager *>(data);
    myPager->ThreadLoop();
    return 0L;
}
#endif

/* Start Thread
    This initialized
    --- Main Thread ---
 */
bool OSGPageManager::StartThread(ThreadMode mode,ThreadID &newThread)
{
    positionValid = false;

#if defined(_WIN32) && !defined(__CYGWIN__)
    // Create the event we'll use to wake up the pager thread when the location changes
    locationChangeEvent = CreateEvent(NULL,false,false,"Location Change Event");

    // Create the mutexes we'll need later
    changeListMutex = CreateMutex(NULL,false,"Change List Mutex");
    locationMutex = CreateMutex(NULL,false,"Location Mutex");

    {
        // Create the thread
        DWORD dwThreadId=0;
        LPVOID dwThrdParam = (LPVOID) this;

        threadID = newThread = CreateThread( 
            NULL,                        // default security attributes 
            0,                           // use default stack size  
            ThreadFunc,                  // thread function 
            dwThrdParam,                // argument to thread function 
            0,                           // use default creation flags 
            &dwThreadId);                // returns the thread identifier 
    }

    // Note: Should be optional
    // Set the priority low so this is only called when the other thread is idle
//    if (!SetThreadPriority(newThread,THREAD_PRIORITY_IDLE))
//        fprintf(stderr,"Couldn't set paging thread priority.\n");

    // Was successfull
    if (newThread != NULL)
        threadMode = mode;
#else
    //locationChangeEvent is self-initialized.
    pthread_mutex_init( &changeListMutex, 0L );
    pthread_mutex_init( &locationMutex, 0L );
    threadMode = mode;
    if( pthread_create( &newThread, 0L, ThreadFunc, (void *)this ) != 0 )
        threadMode = ThreadNone;
    threadID = newThread;
#endif
    return threadMode != ThreadNone;
}

/* End Thread
    Note: Do this
 */
bool OSGPageManager::EndThread()
{
    cancel = true;
    // claer the path for thred loop to finish 
    osgSetEvent(locationChangeEvent);
#ifdef _WIN32
    DWORD res = STILL_ACTIVE;
    while(res==STILL_ACTIVE){
        GetExitCodeThread(threadID,&res);
        Sleep(100);
    }
#else
    // Need a handle to the thread ID here.
    pthread_join( threadID, 0 );
#endif
    return true;
}

/* Thread Loop
    This method is the main loop for the pager when it's
    working in its own thread.
    --- Paging Thread ---
 */
bool OSGPageManager::ThreadLoop()
{
    // Note: Only works for free running thread
    if (threadMode != ThreadFree)
        throw 1;

    bool pagingActive = false;
    cancel = false;

    while (!cancel) {
        /* Here's how we do it:
            Wait for position change
              Update manager w/ position
              Form delete list
              Load tile (if needed)
              Add tile to merge list
         */
        // Position has already changed or we'll wait for it to do so
        osgWaitEvent(locationChangeEvent);
        double myLocX,myLocY;
        {
            osgLockMutex(locationMutex);
            myLocX = locX;
            myLocY = locY;
            osgUnLockMutex(locationMutex);
        }

        // Pass the new location on to page manager
        trpg2dPoint loc;
        loc.x = myLocX;
        loc.y = myLocY;
        if (pageManage->SetLocation(loc) || pagingActive) {
            // If there were changes, process them
            // Form the delete list first
            trpgManagedTile *tile=NULL;
            std::vector<osg::Group *> unhook;
            while ((tile = pageManage->GetNextUnload())) {
                unhook.push_back((Group *)(tile->GetLocalData()));
                pageManage->AckUnload();
            }

            // Tell the main thread to unhook tiles
            //  and get the groups to delete as well.
            std::vector <osg::Group *> nextDelete;
            {
                osgLockMutex(changeListMutex);
                // Add to the unhook list
                for (unsigned int ti=0;ti<unhook.size();ti++)
                    toUnhook.push_back(unhook[ti]);
                // Also get the list of deletions while we're here
#ifdef USE_THREADLOOP_DELETE
                nextDelete = toDelete;
                toDelete.clear();
#endif
                osgUnLockMutex(changeListMutex);
            }

#ifdef USE_THREADLOOP_DELETE
           // Unreference whatever we're supposed to delete
           for (unsigned int gi=0;gi<nextDelete.size();gi++)
                nextDelete[gi]->unref();
#endif

            // Now do a single load
           while( (tile = pageManage->GetNextLoad()) ) // Sasa's new code - more frame drops, but less missing tiles.
           //if( (tile = pageManage->GetNextLoad()) )  // original code (0.9.4 and before) - less frame drops, more missing tiles.
           {
               osg::Group *tileGroup=NULL;
               pagingActive = false;
               osg::Group *parentNode = NULL;
               tileGroup = archive->LoadTile(NULL,pageManage,tile,&parentNode);
               pageManage->AckLoad();
               if (tileGroup) {
#ifdef USE_THREADLOOP_DELETE
                   // Make an extra reference to it because we want it back for deletion
                   // RO, commenting out as we don't want to do delete here, we want it to happen in the merge thread.
                   tileGroup->ref();
#endif
               } 
               else 
               {
                   int x,y,lod;
                   tile->GetTileLoc(x,y,lod);
                   fprintf(stderr,"Failed to load tile (%d,%d,%d)\n",x,y,lod);
               }
               
               // Now add this tile to the merge list
               if (tileGroup) {
                   osgLockMutex(changeListMutex);
                   toMerge.push_back(tileGroup);
                   toMergeParent.push_back(parentNode);
                   osgUnLockMutex(changeListMutex);
               }
               // We're not necessarily done paging, we're just handing control back
               // It's likely we'll be back here
               pagingActive = true;
           }
        }
    }


    return true;
}

/* Update Position Thread
    This method updates the location for the paging thread to use.
    --- Main Thread ---
 */
void OSGPageManager::UpdatePositionThread(double inLocX,double inLocY)
{
    // Update the position
    {
        // Get the location mutex
        osgLockMutex(locationMutex);
        positionValid = true;
        locX = inLocX-originX;
        locY = inLocY-originY;
        osgUnLockMutex(locationMutex);
    }

    // Notify the paging thread there's been a position update
    osgSetEvent(locationChangeEvent);
}

class ReleaseTexturesAndDrawablesVisitor : public osg::NodeVisitor
{
public:
    ReleaseTexturesAndDrawablesVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }
    
    virtual void apply(osg::Node& node)
    {
        apply(node.getStateSet());

        traverse(node);
    }
    
    virtual void apply(osg::Geode& geode)
    {
        apply(geode.getStateSet());
    
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            apply(geode.getDrawable(i));
        }

        traverse(geode);
    }
    
    inline void apply(osg::StateSet* stateset)
    {
        if (stateset)
        {
            // search for the existance of any texture object attributes
            bool foundTextureState = false;
            osg::StateSet::TextureAttributeList& tal = stateset->getTextureAttributeList();
            for(osg::StateSet::TextureAttributeList::iterator itr=tal.begin();
                itr!=tal.end() && !foundTextureState;
                ++itr)
            {
                osg::StateSet::AttributeList& al = *itr;
                osg::StateSet::AttributeList::iterator alitr = al.find(osg::StateAttribute::TEXTURE);
                if (alitr!=al.end())
                {
                    // found texture, so place it in the texture list.
                    osg::Texture* texture = static_cast<osg::Texture*>(alitr->second.first.get());
                    texture->dirtyTextureObject();
                }
            }
        }
    }
    
    inline void apply(osg::Drawable* drawable)
    {
        apply(drawable->getStateSet());

        if (drawable->getUseDisplayList() || drawable->getUseVertexBufferObjects());
        {
            drawable->dirtyDisplayList();
        }
    }
        
};

/* Merge Updates
    Merge in the new tiles and unhook the ones we'll be deleting.
    Actually, we'll hand these back to the paging thread for deletion.
    --- Main Thread ---
 */
bool OSGPageManager::MergeUpdateThread(osg::Group *rootNode)
{
    std::vector<osg::Group *> mergeList;
    std::vector<osg::Group *> mergeParentList;
    std::vector<osg::Group *> unhookList;

    // Make local copies of the merge and unhook lists
    {
        osgLockMutex(changeListMutex);
        mergeList = toMerge;
        mergeParentList = toMergeParent;
        unhookList = toUnhook;

        toMerge.clear();
        toMergeParent.clear();
        toUnhook.clear();
        osgUnLockMutex(changeListMutex);
    }

    // Do the unhooking first
    for (unsigned int ui=0;ui<unhookList.size();ui++) {
        osg::Group *unhookMe = unhookList[ui];

        // Look for its parent(s)
        // Only expecting one, but it doesn't matter
        const osg::Node::ParentList &parents = unhookMe->getParents();
        for (unsigned int pi=0;pi<parents.size();pi++) {
            osg::Group *parent = parents[pi];
            parent->removeChild(unhookMe);
        }
    }
    
#ifdef USE_THREADLOOP_DELETE
    // Put the unhooked things on the list to delete
    {
        ReleaseTexturesAndDrawablesVisitor rtadv;
        osgLockMutex(changeListMutex);
        for (unsigned int di=0;di<unhookList.size();di++)
        {
            toDelete.push_back(unhookList[di]);
            unhookList[di]->accept(rtadv);
        }
        osgUnLockMutex(changeListMutex);
    }
#endif

    // Do the merging last
    {
        for (unsigned int mi=0;mi<mergeList.size();mi++) {
            osg::Group *mergeMe = mergeList[mi];
            osg::Group *parent = mergeParentList[mi];
            if (parent)
                parent->addChild(mergeMe);
            else
                rootNode->addChild(mergeMe);
        }
    }

    return true;
}
