#include <stdlib.h>
#include <stdio.h>

#include "TrPageArchive.h"
#include "trPagePageManager.h"
#include "trpage_print.h"

#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileUtils>

#include <iostream>

using namespace txp;
using namespace osg;

#if defined(WIN32) 
#define sleep(x) Sleep((x)) 
#else
#define sleep(x) usleep((x)*1000) 
#endif

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
struct osgGuard
{
    ThreadMutex& _mtx;
    osgGuard(ThreadMutex &mtx):_mtx(mtx){
        _mtx.lock();
    }
    ~osgGuard(){
        _mtx.unlock();
    }
};

void PagingThread::run()
{
    // need to set the texture object manager to be able to reuse textures
    // by keeping deleted texture objects around for 10 seconds after being deleted.
    osg::Texture::getTextureObjectManager()->setExpiryDelay(10.0f);

    pager->ThreadLoop(this);
}

/* Start Thread
    This initialized
    --- Main Thread ---
 */
bool OSGPageManager::StartThread(ThreadMode mode,ThreadID &newThread)
{
    positionValid = false;

    //locationChangeEvent is self-initialized.
    threadMode = mode;
    pagingThread.pager = this;
    pagingThread.startThread();
    newThread = pagingThread.getThreadId();
    return threadMode != ThreadNone;
}


/* End Thread
    Note: Do this
 */
bool OSGPageManager::EndThread()
{
//    locationChangeEvent.release();
    pagingThread.cancel();
    return true;
}

void OSGPageManager::LoadOneTile(trpgManagedTile* tile)
{
    osg::Group* tileGroup;
    osg::Group* parentNode ;    
    int x,y,lod;
    tile->GetTileLoc(x,y,lod);

    tileGroup = archive->LoadTile(NULL,pageManage,tile,&parentNode);
    if (tileGroup) 
    {
#ifdef USE_THREADLOOP_DELETE
        // Make an extra reference to it because we want it back for deletion
    // RO, commenting out as we don't want to do delete here, we want it to happen in the merge thread.
        tileGroup->ref();
#endif
        // we only put tiles with NULL parent to merge list 
        // others are referenced when added as a child 
        osgGuard g(changeListMutex);
        if(parentNode)
            parentNode->addChild(tileGroup) ;
        else
            toMerge.push_back( tileGroup) ;
        //toMergeParent.push_back(parentNode );
    } 
    else 
    {
        osg::notify(WARN) << "Failed to load tile (" << x << y << lod << ")" << std::endl ;
    }
                
}

/* Thread Loop
    This method is the main loop for the pager when it's
    working in its own thread.
    --- Paging Thread ---
 */
bool OSGPageManager::ThreadLoop(PagingThread* t)
{
    // Note: Only works for free running thread
    if (threadMode != ThreadFree)
        throw 1;


    //std::cout<<"OSGPageManager::ThreadLoop()"<<std::endl;
    
    std::vector<osg::Group *> unhook;
    std::vector <osg::Group *> nextDelete;

    //bool pagingActive = false;
    do
    {

        /*  Here's how we do it:
            Wait for position change
            Update manager w/ position
            Form delete list
            Load tile (if needed)
            Add tile to merge list
        */
        // Position has already changed or we'll wait for it to do so
        //        locationChangeEvent.wait();
        double myLocX,myLocY;
        {
            osgGuard g(locationMutex);
            myLocX = locX;
            myLocY = locY;
            positionValid = false;
        }
        
        // Pass the new location on to page manager
        int x,y,lod;

        trpg2dPoint loc;
        loc.x = myLocX;
        loc.y = myLocY;
        
        //std::cout<<"location "<<myLocX<<"  "<<myLocY<<std::endl;

        if (pageManage->SetLocation(loc) ) {
            // If there were changes, process them
            // Form the delete list first
            trpgManagedTile *tile=NULL;

            unhook.clear();

            while ((tile = pageManage->GetNextUnload())) {
                tile->GetTileLoc(x,y,lod);
                unhook.push_back((Group *)(tile->GetLocalData()));
                pageManage->AckUnload();
            }

            nextDelete.clear();
            {
                osgGuard g(changeListMutex);
                // Add to the unhook list
                for(unsigned int kk = 0; kk < unhook.size();kk++)
                {
                    toUnhook.push_back(unhook[kk]);
                }
                // Also get the list of deletions while we're here
#ifdef USE_THREADLOOP_DELETE
                // use the stl Luke :-) swap is constant time operation that do a = b; b.clear() 
                // if a is empty which is our case
                nextDelete.swap(toDelete);
                //                toDelete.clear();
#endif
            }
            
#ifdef USE_THREADLOOP_DELETE
            // Unreference whatever we're supposed to delete
            for (unsigned int gi=0;gi<nextDelete.size();gi++)
            {
                if(    nextDelete[gi] )
                    nextDelete[gi]->unref();
            }
#endif
        

            // Now do a single load
            while((tile = pageManage->GetNextLoad()))
            {
                tile->GetTileLoc(x,y,lod);

                //osg::notify(WARN) << "Tile to load :" << x << ' ' << y << ' ' << lod << std::endl;
                //osg::notify(WARN) << "Position     :" << loc.x << ' ' << loc.y << std::endl;
                LoadOneTile(tile);                
                // Now add this tile to the merge list
                pageManage->AckLoad();
            }


        }
        else
        {
            //sleep(10);
            OpenThreads::Thread::Yield();
        }
            
    } while (!t->testCancel());

    return true;
}

/* Update Position Thread
    This method updates the location for the paging thread to use.
    --- Main Thread ---
 */
void OSGPageManager::UpdatePositionThread(double inLocX,double inLocY)
{
    // Update the position
    if(!positionValid)
    {
        // Get the location mutex
        osgGuard g(locationMutex);
        positionValid = true;
        locX = inLocX-originX;
        locY = inLocY-originY;
    }

    // Notify the paging thread there's been a position update
//    locationChangeEvent.release();
}

/* Merge Updates
    Merge in the new tiles and unhook the ones we'll be deleting.
    Actually, we'll hand these back to the paging thread for deletion.
    --- Main Thread ---
 */
bool OSGPageManager::MergeUpdateThread(osg::Group *rootNode)
{
    std::vector<osg::Group *> mergeList;
    std::vector<osg::Group *> unhookList;

    // Make local copies of the merge and unhook lists
    {
        osgGuard g(changeListMutex);
        // use the stl Luke :-) swap is constant time operation that do a = b; b.clear() 
        // if a is empty which is our case
        mergeList.swap(toMerge);
        unhookList.swap(toUnhook);
//        toMerge.clear();
//        toMergeParent.clear();
//        toUnhook.clear();
    }

    // Do the unhooking first
    for (unsigned int ui=0;ui<unhookList.size();ui++) {
        osg::Group *unhookMe = unhookList[ui];

        // better safe than sorry

        if(!unhookMe ) continue;
        // Look for its parent(s)
        // Only expecting one, but it doesn't matter
        const osg::Node::ParentList &parents = unhookMe->getParents();
        for (unsigned int pi=0;pi<parents.size();pi++) {
            osg::Group *parent = parents[pi];
            if(parent != rootNode)
                parent->removeChild(unhookMe);
        }
    }
    
#ifdef USE_THREADLOOP_DELETE
    // Put the unhooked things on the list to delete
    {
        osgGuard g(changeListMutex);
        for (unsigned int i = 0; i < unhookList.size();i++)
            toDelete.push_back(unhookList[i]);
    }
#endif

    // Do the merging last
    {
        for (unsigned int mi=0;mi<mergeList.size();mi++) {
            osg::Group *mergeMe = mergeList[mi];
//  NO need for that - we only put tiles with NULL parent to merge list         
//            osg::Group *parent = mergeParentList[mi];
//            if (parent)
//                parent->addChild(mergeMe);
//            else
            rootNode->addChild(mergeMe);
        }
    }


    return true;
}
