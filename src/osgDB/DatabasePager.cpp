#include <osgDB/DatabasePager>
#include <osgDB/ReadFile>
#include <osg/Geode>
#include <osg/Timer>
#include <osg/Texture>
#include <osg/Notify>

#include <OpenThreads/ScopedLock>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace osgDB;

DatabasePager::DatabasePager()
{
    //osg::notify(osg::INFO)<<"Constructing DatabasePager()"<<std::endl;
    
    _useFrameBlock = false;
    _frameNumber = 0;
    _frameBlock = new Block;
    _fileRequestListEmptyBlock = new Block;

    _threadPriorityDuringFrame = PRIORITY_MIN;
    _threadPriorityOutwithFrame = PRIORITY_NOMINAL;

    _deleteRemovedSubgraphsInDatabaseThread = true;
    
    _expiryDelay = 1.0;

    // make sure a SharedStateManager exists.
    //osgDB::Registry::instance()->getOrCreateSharedStateManager();
    
    //if (osgDB::Registry::instance()->getSharedStateManager())
        //osgDB::Registry::instance()->setUseObjectCacheHint(true);
}

DatabasePager::~DatabasePager()
{

    // release the frameBlock and _fileRequestListEmptyBlock incase its holding up thread cancelation.
    _frameBlock->release();
    _fileRequestListEmptyBlock->release();

    //std::cout<<"DatabasePager::~DatabasePager()"<<std::endl;
    if( isRunning() )
    {

        // cancel the thread..
        cancel();
        //join();

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            osg::notify(osg::DEBUG_INFO)<<"Waiting for DatabasePager to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
        
    }
    //std::cout<<"DatabasePager::~DatabasePager() stopped running"<<std::endl;
}

void DatabasePager::requestNodeFile(const std::string& fileName,osg::Group* group, float priority, const osg::FrameStamp* framestamp)
{
   
    double timestamp = framestamp?framestamp->getReferenceTime():0.0;
    int frameNumber = framestamp?framestamp->getFrameNumber():_frameNumber;
   
    // search to see if filename already exist in the file loaded list.
    bool foundEntry = false;

    _dataToCompileListMutex.lock();
    
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

    _dataToCompileListMutex.unlock();

    if (!foundEntry)
    {
        _dataToMergeListMutex.lock();

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

        _dataToMergeListMutex.unlock();
    }
    
    if (!foundEntry)
    {
    
        _fileRequestListMutex.lock();

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

                if (_fileRequestList.empty()) _fileRequestListEmptyBlock->release();

                _fileRequestList.push_back(databaseRequest);
            }

        _fileRequestListMutex.unlock();
    }
    
    if (!isRunning())
    {
        static OpenThreads::Mutex s_mutex;
                
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex);
        
        static bool s_startThreadCalled = false;
        
        if (!s_startThreadCalled)
        {
            s_startThreadCalled = true;
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

    _frameBlock->reset();

    if (_threadPriorityDuringFrame!=getSchedulePriority())
        setSchedulePriority(_threadPriorityDuringFrame);

}

void DatabasePager::signalEndFrame()
{
    //osg::notify(osg::INFO) << "signalEndFrame <<<<<<<<<<<<<<<<<<<< "<<std::endl;
    _frameBlock->release();

    if (_threadPriorityOutwithFrame!=getSchedulePriority())
        setSchedulePriority(_threadPriorityOutwithFrame);

}

class FindCompileableGLObjectsVisitor : public osg::NodeVisitor
{
public:
    FindCompileableGLObjectsVisitor(DatabasePager::DataToCompile& dataToCompile):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _dataToCompile(dataToCompile)
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
                if (al.count(osg::StateAttribute::TEXTURE)==1) foundTextureState = true;
            }

            // if texture object attributes exist add the state to the list for later compilation.
            if (foundTextureState)
            {
                //osg::notify(osg::DEBUG_INFO)<<"Found compilable texture state"<<std::endl;
                _dataToCompile.first.push_back(stateset);
            }
        }
    }
    
    inline void apply(osg::Drawable* drawable)
    {
        apply(drawable->getStateSet());

        if (drawable->getUseDisplayList() || drawable->getUseVertexBufferObjects())
        {
            //osg::notify(osg::INFO)<<"Found compilable drawable"<<std::endl;
            _dataToCompile.second.push_back(drawable);
        }
    }
    
    DatabasePager::DataToCompile& _dataToCompile;
};


struct SortFileRequestFunctor
{
    bool operator() (const osg::ref_ptr<DatabasePager::DatabaseRequest>& lhs,const osg::ref_ptr<DatabasePager::DatabaseRequest>& rhs) const
    {
        if (lhs->_timestampLastRequest>rhs->_timestampLastRequest) return true;
        else if (lhs->_timestampLastRequest<rhs->_timestampLastRequest) return false;
        else return (lhs->_priorityLastRequest>rhs->_priorityLastRequest);
    }
};


void DatabasePager::run()
{
    osg::notify(osg::NOTICE)<<"DatabasePager::run()"<<std::endl;

    // need to set the texture object manager to be able to reuse textures
    // by keeping deleted texture objects around for 10 seconds after being deleted.
    osg::Texture::getTextureObjectManager()->setExpiryDelay(10.0f);
    
    bool firstTime = true;
    
    do
    {
    
        _fileRequestListEmptyBlock->block();

        if (_useFrameBlock)
        {
            _frameBlock->block();
        }      
   
        //
        // delete any children if required.
        //
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            _childrenToDeleteListMutex.lock();
                if (!_childrenToDeleteList.empty())
                {
                    osg::notify(osg::INFO)<<"In DatabasePager thread deleting "<<_childrenToDeleteList.size()<<" subgraphs"<<std::endl;
                    _childrenToDeleteList.clear();
                }
            _childrenToDeleteListMutex.unlock();
        }

        //
        // load any subgraphs that are required.
        //
        osg::ref_ptr<DatabaseRequest> databaseRequest;
    
        // get the front of the file request list.
        _fileRequestListMutex.lock();
            if (!_fileRequestList.empty())
            {
                std::sort(_fileRequestList.begin(),_fileRequestList.end(),SortFileRequestFunctor());
                databaseRequest = _fileRequestList.front();
            }
        _fileRequestListMutex.unlock();
        
        if (databaseRequest.valid())
        {
            // check if databaseRequest is still relevant
            if (_frameNumber-databaseRequest->_frameNumberLastRequest<=1)
            {
                       
                // load the data, note safe to write to the databaseRequest since once 
                // it is created this thread is the only one to write to the _loadedModel pointer.
                osg::notify(osg::INFO)<<"In DatabasePager thread readNodeFile("<<databaseRequest->_fileName<<")"<<std::endl;
                osg::Timer_t before = osg::Timer::instance()->tick();
                databaseRequest->_loadedModel = osgDB::readNodeFile(databaseRequest->_fileName);
                osg::notify(osg::INFO)<<"     node read in "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms"<<std::endl;
                
                bool loadedObjectsNeedToBeCompiled = false;

                if (databaseRequest->_loadedModel.valid() && !_activeGraphicsContexts.empty())
                {
                    // force a compute of the loaded model's bounding volume, so that when the subgraph
                    // merged with the main scene graph and large computeBound() isn't incurred.
                    databaseRequest->_loadedModel->getBound();


                    ActiveGraphicsContexts::iterator itr = _activeGraphicsContexts.begin();

                    DataToCompile& dtc = databaseRequest->_dataToCompileMap[*itr];
                    ++itr;                

                    // find all the compileable rendering objects
                    FindCompileableGLObjectsVisitor frov(dtc);
                    databaseRequest->_loadedModel->accept(frov);

                    if (!dtc.first.empty() || !dtc.second.empty())
                    {
                        loadedObjectsNeedToBeCompiled = true;                

                        // copy the objects to compile list to the other graphics context list.
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
                _fileRequestListMutex.lock();

                    if (databaseRequest->_loadedModel.valid())
                    {
                        if (loadedObjectsNeedToBeCompiled)
                        {
                            _dataToCompileListMutex.lock();
                                _dataToCompileList.push_back(databaseRequest);
                            _dataToCompileListMutex.unlock();
                        }
                        else
                        {
                            _dataToMergeListMutex.lock();
                                _dataToMergeList.push_back(databaseRequest);
                            _dataToMergeListMutex.unlock();
                        }
                    }        

                    _fileRequestList.erase(_fileRequestList.begin());

                    if (_fileRequestList.empty()) _fileRequestListEmptyBlock->reset();

                _fileRequestListMutex.unlock();

            }
            else
            {
                //std::cout<<"frame number delta for "<<databaseRequest->_fileName<<" "<<_frameNumber-databaseRequest->_frameNumberLastRequest<<std::endl;
                // remove the databaseRequest from the front of the fileRequest to the end of
                // dataLoad list as its is no longer relevant
                _fileRequestListMutex.lock();

                    _fileRequestList.erase(_fileRequestList.begin());

                    if (_fileRequestList.empty()) _fileRequestListEmptyBlock->reset();

                _fileRequestListMutex.unlock();

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

    } while (!testCancel());
    
    cancel();
    join();   
}

void DatabasePager::addLoadedDataToSceneGraph(double timeStamp)
{
    DatabaseRequestList localFileLoadedList;

    // get the dat for the _dataToCompileList, leaving it empty via a std::vector<>.swap.
    _dataToMergeListMutex.lock();
        localFileLoadedList.swap(_dataToMergeList);
    _dataToMergeListMutex.unlock();
    
    // add the loaded data into the scene graph.
    for(DatabaseRequestList::iterator itr=localFileLoadedList.begin();
        itr!=localFileLoadedList.end();
        ++itr)
    {
        DatabaseRequest* databaseRequest = itr->get();

        
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
    
}

void DatabasePager::ReleaseTexturesAndDrawablesVisitor::apply(osg::Node& node)
{
    apply(node.getStateSet());

    traverse(node);
}
    
void DatabasePager::ReleaseTexturesAndDrawablesVisitor::apply(osg::Geode& geode)
{
    apply(geode.getStateSet());

    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        apply(geode.getDrawable(i));
    }

    traverse(geode);
}

void DatabasePager::ReleaseTexturesAndDrawablesVisitor::apply(osg::StateSet* stateset)
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

void DatabasePager::ReleaseTexturesAndDrawablesVisitor::apply(osg::Drawable* drawable)
{
    apply(drawable->getStateSet());

    if (drawable->getUseDisplayList() || drawable->getUseVertexBufferObjects())
    {
        drawable->dirtyDisplayList();
    }
}

void DatabasePager::removeExpiredSubgraphs(double currentFrameTime)
{
    double expiryTime = currentFrameTime - _expiryDelay;

    osg::NodeList childrenRemoved;

    //osg::notify(osg::INFO)<<"DatabasePager::removeExpiredSubgraphs("<<expiryTime<<") "<<std::endl;
    for(PagedLODList::iterator itr=_pagedLODList.begin();
        itr!=_pagedLODList.end();
        ++itr)
    {
        osg::PagedLOD* plod = itr->get();
        plod->removeExpiredChildren(expiryTime,childrenRemoved);
    }
    
    for(unsigned int i=_pagedLODList.size();
        i>0;
        )
    {
        --i;
        
        osg::PagedLOD* plod = _pagedLODList[i].get();
        if (plod->referenceCount()==1)
        {
            //osg::notify(osg::INFO)<<"    PagedLOD "<<plod<<" orphaned"<<std::endl;
            _pagedLODList.erase(_pagedLODList.begin()+i);
        }
        else
        {
            //osg::notify(osg::INFO)<<"    PagedLOD "<<plod<<" refcount "<<plod->referenceCount()<<std::endl;
        }
    }
     
    // for all the subgraphs to remove find all the textures and drawables and
    // strip them from the display lists.   
    {
        ReleaseTexturesAndDrawablesVisitor rtadv;
        for(osg::NodeList::iterator nitr=childrenRemoved.begin();
            nitr!=childrenRemoved.end();
            ++nitr)
        {
            (*nitr)->accept(rtadv);
        }
    }


    if (osgDB::Registry::instance()->getSharedStateManager()) 
        osgDB::Registry::instance()->getSharedStateManager()->prune();

    
    if (_deleteRemovedSubgraphsInDatabaseThread)
    {
        // transfer the removed children over to the to delete list so the database thread can delete them.
        _childrenToDeleteListMutex.lock();
            _childrenToDeleteList.insert(_childrenToDeleteList.begin(),childrenRemoved.begin(),childrenRemoved.end());
        _childrenToDeleteListMutex.unlock();
    }
    // otherwise the childrenRemoved list will automatically unref() and deleting the nodes.    

    // update the Registry object cache.
    osgDB::Registry::instance()->updateTimeStampOfObjectsInCacheWithExtenalReferences(currentFrameTime);
    osgDB::Registry::instance()->removeExpiredObjectsInCache(expiryTime);
}


class FindPagedLODsVisitor : public osg::NodeVisitor
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
    FindPagedLODsVisitor fplv(_pagedLODList);
    if (subgraph) subgraph->accept(fplv);
}

void DatabasePager::setCompileGLObjectsForContexID(unsigned int contextID, bool on)
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

bool DatabasePager::getCompileGLObjectsForContexID(unsigned int contextID)
{
    return _activeGraphicsContexts.count(contextID)!=0;
}


void DatabasePager::compileGLObjects(osg::State& state, double& availableTime)
{

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    osg::ref_ptr<DatabaseRequest> databaseRequest;
    
    // get the first compileable entry.
    _dataToCompileListMutex.lock();
        if (!_dataToCompileList.empty())
        {
            std::sort(_dataToCompileList.begin(),_dataToCompileList.end(),SortFileRequestFunctor());
            databaseRequest = _dataToCompileList.front();
        }
    _dataToCompileListMutex.unlock();

    // while there are valid databaseRequest's in the to compile list and there is
    // sufficient time left compile each databaseRequest's stateset and drawables.
    while (databaseRequest.valid() && elapsedTime<availableTime)
    {
        DataToCompileMap& dcm = databaseRequest->_dataToCompileMap;
        DataToCompile& dtc = dcm[state.getContextID()];
        if (!dtc.first.empty() && elapsedTime<availableTime)
        {
            // we have StateSet's to compile
            StateSetList& sslist = dtc.first;
            //osg::notify(osg::INFO)<<"Compiling statesets"<<std::endl;
            StateSetList::iterator itr=sslist.begin();
            for(;
                itr!=sslist.end() && elapsedTime<availableTime;
                ++itr)
            {
                //osg::notify(osg::INFO)<<"    Compiling stateset "<<(*itr).get()<<std::endl;
                (*itr)->compile(state);
                elapsedTime = timer.delta_s(start_tick,timer.tick());
            }
            // remove the compiled stateset from the list.
            sslist.erase(sslist.begin(),itr);
        }
        if (!dtc.second.empty() && elapsedTime<availableTime)
        {
            // we have Drawable's to compile
            //osg::notify(osg::INFO)<<"Compiling drawables"<<std::endl;
            DrawableList& dwlist = dtc.second;
            DrawableList::iterator itr=dwlist.begin();
            for(;
                itr!=dwlist.end() && elapsedTime<availableTime;
                ++itr)
            {
                //osg::notify(osg::INFO)<<"    Compiling drawable "<<(*itr).get()<<std::endl;
                (*itr)->compile(state);
                elapsedTime = timer.delta_s(start_tick,timer.tick());
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
            osg::notify(osg::INFO)<<"All compiled"<<std::endl;

            // we've compile all of the current databaseRequest so we can now pop it off the
            // to compile list and place it on the merge list.
            _dataToCompileListMutex.lock();
            
                _dataToMergeListMutex.lock();
                    _dataToMergeList.push_back(databaseRequest);
                _dataToMergeListMutex.unlock();

                _dataToCompileList.erase(_dataToCompileList.begin());
                
                if (!_dataToCompileList.empty())
                {
                    std::sort(_dataToCompileList.begin(),_dataToCompileList.end(),SortFileRequestFunctor());
                    databaseRequest = _dataToCompileList.front();
                }
                else databaseRequest = 0;

            _dataToCompileListMutex.unlock();
        }
        else 
        {
            //osg::notify(osg::INFO)<<"Not all compiled"<<std::endl;
            databaseRequest = 0;
        }
        
        elapsedTime = timer.delta_s(start_tick,timer.tick());
    }
    
    availableTime -= elapsedTime;
}

