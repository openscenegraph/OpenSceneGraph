#include <osgProducer/DatabasePager>
#include <osgDB/ReadFile>
#include <osg/Geode>
#include <osg/Timer>
#include <osg/Texture>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace osgProducer;

DatabasePager::DatabasePager()
{
    //std::cout<<"Constructing DatabasePager()"<<std::endl;
    
    _deleteRemovedSubgraphsInDatabaseThread = true;
    
    _expiryDelay = 1.0;
}

void DatabasePager::requestNodeFile(const std::string& fileName,osg::Group* group)
{
   
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
                    ++((*ritr)->_numOfRequests);
                }
            }        

            if (!foundEntry)
            {
                osg::ref_ptr<DatabaseRequest> databaseRequest = new DatabaseRequest;

                databaseRequest->_fileName = fileName;
                databaseRequest->_groupForAddingLoadedSubgraph = group;

                _fileRequestList.push_back(databaseRequest);
            }

        _fileRequestListMutex.unlock();
    }
    
    if (!threadIsRunning())
    {
        std::cout<<"DatabasePager::startThread()"<<std::endl;
        startThread();
    }
}

class FindCompileableRenderingObjectsVisitor : public osg::NodeVisitor
{
public:
    FindCompileableRenderingObjectsVisitor(DatabasePager::DataToCompile& dataToCompile):
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
                //std::cout<<"Found compilable texture state"<<std::endl;
                _dataToCompile.first.push_back(stateset);
            }
        }
    }
    
    inline void apply(osg::Drawable* drawable)
    {
        apply(drawable->getStateSet());

        if (drawable->getUseDisplayList() || drawable->getUseVertexBufferObjects())
        {
            //std::cout<<"Found compilable drawable"<<std::endl;
            _dataToCompile.second.push_back(drawable);
        }
    }
    
    DatabasePager::DataToCompile& _dataToCompile;
};


void DatabasePager::run()
{
    std::cout<<"DatabasePager::run()"<<std::endl;
    
    while(true)
    {
        //
        // delete any children if required.
        //
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            _childrenToDeleteListMutex.lock();
                if (!_childrenToDeleteList.empty())
                {
                    std::cout<<"In DatabasePager thread deleting "<<_childrenToDeleteList.size()<<" subgraphs"<<std::endl;
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
            if (!_fileRequestList.empty()) databaseRequest = _fileRequestList.front();
        _fileRequestListMutex.unlock();
        
        if (databaseRequest.valid())
        {
            // load the data, note safe to write to the databaseRequest since once 
            // it is created this thread is the only one to write to the _loadedModel pointer.
            databaseRequest->_loadedModel = osgDB::readNodeFile(databaseRequest->_fileName);

            bool loadedObjectsNeedToBeCompiled = false;

            if (databaseRequest->_loadedModel.valid() && !_activeGraphicsContexts.empty())
            {
                ActiveGraphicsContexts::iterator itr = _activeGraphicsContexts.begin();
            
                DataToCompile& dtc = databaseRequest->_dataToCompileMap[*itr];
                ++itr;                
                
                // find all the compileable rendering objects
                FindCompileableRenderingObjectsVisitor frov(dtc);
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

            _fileRequestListMutex.unlock();


            

        }
        
        // hack hack hack... sleep for 1ms so we give other threads a chance..
        #ifdef WIN32
            Sleep(1);
        #else
            usleep(1000);
        #endif
    }
    
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
        registerPagedLODs(databaseRequest->_loadedModel.get());
        
        osg::Group* group = databaseRequest->_groupForAddingLoadedSubgraph.get();
        osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(group);
        if (plod)
        {
            plod->setTimeStamp(plod->getNumChildren(),timeStamp);
        } 
        group->addChild(databaseRequest->_loadedModel.get());
        std::cout<<"merged subgraph"<<databaseRequest->_fileName<<" after "<<databaseRequest->_numOfRequests<<" requests."<<std::endl;

    }
    
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

        if (drawable->getUseDisplayList() || drawable->getUseVertexBufferObjects())
        {
            drawable->dirtyDisplayList();
        }
    }
        
};

void DatabasePager::removeExpiredSubgraphs(double currentFrameTime)
{
    double expiryTime = currentFrameTime - _expiryDelay;

    osg::NodeList childrenRemoved;

    //std::cout<<"DatabasePager::removeExpiredSubgraphs("<<expiryTime<<") "<<std::endl;
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
            //std::cout<<"    PagedLOD "<<plod<<" orphaned"<<std::endl;
            _pagedLODList.erase(_pagedLODList.begin()+i);
        }
        else
        {
            //std::cout<<"    PagedLOD "<<plod<<" refcount "<<plod->referenceCount()<<std::endl;
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


    if (_deleteRemovedSubgraphsInDatabaseThread)
    {
        // transfer the removed children over to the to delete list so the database thread can delete them.
        _childrenToDeleteListMutex.lock();
            _childrenToDeleteList.insert(_childrenToDeleteList.begin(),childrenRemoved.begin(),childrenRemoved.end());
        _childrenToDeleteListMutex.unlock();
    }
    // otherwise the childrenRemoved list will automatically unref() and deleting the nodes.    
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

void DatabasePager::setCompileRenderingObjectsForContexID(unsigned int contextID, bool on)
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

bool DatabasePager::getCompileRenderingObjectsForContexID(unsigned int contextID)
{
    return _activeGraphicsContexts.count(contextID)!=0;
}


void DatabasePager::compileRenderingObjects(osg::State& state, double& availableTime)
{

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    osg::ref_ptr<DatabaseRequest> databaseRequest;
    
    // get the first compileable entry.
    _dataToCompileListMutex.lock();
        if (!_dataToCompileList.empty()) databaseRequest = _dataToCompileList.front();
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
            //std::cout<<"Compiling statesets"<<std::endl;
            StateSetList::iterator itr=sslist.begin();
            for(;
                itr!=sslist.end() && elapsedTime<availableTime;
                ++itr)
            {
                //std::cout<<"    Compiling stateset "<<(*itr).get()<<std::endl;
                (*itr)->compile(state);
                elapsedTime = timer.delta_s(start_tick,timer.tick());
            }
            // remove the compiled stateset from the list.
            sslist.erase(sslist.begin(),itr);
        }
        if (!dtc.second.empty() && elapsedTime<availableTime)
        {
            // we have Drawable's to compile
            //std::cout<<"Compiling drawables"<<std::endl;
            DrawableList& dwlist = dtc.second;
            DrawableList::iterator itr=dwlist.begin();
            for(;
                itr!=dwlist.end() && elapsedTime<availableTime;
                ++itr)
            {
                //std::cout<<"    Compiling drawable "<<(*itr).get()<<std::endl;
                (*itr)->compile(state);
                elapsedTime = timer.delta_s(start_tick,timer.tick());
            }
            // remove the compiled drawables from the list.
            dwlist.erase(dwlist.begin(),itr);
        }
        
        //std::cout<<"Checking if compiled"<<std::endl;

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
            std::cout<<"All compiled"<<std::endl;

            // we've compile all of the current databaseRequest so we can now pop it off the
            // to compile list and place it on the merge list.
            _dataToCompileListMutex.lock();
            
                _dataToMergeListMutex.lock();
                    _dataToMergeList.push_back(databaseRequest);
                _dataToMergeListMutex.unlock();

                _dataToCompileList.erase(_dataToCompileList.begin());
                
                if (!_dataToCompileList.empty()) databaseRequest = _dataToCompileList.front();
                else databaseRequest = 0;

            _dataToCompileListMutex.unlock();
        }
        else 
        {
            //std::cout<<"Not all compiled"<<std::endl;
            databaseRequest = 0;
        }
        
        elapsedTime = timer.delta_s(start_tick,timer.tick());
    }
    
    availableTime -= elapsedTime;
}

