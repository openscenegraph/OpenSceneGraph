#include <osgProducer/DatabasePager>
#include <osgDB/ReadFile>
#include <osg/Geode>

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
    
    _maximumTimeForCompiling = 0.005; // 5ms.
}

void DatabasePager::requestNodeFile(const std::string& fileName,osg::Group* group)
{
   
    // search to see if filename already exist in the file loaded list.
    bool foundEntry = false;

    _dataLoadedListMutex.lock();
    
        for(DatabaseRequestList::iterator litr = _dataLoadedList.begin();
            litr != _dataLoadedList.end() && !foundEntry;
            ++litr)
        {
            if ((*litr)->_fileName==fileName)
            {
                foundEntry = true;
                ++((*litr)->_numOfRequests);
            }
        }        

    _dataLoadedListMutex.unlock();

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

class DatabasePager::FindRenderingObjectsVisitor : public osg::NodeVisitor
{
public:
    FindRenderingObjectsVisitor(DatabasePager::DataToCompile& dataToCompile):
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
            _dataToCompile.first.push_back(stateset);
        }
    }
    
    inline void apply(osg::Drawable* drawable)
    {
        apply(drawable->getStateSet());

        if (drawable->getUseDisplayList() || drawable->getUseVertexBufferObjects())
        {
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

            _fileRequestListMutex.lock();
                _fileRequestList.erase(_fileRequestList.begin());
            _fileRequestListMutex.unlock();

            if (databaseRequest->_loadedModel.valid())
            {
                _dataLoadedListMutex.lock();

                    _dataLoadedList.push_back(databaseRequest);

                _dataLoadedListMutex.unlock();
            }        
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

void DatabasePager::addLoadedDataToSceneGraph()
{
    DatabaseRequestList localFileLoadedList;

    // get the dat for the _dataLoadedList, leaving it empty via a std::vector<>.swap.
    _dataLoadedListMutex.lock();
        localFileLoadedList.swap(_dataLoadedList);
    _dataLoadedListMutex.unlock();
    
    // add the loaded data into the scene graph.
    for(DatabaseRequestList::iterator itr=localFileLoadedList.begin();
        itr!=localFileLoadedList.end();
        ++itr)
    {
        DatabaseRequest* databaseRequest = itr->get();
        registerPagedLODs(databaseRequest->_loadedModel.get());
        databaseRequest->_groupForAddingLoadedSubgraph->addChild(databaseRequest->_loadedModel.get());
        //std::cout<<"merged subgraph"<<databaseRequest->_fileName<<" after "<<databaseRequest->_numOfRequests<<" requests."<<std::endl;

    }
    
}

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
        
    if (_deleteRemovedSubgraphsInDatabaseThread)
    {
        // transfer the removed children over to the to delete list so the database thread can delete them.
        _childrenToDeleteListMutex.lock();
            _childrenToDeleteList.insert(_childrenToDeleteList.begin(),childrenRemoved.begin(),childrenRemoved.end());
        _childrenToDeleteListMutex.unlock();
    }
    // otherwise the childrenRemoved list will automatically unref() and deleting the nodes.    
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
    FindPagedLODsVisitor fplv(_pagedLODList);
    if (subgraph) subgraph->accept(fplv);
}

void DatabasePager::compileRenderingObjects(osg::State& state)
{
//    std::cout<<"Compiling rendering objects"<<std::endl;
}

