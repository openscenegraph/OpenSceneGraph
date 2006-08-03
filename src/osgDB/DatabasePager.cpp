#include <osgDB/DatabasePager>
#include <osgDB/ReadFile>

#include <osg/Geode>
#include <osg/Timer>
#include <osg/Texture>
#include <osg/Notify>
#include <osg/ApplicationUsage>

#include <OpenThreads/ScopedLock>

#include <algorithm>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace osgDB;
using namespace OpenThreads;

static osg::ApplicationUsageProxy DatabasePager_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DO_PRE_COMPILE <ON/OFF>","Switch on or off the pre compile of OpenGL object database pager.");
static osg::ApplicationUsageProxy DatabasePager_e1(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MINIMUM_COMPILE_TIME_PER_FRAME <float>","minimum compile time alloted to compiling OpenGL objects per frame in database pager.");
static osg::ApplicationUsageProxy DatabasePager_e2(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MAXIMUM_OBJECTS_TO_COMPILE_PER_FRAME <int>","maximum number of OpenGL objects to compile per frame in database pager.");
static osg::ApplicationUsageProxy DatabasePager_e3(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DATABASE_PAGER_DRAWABLE <mode>","Set the drawable policy for setting of loaded drawable to specified type.  mode can be one of DoNotModify, DisplayList, VBO or VertexArrays>.");
DatabasePager::DatabasePager()
{
    //osg::notify(osg::INFO)<<"Constructing DatabasePager()"<<std::endl;
    
    _startThreadCalled = false;

    _done = false;
    _acceptNewRequests = true;
    _databasePagerThreadPaused = false;
    
    _useFrameBlock = false;
    _numFramesActive = 0;
    _frameNumber = 0;
    _frameBlock = new osg::Block;
    _databasePagerThreadBlock = new osg::Block;

    _threadPriorityDuringFrame = THREAD_PRIORITY_MIN;
    _threadPriorityOutwithFrame = THREAD_PRIORITY_MIN;

#if __APPLE__
    // OSX really doesn't like compiling display lists, and performs poorly when they are used,
    // so apply this hack to make up for its short comings.
    _drawablePolicy = USE_VERTEX_ARRAYS;
#else
    _drawablePolicy = DO_NOT_MODIFY_DRAWABLE_SETTINGS;
#endif    
    
    const char* str = getenv("OSG_DATABASE_PAGER_GEOMETRY");
    if (!str) str = getenv("OSG_DATABASE_PAGER_DRAWABLE");
    if (str)
    {
        if (strcmp(str,"DoNotModify")==0)
        {
            _drawablePolicy = DO_NOT_MODIFY_DRAWABLE_SETTINGS;
        }
        else if (strcmp(str,"DisplayList")==0 || strcmp(str,"DL")==0)
        {
            _drawablePolicy = USE_DISPLAY_LISTS;
        }
        else if (strcmp(str,"VBO")==0)
        {
            _drawablePolicy = USE_VERTEX_BUFFER_OBJECTS;
        }
        else if (strcmp(str,"VertexArrays")==0 || strcmp(str,"VA")==0 )
        {
            _drawablePolicy = USE_VERTEX_ARRAYS;
        } 
    }

    _changeAutoUnRef = true;
    _valueAutoUnRef = true;
    _changeAnisotropy = false;
    _valueAnisotropy = 1.0f;


#if 1
    _deleteRemovedSubgraphsInDatabaseThread = true;
#else
    _deleteRemovedSubgraphsInDatabaseThread = false;
#endif
    
    _expiryDelay = 10;

    const char* ptr=0;
    _doPreCompile = true;
    if( (ptr = getenv("OSG_DO_PRE_COMPILE")) != 0)
    {
        _doPreCompile = strcmp(ptr,"yes")==0 || strcmp(ptr,"YES")==0 ||
                        strcmp(ptr,"on")==0 || strcmp(ptr,"ON")==0;
    }

    _targetFrameRate = 100.0;
    _minimumTimeAvailableForGLCompileAndDeletePerFrame = 0.001; // 1ms.
    _maximumNumOfObjectsToCompilePerFrame = 4;
    if( (ptr = getenv("OSG_MINIMUM_COMPILE_TIME_PER_FRAME")) != 0)
    {
        _minimumTimeAvailableForGLCompileAndDeletePerFrame = atof(ptr);
    }

    if( (ptr = getenv("OSG_MAXIMUM_OBJECTS_TO_COMPILE_PER_FRAME")) != 0)
    {
        _maximumNumOfObjectsToCompilePerFrame = atoi(ptr);
    }

    // make sure a SharedStateManager exists.
    //osgDB::Registry::instance()->getOrCreateSharedStateManager();
    
    //if (osgDB::Registry::instance()->getSharedStateManager())
        //osgDB::Registry::instance()->setUseObjectCacheHint(true);
}

DatabasePager::~DatabasePager()
{
    cancel();
}

int DatabasePager::cancel()
{
    int result = 0;
    if( isRunning() )
    {
    
        _done = true;

        // cancel the thread..
        result = Thread::cancel();
        //join();

        // release the frameBlock and _databasePagerThreadBlock incase its holding up thread cancelation.
        _frameBlock->release();
        _databasePagerThreadBlock->release();

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            // commenting out debug info as it was cashing crash on exit, presumable
            // due to osg::notify or std::cout destructing earlier than this destructor.
            // osg::notify(osg::DEBUG_INFO)<<"Waiting for DatabasePager to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
        
        _startThreadCalled = false;
    }
    //std::cout<<"DatabasePager::~DatabasePager() stopped running"<<std::endl;
    return result;
}

void DatabasePager::clear()
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestListMutex);
        _fileRequestList.clear();
    }

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileListMutex);
        _dataToCompileList.clear();
    }

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_childrenToDeleteListMutex);
        _childrenToDeleteList.clear();
    }
    
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeListMutex);
        _dataToMergeList.clear();
    }

    // no mutex??
    _activePagedLODList.clear();
    _inactivePagedLODList.clear();
    
    // ??
    // _activeGraphicsContexts
}

void DatabasePager::requestNodeFile(const std::string& fileName,osg::Group* group, float priority, const osg::FrameStamp* framestamp)
{
    if (!_acceptNewRequests) return;
   
    double timestamp = framestamp?framestamp->getReferenceTime():0.0;
    int frameNumber = framestamp?framestamp->getFrameNumber():_frameNumber;
   
    // search to see if filename already exist in the file loaded list.
    bool foundEntry = false;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileListMutex);
    
        for(DatabaseRequestList::iterator litr = _dataToCompileList.begin();
            litr != _dataToCompileList.end() && !foundEntry;
            ++litr)
        {
            if ((*litr)->_fileName==fileName)
            {
                foundEntry = true;
                (*litr)->_frameNumberLastRequest = frameNumber;
                (*litr)->_timestampLastRequest = timestamp;
                (*litr)->_priorityLastRequest = priority;
                ++((*litr)->_numOfRequests);
            }
        }        

    }

    if (!foundEntry)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeListMutex);

        for(DatabaseRequestList::iterator litr = _dataToMergeList.begin();
            litr != _dataToMergeList.end() && !foundEntry;
            ++litr)
        {
            if ((*litr)->_fileName==fileName)
            {
                foundEntry = true;
                (*litr)->_frameNumberLastRequest = frameNumber;
                (*litr)->_timestampLastRequest = timestamp;
                (*litr)->_priorityLastRequest = priority;
                ++((*litr)->_numOfRequests);
            }
        }        
    }
    
    if (!foundEntry)
    {
    
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestListMutex);

        // search to see if entry already  in file request list.
        bool foundEntry = false;
        for(DatabaseRequestList::iterator ritr = _fileRequestList.begin();
            ritr != _fileRequestList.end() && !foundEntry;
            ++ritr)
        {
            if ((*ritr)->_fileName==fileName)
            {
                foundEntry = true;
                (*ritr)->_timestampLastRequest = timestamp;
                (*ritr)->_priorityLastRequest = priority;
                (*ritr)->_frameNumberLastRequest = frameNumber;
                ++((*ritr)->_numOfRequests);
            }
        }        

        if (!foundEntry)
        {
            osg::notify(osg::INFO)<<"In DatabasePager::fileRquest("<<fileName<<")"<<std::endl;

            osg::ref_ptr<DatabaseRequest> databaseRequest = new DatabaseRequest;

            databaseRequest->_fileName = fileName;
            databaseRequest->_frameNumberFirstRequest = frameNumber;
            databaseRequest->_timestampFirstRequest = timestamp;
            databaseRequest->_priorityFirstRequest = priority;
            databaseRequest->_frameNumberLastRequest = frameNumber;
            databaseRequest->_timestampLastRequest = timestamp;
            databaseRequest->_priorityLastRequest = priority;
            databaseRequest->_groupForAddingLoadedSubgraph = group;

            _fileRequestList.push_back(databaseRequest);

            updateDatabasePagerThreadBlock();
        }
    }
    
    if (!isRunning())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_run_mutex);
        
        if (!_startThreadCalled)
        {
            _startThreadCalled = true;
            _done = false;
            osg::notify(osg::DEBUG_INFO)<<"DatabasePager::startThread()"<<std::endl;
            setSchedulePriority(_threadPriorityDuringFrame);
            startThread();
        }
                
    }
}

void DatabasePager::signalBeginFrame(const osg::FrameStamp* framestamp)
{

    if (framestamp)
    {
        //osg::notify(osg::INFO) << "signalBeginFrame "<<framestamp->getFrameNumber()<<">>>>>>>>>>>>>>>>"<<std::endl;
        _frameNumber = framestamp->getFrameNumber();
        
    } //else osg::notify(osg::INFO) << "signalBeginFrame >>>>>>>>>>>>>>>>"<<std::endl;

    updateFrameBlock(1);

    if (_numFramesActive>0 && _threadPriorityDuringFrame!=getSchedulePriority())
        setSchedulePriority(_threadPriorityDuringFrame);
}

void DatabasePager::signalEndFrame()
{
    //osg::notify(osg::INFO) << "signalEndFrame <<<<<<<<<<<<<<<<<<<< "<<std::endl;
    updateFrameBlock(-1);

    if (_numFramesActive<=0 && _threadPriorityOutwithFrame!=getSchedulePriority())
        setSchedulePriority(_threadPriorityOutwithFrame);

}

class DatabasePager::FindCompileableGLObjectsVisitor : public osg::NodeVisitor
{
public:
    FindCompileableGLObjectsVisitor(DatabasePager::DataToCompile& dataToCompile, 
                               bool changeAutoUnRef, bool valueAutoUnRef,
                               bool changeAnisotropy, float valueAnisotropy,
                               DatabasePager::DrawablePolicy drawablePolicy):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _dataToCompile(dataToCompile),
            _changeAutoUnRef(changeAutoUnRef), _valueAutoUnRef(valueAutoUnRef),
            _changeAnisotropy(changeAnisotropy), _valueAnisotropy(valueAnisotropy),
            _drawablePolicy(drawablePolicy)
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
            for(unsigned int i=0;i<stateset->getTextureAttributeList().size();++i)
            {
                osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset->getTextureAttribute(i,osg::StateAttribute::TEXTURE));
                if (texture)
                {
                    if (_changeAutoUnRef) texture->setUnRefImageDataAfterApply(_valueAutoUnRef);
                    if (_changeAnisotropy) texture->setMaxAnisotropy(_valueAnisotropy);
                    foundTextureState = true;
                }
            }

            // if texture object attributes exist add the state to the list for later compilation.
            if (foundTextureState)
            {
                //osg::notify(osg::DEBUG_INFO)<<"Found compilable texture state"<<std::endl;
                _dataToCompile.first.insert(stateset);
            }
        }
    }
    
    inline void apply(osg::Drawable* drawable)
    {
        apply(drawable->getStateSet());
        
        switch(_drawablePolicy)
        {
        case DatabasePager::DO_NOT_MODIFY_DRAWABLE_SETTINGS: 
             // do nothing, leave settings as they came in from loaded database.
             // osg::notify(osg::NOTICE)<<"DO_NOT_MODIFY_DRAWABLE_SETTINGS"<<std::endl;
             break;
        case DatabasePager::USE_DISPLAY_LISTS: 
             drawable->setUseDisplayList(true);
             drawable->setUseVertexBufferObjects(false);
             break;
        case DatabasePager::USE_VERTEX_BUFFER_OBJECTS:
             drawable->setUseDisplayList(true);
             drawable->setUseVertexBufferObjects(true);
             // osg::notify(osg::NOTICE)<<"USE_VERTEX_BUFFER_OBJECTS"<<std::endl;
             break;
        case DatabasePager::USE_VERTEX_ARRAYS:
             drawable->setUseDisplayList(false);
             drawable->setUseVertexBufferObjects(false);
             // osg::notify(osg::NOTICE)<<"USE_VERTEX_ARRAYS"<<std::endl;
             break;
        }
        
        if (drawable->getUseDisplayList() || drawable->getUseVertexBufferObjects())
        {
            // osg::notify(osg::NOTICE)<<"  Found compilable drawable"<<std::endl;
            _dataToCompile.second.push_back(drawable);
        }
    }
    
    DatabasePager::DataToCompile&   _dataToCompile;
    bool                            _changeAutoUnRef;
    bool                            _valueAutoUnRef;
    bool                            _changeAnisotropy;
    float                           _valueAnisotropy;
    DatabasePager::DrawablePolicy   _drawablePolicy;
};


struct DatabasePager::SortFileRequestFunctor
{
    bool operator() (const osg::ref_ptr<DatabasePager::DatabaseRequest>& lhs,const osg::ref_ptr<DatabasePager::DatabaseRequest>& rhs) const
    {
        if (lhs->_timestampLastRequest>rhs->_timestampLastRequest) return true;
        else if (lhs->_timestampLastRequest<rhs->_timestampLastRequest) return false;
        else return (lhs->_priorityLastRequest>rhs->_priorityLastRequest);
    }
};


void DatabasePager::setDatabasePagerThreadPause(bool pause)
{
    _databasePagerThreadPaused = pause;
    updateDatabasePagerThreadBlock();
}

void DatabasePager::run()
{
    osg::notify(osg::INFO)<<"DatabasePager::run()"<<std::endl;

    // need to set the texture object manager to be able to reuse textures
    osg::Texture::setMinimumNumberOfTextureObjectsToRetainInCache(100);
    
    // need to set the texture object manager to be able to reuse textures
    osg::Drawable::setMinimumNumberOfDisplayListsToRetainInCache(500);

    bool firstTime = true;
    
    do
    {
    
        _databasePagerThreadBlock->block();

        if (_useFrameBlock)
        {
            _frameBlock->block();
        }      
   
        //
        // delete any children if required.
        //
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            osg::ref_ptr<osg::Object> obj = 0;
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_childrenToDeleteListMutex);
                if (!_childrenToDeleteList.empty())
                {
                    //osg::notify(osg::NOTICE)<<"In DatabasePager thread deleting "<<_childrenToDeleteList.size()<<" objects"<<std::endl;
                    //osg::Timer_t before = osg::Timer::instance()->tick();
                    obj = _childrenToDeleteList.back();
                    _childrenToDeleteList.pop_back();
                    //osg::notify(osg::NOTICE)<<"Done DatabasePager thread deleted in "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms"<<" objects"<<std::endl;

                    updateDatabasePagerThreadBlock();

                }
            }
        }

        //
        // load any subgraphs that are required.
        //
        osg::ref_ptr<DatabaseRequest> databaseRequest;
    
        // get the front of the file request list.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestListMutex);
            if (!_fileRequestList.empty())
            {
                std::sort(_fileRequestList.begin(),_fileRequestList.end(),SortFileRequestFunctor());
                databaseRequest = _fileRequestList.front();
            }
        }
                
        if (databaseRequest.valid())
        {
            // check if databaseRequest is still relevant
            if (_frameNumber-databaseRequest->_frameNumberLastRequest<=1)
            {
                       
                // load the data, note safe to write to the databaseRequest since once 
                // it is created this thread is the only one to write to the _loadedModel pointer.
                // osg::notify(osg::NOTICE)<<"In DatabasePager thread readNodeFile("<<databaseRequest->_fileName<<")"<<std::endl;
                //osg::Timer_t before = osg::Timer::instance()->tick();
                
                
                bool serialize_readNodeFile = true;
                if (serialize_readNodeFile)
                {
                    // do *not* assume that we only have one DatabasePager, or that reaNodeFile is thread safe...
                    static OpenThreads::Mutex s_serialize_readNodeFile_mutex;
                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_serialize_readNodeFile_mutex);
                    databaseRequest->_loadedModel = osgDB::readNodeFile(databaseRequest->_fileName);
                }
                else
                {
                    // assume that we only have one DatabasePager, or that reaNodeFile is thread safe...
                    databaseRequest->_loadedModel = osgDB::readNodeFile(databaseRequest->_fileName);
                }
                    
                //osg::notify(osg::NOTICE)<<"     node read in "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms"<<std::endl;
                
                bool loadedObjectsNeedToBeCompiled = false;

                if (_doPreCompile && databaseRequest->_loadedModel.valid() && !_activeGraphicsContexts.empty())
                {
                    // force a compute of the loaded model's bounding volume, so that when the subgraph
                    // merged with the main scene graph and large computeBound() isn't incurred.
                    databaseRequest->_loadedModel->getBound();


                    ActiveGraphicsContexts::iterator itr = _activeGraphicsContexts.begin();

                    DataToCompile& dtc = databaseRequest->_dataToCompileMap[*itr];
                    ++itr;                

                    // find all the compileable rendering objects
                    FindCompileableGLObjectsVisitor frov(dtc, 
                                                         _changeAutoUnRef, _valueAutoUnRef,
                                                         _changeAnisotropy, _valueAnisotropy,
                                                         _drawablePolicy);

                    databaseRequest->_loadedModel->accept(frov);

                    if (!dtc.first.empty() || !dtc.second.empty())
                    {
                        loadedObjectsNeedToBeCompiled = true;                

                        // copy the objects from the compile list to the other graphics context list.
                        for(;
                            itr != _activeGraphicsContexts.end();
                            ++itr)
                        {
                            databaseRequest->_dataToCompileMap[*itr] = dtc;
                        }
                    }
                }            

                // move the databaseRequest from the front of the fileRequest to the end of
                // dataLoad list.
                
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestListMutex);

                DatabaseRequestList::iterator itr = std::find(_fileRequestList.begin(),_fileRequestList.end(),databaseRequest);
                if (itr != _fileRequestList.end()) 
                {
                    if (databaseRequest->_loadedModel.valid())
                    {
                        if (loadedObjectsNeedToBeCompiled)
                        {
                            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileListMutex);
                            _dataToCompileList.push_back(databaseRequest);
                        }
                        else
                        {
                            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeListMutex);
                            _dataToMergeList.push_back(databaseRequest);
                        }
                    }        
                    
                    _fileRequestList.erase(itr);
                }

                updateDatabasePagerThreadBlock();

            }
            else
            {
                //std::cout<<"frame number delta for "<<databaseRequest->_fileName<<" "<<_frameNumber-databaseRequest->_frameNumberLastRequest<<std::endl;
                // remove the databaseRequest from the front of the fileRequest to the end of
                // dataLoad list as its is no longer relevant
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestListMutex);

                if (!_fileRequestList.empty()) _fileRequestList.erase(_fileRequestList.begin());

                updateDatabasePagerThreadBlock();

            }
        }
        
        // go to sleep till our the next time our thread gets scheduled.

        if (firstTime)
        {
            // do a yield to get round a peculiar thread hang when testCancel() is called 
            // in certain cirumstances - of which there is no particular pattern.
            YieldCurrentThread();
            firstTime = false;
        }

    } while (!testCancel() && !_done);

}


bool DatabasePager::requiresUpdateSceneGraph() const
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeListMutex);
        if (!_dataToMergeList.empty()) return true;
    }
    
    return false;
    
}
 

void DatabasePager::addLoadedDataToSceneGraph(double timeStamp)
{

    // osg::Timer_t before = osg::Timer::instance()->tick();

    DatabaseRequestList localFileLoadedList;

    // get the dat for the _dataToCompileList, leaving it empty via a std::vector<>.swap.
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeListMutex);
        localFileLoadedList.swap(_dataToMergeList);
    }
        
    // add the loaded data into the scene graph.
    for(DatabaseRequestList::iterator itr=localFileLoadedList.begin();
        itr!=localFileLoadedList.end();
        ++itr)
    {
        DatabaseRequest* databaseRequest = itr->get();

        // osg::notify(osg::NOTICE)<<"Merging "<<_frameNumber-(*itr)->_frameNumberLastRequest<<std::endl;
        
        if (osgDB::Registry::instance()->getSharedStateManager()) 
            osgDB::Registry::instance()->getSharedStateManager()->share(databaseRequest->_loadedModel.get());

        
        registerPagedLODs(databaseRequest->_loadedModel.get());
        
        osg::Group* group = databaseRequest->_groupForAddingLoadedSubgraph.get();
        osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(group);
        if (plod)
        {
            plod->setTimeStamp(plod->getNumChildren(),timeStamp);
        } 
        group->addChild(databaseRequest->_loadedModel.get());
        osg::notify(osg::INFO)<<"merged subgraph"<<databaseRequest->_fileName<<" after "<<databaseRequest->_numOfRequests<<" requests."<<std::endl;

    }

    // osg::notify(osg::NOTICE)<<"Done DatabasePager::addLoadedDataToSceneGraph"<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms  objects"<<localFileLoadedList.size()<<std::endl;
    
}


void DatabasePager::removeExpiredSubgraphs(double currentFrameTime)
{
    //osg::notify(osg::NOTICE)<<"DatabasePager::removeExpiredSubgraphs()"<<std::endl;

    // osg::Timer_t before = osg::Timer::instance()->tick();

    double expiryTime = currentFrameTime - _expiryDelay;

    osg::NodeList childrenRemoved;
    

    for(PagedLODList::iterator active_itr = _activePagedLODList.begin();
        active_itr!=_activePagedLODList.end();
        )
    {
        const osg::PagedLOD* plod = active_itr->get();
        bool remove_plod = false;
        if (plod->referenceCount()<=1)
        {
            // prune PageLOD's that are no longer externally referenced
            childrenRemoved.push_back(const_cast<osg::PagedLOD*>(plod));
            //osg::notify(osg::NOTICE)<<"_activePagedLODList : pruning no longer externally referenced"<<std::endl;
            remove_plod = true;
        }
        else if (plod->getFrameNumberOfLastTraversal()<_frameNumber)
        {                
            // osg::notify(osg::NOTICE)<<"_activePagedLODList : moving PageLOD to inactive list"<<std::endl;
            _inactivePagedLODList.push_back(*active_itr);
            remove_plod = true;
        }
        
        if (remove_plod)
        {
            PagedLODList::iterator itr_to_erase = active_itr;
            ++active_itr;
            
            _activePagedLODList.erase(itr_to_erase);            
        }
        else
        {
            ++active_itr;
        }
    }
    
    unsigned int i = 0;
    // unsigned int numberOfPagedLODToTest = _inactivePagedLODList.size();
    unsigned int targetNumOfInActivePagedLODs = 100;
    unsigned int targetNumOfRemovedChildPagedLODs = 0;
    if (_inactivePagedLODList.size()>targetNumOfInActivePagedLODs) targetNumOfRemovedChildPagedLODs = _inactivePagedLODList.size()-targetNumOfInActivePagedLODs;
    
    if (targetNumOfRemovedChildPagedLODs>1) targetNumOfRemovedChildPagedLODs=1;
    

    // filter out singly referenced PagedLOD and move reactivated PagedLOD into the active list
    for(PagedLODList::iterator inactive_itr = _inactivePagedLODList.begin();
        inactive_itr!=_inactivePagedLODList.end();
        )
    {
        const osg::PagedLOD* plod = inactive_itr->get();
        bool remove_plod = false;
        if (plod->referenceCount()<=1)
        {
            // prune PageLOD's that are no longer externally referenced
            childrenRemoved.push_back(const_cast<osg::PagedLOD*>(plod));
            //osg::notify(osg::NOTICE)<<"_activePagedLODList : pruning no longer externally referenced"<<std::endl;
            remove_plod = true;
        }
        else if (plod->getFrameNumberOfLastTraversal()>=_frameNumber)
        {
            // osg::notify(osg::NOTICE)<<"_inactivePagedLODList : moving PageLOD to active list"<<std::endl;
            // found a reactivated pagedLOD's need to put it back in the active list.                
            _activePagedLODList.push_back(*inactive_itr);
            remove_plod = true;
        }
        
        if (remove_plod)
        {
            PagedLODList::iterator itr_to_erase = inactive_itr;
            ++inactive_itr;
            
            _inactivePagedLODList.erase(itr_to_erase);            
        }
        else
        {
//            if (i<numberOfPagedLODToTest)
            if (childrenRemoved.size()<targetNumOfRemovedChildPagedLODs)
            {
                // check for removing expired children.
                if (const_cast<osg::PagedLOD*>(plod)->removeExpiredChildren(expiryTime,childrenRemoved))
                {
                    //osg::notify(osg::NOTICE)<<"Some children removed from PLod"<<std::endl;
                }
                else
                {
                    // osg::notify(osg::NOTICE)<<"no children removed from PLod"<<std::endl;
                }
            }
        
            ++inactive_itr;
            ++i;
        }
    }

    //osg::notify(osg::NOTICE)<<"_activePagedLODList.size()="<<_activePagedLODList.size()<<"\t_inactivePagedLODList.size()="<<_inactivePagedLODList.size()<<std::endl;


    // osg::notify(osg::NOTICE)<<"   number stale "<<numberStale<<"  number active "<<_pagedLODList.size()-numberStale<<std::endl;

    //double t = osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick());
    //osg::notify(osg::NOTICE)<<"   time 1 "<<t<<" ms "<<_pagedLODList.size()<<" pagedLOD's"<<std::endl;
    //osg::notify(osg::NOTICE)<<"   average time = "<<t/(double)_pagedLODList.size()<<" ms/per PagedLOD"<<std::endl;

    if (!childrenRemoved.empty())
    { 
        // pass the objects across to the database pager delete list
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_childrenToDeleteListMutex);
            for (osg::NodeList::iterator critr = childrenRemoved.begin();
                 critr!=childrenRemoved.end();
                 ++critr)
            {
                _childrenToDeleteList.push_back(critr->get());
            }
            updateDatabasePagerThreadBlock();
        }

        childrenRemoved.clear();
    }

    // osg::notify(osg::NOTICE)<<"   time 2 "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms "<<std::endl;

    
    if (osgDB::Registry::instance()->getSharedStateManager()) 
        osgDB::Registry::instance()->getSharedStateManager()->prune();

    // update the Registry object cache.
    osgDB::Registry::instance()->updateTimeStampOfObjectsInCacheWithExtenalReferences(currentFrameTime);
    osgDB::Registry::instance()->removeExpiredObjectsInCache(expiryTime);


    // osg::notify(osg::NOTICE)<<"Done DatabasePager::removeExpiredSubgraphs() "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms "<<std::endl;

}


class DatabasePager::FindPagedLODsVisitor : public osg::NodeVisitor
{
public:
    FindPagedLODsVisitor(DatabasePager::PagedLODList& pagedLODList):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _pagedLODList(pagedLODList)
    {
    }
    
    virtual void apply(osg::PagedLOD& plod)
    {
        _pagedLODList.push_back(&plod);
    
        traverse(plod);
    }
    
    DatabasePager::PagedLODList& _pagedLODList;
};

void DatabasePager::registerPagedLODs(osg::Node* subgraph)
{
    FindPagedLODsVisitor fplv(_activePagedLODList);
    if (subgraph) subgraph->accept(fplv);
}

bool DatabasePager::requiresCompileGLObjects() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileListMutex);
    return !_dataToCompileList.empty();
}

void DatabasePager::setCompileGLObjectsForContextID(unsigned int contextID, bool on)
{
    if (on)
    {
        _activeGraphicsContexts.insert(contextID);
    }
    else
    {
        _activeGraphicsContexts.erase(contextID);
    }
}

bool DatabasePager::getCompileGLObjectsForContextID(unsigned int contextID)
{
    return _activeGraphicsContexts.count(contextID)!=0;
}


void DatabasePager::compileGLObjects(osg::State& state, double& availableTime)
{
//    osg::notify(osg::NOTICE)<<"DatabasePager::compileGLObjects "<<_frameNumber<<std::endl;

    if (availableTime>0.0)
    {

        const osg::Timer& timer = *osg::Timer::instance();
        osg::Timer_t start_tick = timer.tick();
        double elapsedTime = 0.0;
        double estimatedTextureDuration = 0.0001;
        double estimatedDrawableDuration = 0.0001;

        osg::ref_ptr<DatabaseRequest> databaseRequest;

        // get the first compileable entry.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileListMutex);
            if (!_dataToCompileList.empty())
            {
                std::sort(_dataToCompileList.begin(),_dataToCompileList.end(),SortFileRequestFunctor());

                // prune all the old entries.
                DatabaseRequestList::iterator litr;
                int i=0;
                for(litr = _dataToCompileList.begin();
                    (litr != _dataToCompileList.end()) && (_frameNumber-(*litr)->_frameNumberLastRequest)<=1;
                    ++litr,i++)
                {
                    //osg::notify(osg::NOTICE)<<"Compile "<<_frameNumber-(*litr)->_frameNumberLastRequest<<std::endl;
                }
                if (litr != _dataToCompileList.end())
                {
                    if (_deleteRemovedSubgraphsInDatabaseThread)
                    {
                        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_childrenToDeleteListMutex);

                        for(DatabaseRequestList::iterator ditr=litr;
                            ditr!=_dataToCompileList.end();
                            ++ditr)
                        {
                            _childrenToDeleteList.push_back((*ditr)->_loadedModel.get());
                        }
                    }
                    //osg::notify(osg::NOTICE)<<"Pruning "<<_dataToCompileList.size()-i<<std::endl;
                    _dataToCompileList.erase(litr,_dataToCompileList.end());
                }

                // advance to the next entry to compile if one is available.
                databaseRequest = _dataToCompileList.empty() ? 0 : _dataToCompileList.front();
            
            }
        };

        unsigned int numObjectsCompiled = 0;

        // while there are valid databaseRequest's in the to compile list and there is
        // sufficient time left compile each databaseRequest's stateset and drawables.
        while (databaseRequest.valid() && elapsedTime<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame) 
        {
            DataToCompileMap& dcm = databaseRequest->_dataToCompileMap;
            DataToCompile& dtc = dcm[state.getContextID()];
            if (!dtc.first.empty() && (elapsedTime+estimatedTextureDuration)<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame)
            {

                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0);

                // we have StateSet's to compile
                StateSetList& sslist = dtc.first;
                //osg::notify(osg::INFO)<<"Compiling statesets"<<std::endl;
                StateSetList::iterator itr=sslist.begin();
                for(;
                    itr!=sslist.end() && (elapsedTime+estimatedTextureDuration)<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame;
                    ++itr)
                {
                    //osg::notify(osg::INFO)<<"    Compiling stateset "<<(*itr).get()<<std::endl;

                    double startCompileTime = timer.delta_s(start_tick,timer.tick());

                    (*itr)->compileGLObjects(state);

                    GLint p;
                    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_RESIDENT, &p);

                    elapsedTime = timer.delta_s(start_tick,timer.tick());


                    // estimate the duration of the compile based on current compile duration.
                    estimatedTextureDuration = (elapsedTime-startCompileTime);

                    ++numObjectsCompiled;
                }
                // remove the compiled stateset from the list.
                sslist.erase(sslist.begin(),itr);
            }

            if (!dtc.second.empty() && (elapsedTime+estimatedDrawableDuration)<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame)
            {
                // we have Drawable's to compile
                //osg::notify(osg::INFO)<<"Compiling drawables"<<std::endl;
                DrawableList& dwlist = dtc.second;
                DrawableList::iterator itr=dwlist.begin();
                for(;
                    itr!=dwlist.end() && (elapsedTime+estimatedDrawableDuration)<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame;
                    ++itr)
                {
                    //osg::notify(osg::INFO)<<"    Compiling drawable "<<(*itr).get()<<std::endl;
                    double startCompileTime = timer.delta_s(start_tick,timer.tick());
                    (*itr)->compileGLObjects(state);
                    elapsedTime = timer.delta_s(start_tick,timer.tick());

                    // estimate the duration of the compile based on current compile duration.
                    estimatedDrawableDuration = (elapsedTime-startCompileTime);

                    ++numObjectsCompiled;

                }
                // remove the compiled drawables from the list.
                dwlist.erase(dwlist.begin(),itr);
            }

            //osg::notify(osg::INFO)<<"Checking if compiled"<<std::endl;

            // now check the to compile entries for all active graphics contexts
            // to make sure that all have been compiled.
            bool allCompiled = true;
            for(DataToCompileMap::iterator itr=dcm.begin();
                itr!=dcm.end() && allCompiled;
                ++itr)
            {
                if (!(itr->second.first.empty())) allCompiled=false;
                if (!(itr->second.second.empty())) allCompiled=false;
            }


            if (allCompiled)
            {
                // we've compile all of the current databaseRequest so we can now pop it off the
                // to compile list and place it on the merge list.
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileListMutex);

                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeListMutex);
                    _dataToMergeList.push_back(databaseRequest);
                }

                if (!_dataToCompileList.empty()) _dataToCompileList.erase(_dataToCompileList.begin());

                if (!_dataToCompileList.empty())
                {
                    std::sort(_dataToCompileList.begin(),_dataToCompileList.end(),SortFileRequestFunctor());
                    databaseRequest = _dataToCompileList.front();
                }
                else databaseRequest = 0;

            }
            else 
            {
                //osg::notify(osg::INFO)<<"Not all compiled"<<std::endl;
                databaseRequest = 0;
            }

            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }

        availableTime -= elapsedTime;

        //osg::notify(osg::NOTICE)<<"elapsedTime="<<elapsedTime<<"\ttime remaining ="<<availableTime<<"\tnumObjectsCompiled = "<<numObjectsCompiled<<std::endl;
        //osg::notify(osg::NOTICE)<<"estimatedTextureDuration="<<estimatedTextureDuration;
        //osg::notify(osg::NOTICE)<<"\testimatedDrawableDuration="<<estimatedDrawableDuration<<std::endl;
    }
    else
    {
        availableTime = 0.0f;
    }
}

