/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#include <osgDB/DatabasePager>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osg/Geode>
#include <osg/Timer>
#include <osg/Texture>
#include <osg/Notify>
#include <osg/ProxyNode>
#include <osg/ApplicationUsage>

#include <OpenThreads/ScopedLock>

#include <algorithm>
#include <functional>
#include <set>
#include <iterator>

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace osgDB;
using namespace OpenThreads;

static osg::ApplicationUsageProxy DatabasePager_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DO_PRE_COMPILE <ON/OFF>","Switch on or off the pre compile of OpenGL object database pager.");
static osg::ApplicationUsageProxy DatabasePager_e3(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DATABASE_PAGER_DRAWABLE <mode>","Set the drawable policy for setting of loaded drawable to specified type.  mode can be one of DoNotModify, DisplayList, VBO or VertexArrays>.");
static osg::ApplicationUsageProxy DatabasePager_e4(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DATABASE_PAGER_PRIORITY <mode>", "Set the thread priority to DEFAULT, MIN, LOW, NOMINAL, HIGH or MAX.");
static osg::ApplicationUsageProxy DatabasePager_e11(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MAX_PAGEDLOD <num>","Set the target maximum number of PagedLOD to maintain.");
static osg::ApplicationUsageProxy DatabasePager_e12(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_ASSIGN_PBO_TO_IMAGES <ON/OFF>","Set whether PixelBufferObjects should be assigned to Images to aid download to the GPU.");

// Convert function objects that take pointer args into functions that a
// reference to an osg::ref_ptr. This is quite useful for doing STL
// operations on lists of ref_ptr. This code assumes that a function
// with an argument const Foo* should be composed into a function of
// argument type ref_ptr<Foo>&, not ref_ptr<const Foo>&. Some support
// for that should be added to make this more general.

namespace
{
template <typename U>
struct PointerTraits
{
    typedef class NullType {} PointeeType;
};

template <typename U>
struct PointerTraits<U*>
{
    typedef U PointeeType;
};

template <typename U>
struct PointerTraits<const U*>
{
    typedef U PointeeType;
};

template <typename FuncObj>
class RefPtrAdapter
    : public std::unary_function<const osg::ref_ptr<typename PointerTraits<typename FuncObj::argument_type>::PointeeType>,
                                 typename FuncObj::result_type>
{
public:
    typedef typename PointerTraits<typename FuncObj::argument_type>::PointeeType PointeeType;
    typedef osg::ref_ptr<PointeeType> RefPtrType;
    explicit RefPtrAdapter(const FuncObj& funcObj) : _func(funcObj) {}
    typename FuncObj::result_type operator()(const RefPtrType& refPtr) const
    {
        return _func(refPtr.get());
    }
protected:
        FuncObj _func;
};

template <typename FuncObj>
RefPtrAdapter<FuncObj> refPtrAdapt(const FuncObj& func)
{
    return RefPtrAdapter<FuncObj>(func);
}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CountPagedLODList
//
struct DatabasePager::DatabasePagerCompileCompletedCallback : public osgUtil::IncrementalCompileOperation::CompileCompletedCallback
{
    DatabasePagerCompileCompletedCallback(osgDB::DatabasePager* pager, osgDB::DatabasePager::DatabaseRequest* databaseRequest):
        _pager(pager),
        _databaseRequest(databaseRequest) {}

    virtual bool compileCompleted(osgUtil::IncrementalCompileOperation::CompileSet* /*compileSet*/)
    {
        _pager->compileCompleted(_databaseRequest.get());
        return true;
    }

    osgDB::DatabasePager*                               _pager;
    osg::ref_ptr<osgDB::DatabasePager::DatabaseRequest> _databaseRequest;
};


void DatabasePager::compileCompleted(DatabaseRequest* databaseRequest)
{
    //OSG_NOTICE<<"DatabasePager::compileCompleted("<<databaseRequest<<")"<<std::endl;
    _dataToCompileList->remove(databaseRequest);
    _dataToMergeList->add(databaseRequest);
}

// This class is a helper for the management of SetBasedPagedLODList.
class DatabasePager::ExpirePagedLODsVisitor : public osg::NodeVisitor
{
public:
    ExpirePagedLODsVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }

    META_NodeVisitor("osgDB","ExpirePagedLODsVisitor")

    virtual void apply(osg::PagedLOD& plod)
    {
        _childPagedLODs.insert(&plod);
        markRequestsExpired(&plod);
        traverse(plod);
    }

    // Remove expired children from a PagedLOD. On return
    // removedChildren contains the nodes removed by the call to
    // PagedLOD::removeExpiredChildren, and the _childPagedLODs member
    // contains all the PagedLOD objects found in those children's
    // subgraphs.
    bool removeExpiredChildrenAndFindPagedLODs(osg::PagedLOD* plod, double expiryTime, unsigned int expiryFrame, osg::NodeList& removedChildren)
    {
        size_t sizeBefore = removedChildren.size();

        plod->removeExpiredChildren(expiryTime, expiryFrame, removedChildren);

        for(size_t i = sizeBefore; i<removedChildren.size(); ++i)
        {
            removedChildren[i]->accept(*this);
        }
        return sizeBefore!=removedChildren.size();
    }

    typedef std::set<osg::ref_ptr<osg::PagedLOD> > PagedLODset;
    PagedLODset         _childPagedLODs;
private:
    void markRequestsExpired(osg::PagedLOD* plod)
    {
        unsigned numFiles = plod->getNumFileNames();
        for (unsigned i = 0; i < numFiles; ++i)
        {
            DatabasePager::DatabaseRequest* request
                = dynamic_cast<DatabasePager::DatabaseRequest*>(plod->getDatabaseRequest(i).get());
            if (request)
                request->_groupExpired = true;
        }
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SetBasedPagedLODList
//
class SetBasedPagedLODList : public DatabasePager::PagedLODList
{
public:

    typedef std::set< osg::observer_ptr<osg::PagedLOD> > PagedLODs;
    PagedLODs _pagedLODs;


    virtual PagedLODList* clone() { return new SetBasedPagedLODList(); }
    virtual void clear() { _pagedLODs.clear(); }
    virtual unsigned int size() { return _pagedLODs.size(); }

    virtual void removeExpiredChildren(
        int numberChildrenToRemove, double expiryTime, unsigned int expiryFrame,
        DatabasePager::ObjectList& childrenRemoved, bool visitActive)
    {
        int leftToRemove = numberChildrenToRemove;
        for(PagedLODs::iterator itr = _pagedLODs.begin();
            itr!=_pagedLODs.end() && leftToRemove > 0;
            )
        {
            osg::ref_ptr<osg::PagedLOD> plod;
            if (itr->lock(plod))
            {
                bool plodActive = expiryFrame < plod->getFrameNumberOfLastTraversal();
                if (visitActive==plodActive) // true if (visitActive && plodActive) OR (!visitActive &&!plodActive)
                {
                    DatabasePager::ExpirePagedLODsVisitor expirePagedLODsVisitor;
                    osg::NodeList expiredChildren; // expired PagedLODs
                    expirePagedLODsVisitor.removeExpiredChildrenAndFindPagedLODs(
                        plod.get(), expiryTime, expiryFrame, expiredChildren);
                    // Clear any expired PagedLODs out of the set
                    for (DatabasePager::ExpirePagedLODsVisitor::PagedLODset::iterator
                             citr = expirePagedLODsVisitor._childPagedLODs.begin(),
                             end = expirePagedLODsVisitor._childPagedLODs.end();
                         citr != end;
                        ++citr)
                    {
                        osg::observer_ptr<osg::PagedLOD> clod(*citr);
                        // This child PagedLOD cannot be equal to the
                        // PagedLOD pointed to by itr because it must be
                        // in itr's subgraph. Therefore erasing it doesn't
                        // invalidate itr.
                        if (_pagedLODs.erase(clod) > 0)
                            leftToRemove--;
                    }
                    std::copy(expiredChildren.begin(), expiredChildren.end(), std::back_inserter(childrenRemoved));
                }

                // advance the iterator to the next element
                ++itr;
            }
            else
            {
                _pagedLODs.erase(itr++);
                // numberChildrenToRemove includes possibly expired
                // observer pointers.
                leftToRemove--;
                OSG_INFO<<"DatabasePager::removeExpiredSubgraphs() _inactivePagedLOD has been invalidated, but ignored"<<std::endl;
            }
        }
    }

    virtual void removeNodes(osg::NodeList& nodesToRemove)
    {
        for(osg::NodeList::iterator itr = nodesToRemove.begin();
            itr != nodesToRemove.end();
            ++itr)
        {
            osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(itr->get());
            osg::observer_ptr<osg::PagedLOD> obs_ptr(plod);
            PagedLODs::iterator plod_itr = _pagedLODs.find(obs_ptr);
            if (plod_itr != _pagedLODs.end())
            {
                OSG_INFO<<"Removing node from PagedLOD list"<<std::endl;
                _pagedLODs.erase(plod_itr);
            }
        }
    }

    virtual void insertPagedLOD(const osg::observer_ptr<osg::PagedLOD>& plod)
    {
        if (_pagedLODs.count(plod)!=0)
        {
            OSG_NOTICE<<"Warning: SetBasedPagedLODList::insertPagedLOD("<<plod.get()<<") already inserted"<<std::endl;
            // abort();
            return;
        }

        // OSG_NOTICE<<"OK: SetBasedPagedLODList::insertPagedLOD("<<plod<<") inserting"<<std::endl;

        _pagedLODs.insert(plod);
    }

    virtual bool containsPagedLOD(const osg::observer_ptr<osg::PagedLOD>& plod) const
    {
        return (_pagedLODs.count(plod)!=0);
    }

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FindCompileableGLObjectsVisitor
//
class DatabasePager::FindCompileableGLObjectsVisitor : public osgUtil::StateToCompile
{
public:
    FindCompileableGLObjectsVisitor(const DatabasePager* pager):
            osgUtil::StateToCompile(osgUtil::GLObjectsVisitor::COMPILE_DISPLAY_LISTS|osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES),
            _pager(pager),
            _changeAutoUnRef(false), _valueAutoUnRef(false),
            _changeAnisotropy(false), _valueAnisotropy(1.0)
    {
        _assignPBOToImages = _pager->_assignPBOToImages;

        _changeAutoUnRef = _pager->_changeAutoUnRef;
        _valueAutoUnRef = _pager->_valueAutoUnRef;
        _changeAnisotropy = _pager->_changeAnisotropy;
        _valueAnisotropy = _pager->_valueAnisotropy;

        switch(_pager->_drawablePolicy)
        {
            case DatabasePager::DO_NOT_MODIFY_DRAWABLE_SETTINGS:
                // do nothing, leave settings as they came in from loaded database.
                // OSG_NOTICE<<"DO_NOT_MODIFY_DRAWABLE_SETTINGS"<<std::endl;
                break;
            case DatabasePager::USE_DISPLAY_LISTS:
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_ON_DISPLAY_LISTS;
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_OFF_VERTEX_BUFFER_OBJECTS;
                _mode = _mode & ~osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS;
                break;
            case DatabasePager::USE_VERTEX_BUFFER_OBJECTS:
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS;
                break;
            case DatabasePager::USE_VERTEX_ARRAYS:
                _mode = _mode & ~osgUtil::GLObjectsVisitor::SWITCH_ON_DISPLAY_LISTS;
                _mode = _mode & ~osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS;
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_OFF_DISPLAY_LISTS;
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_OFF_VERTEX_BUFFER_OBJECTS;
                break;
        }

        if (osgDB::Registry::instance()->getBuildKdTreesHint()==osgDB::Options::BUILD_KDTREES &&
            osgDB::Registry::instance()->getKdTreeBuilder())
        {
            _kdTreeBuilder = osgDB::Registry::instance()->getKdTreeBuilder()->clone();
        }
    }

    META_NodeVisitor("osgDB","FindCompileableGLObjectsVisitor")

    bool requiresCompilation() const { return !empty(); }

    virtual void apply(osg::Geode& geode)
    {
        StateToCompile::apply(geode);

        if (_kdTreeBuilder.valid())
        {
            geode.accept(*_kdTreeBuilder);
        }
    }

    void apply(osg::Texture& texture)
    {
        StateToCompile::apply(texture);

        if (_changeAutoUnRef)
        {
            texture.setUnRefImageDataAfterApply(_valueAutoUnRef);
        }

        if ((_changeAnisotropy && texture.getMaxAnisotropy() != _valueAnisotropy))
        {
            texture.setMaxAnisotropy(_valueAnisotropy);
        }
    }

    const DatabasePager*                    _pager;
    bool                                    _changeAutoUnRef;
    bool                                    _valueAutoUnRef;
    bool                                    _changeAnisotropy;
    float                                   _valueAnisotropy;
    osg::ref_ptr<osg::KdTreeBuilder>        _kdTreeBuilder;

protected:

    FindCompileableGLObjectsVisitor& operator = (const FindCompileableGLObjectsVisitor&) { return *this; }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SortFileRequestFunctor
//
struct DatabasePager::SortFileRequestFunctor
{
    bool operator() (const osg::ref_ptr<DatabasePager::DatabaseRequest>& lhs,const osg::ref_ptr<DatabasePager::DatabaseRequest>& rhs) const
    {
        if (lhs->_timestampLastRequest>rhs->_timestampLastRequest) return true;
        else if (lhs->_timestampLastRequest<rhs->_timestampLastRequest) return false;
        else return (lhs->_priorityLastRequest>rhs->_priorityLastRequest);
    }
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DatabaseRequest
//
void DatabasePager::DatabaseRequest::invalidate()
{
    OSG_INFO<<"   DatabasePager::DatabaseRequest::invalidate()."<<std::endl;
    _valid = false;
    _loadedModel = 0;
    _compileSet = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  RequestQueue
//
DatabasePager::RequestQueue::RequestQueue(DatabasePager* pager):
    _pager(pager),
    _frameNumberLastPruned(osg::UNINITIALIZED_FRAME_NUMBER)
{
}

DatabasePager::RequestQueue::~RequestQueue()
{
    OSG_INFO<<"DatabasePager::RequestQueue::~RequestQueue() Destructing queue."<<std::endl;
    for(RequestList::iterator itr = _requestList.begin();
        itr != _requestList.end();
        ++itr)
    {
        invalidate(itr->get());
    }
}

void DatabasePager::RequestQueue::invalidate(DatabaseRequest* dr)
{
    // OSG_NOTICE<<"DatabasePager::RequestQueue::invalidate(DatabaseRequest* dr) dr->_compileSet="<<dr->_compileSet.get()<<std::endl;
    // XXX _dr_mutex?
    osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet> compileSet;
    if (dr->_compileSet.lock(compileSet) && _pager->getIncrementalCompileOperation())
    {
        _pager->getIncrementalCompileOperation()->remove(compileSet.get());
    }


    dr->invalidate();
}


bool DatabasePager::RequestQueue::pruneOldRequestsAndCheckIfEmpty()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    unsigned int frameNumber = _pager->_frameNumber;
    if (_frameNumberLastPruned != frameNumber)
    {
        for(RequestQueue::RequestList::iterator citr = _requestList.begin();
            citr != _requestList.end();
            )
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
            if ((*citr)->isRequestCurrent(frameNumber))
            {
                ++citr;
            }
            else
            {
                invalidate(citr->get());

                OSG_INFO<<"DatabasePager::RequestQueue::pruneOldRequestsAndCheckIfEmpty(): Pruning "<<(*citr)<<std::endl;
                citr = _requestList.erase(citr);
            }
        }

        _frameNumberLastPruned = frameNumber;

        updateBlock();
    }

    return _requestList.empty();
}

bool DatabasePager::RequestQueue::empty()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    return _requestList.empty();
}

unsigned int DatabasePager::RequestQueue::size()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    return _requestList.size();
}

void DatabasePager::RequestQueue::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    for(RequestList::iterator citr = _requestList.begin();
        citr != _requestList.end();
        ++citr)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
        invalidate(citr->get());
    }

    _requestList.clear();

    _frameNumberLastPruned = _pager->_frameNumber;

    updateBlock();
}


void DatabasePager::RequestQueue::add(DatabasePager::DatabaseRequest* databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    addNoLock(databaseRequest);
}

void DatabasePager::RequestQueue::remove(DatabasePager::DatabaseRequest* databaseRequest)
{
    // OSG_NOTICE<<"DatabasePager::RequestQueue::remove(DatabaseRequest* databaseRequest)"<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    for(RequestList::iterator citr = _requestList.begin();
        citr != _requestList.end();
        ++citr)
    {
        if (citr->get()==databaseRequest)
        {
            // OSG_NOTICE<<"  done remove(DatabaseRequest* databaseRequest)"<<std::endl;
            _requestList.erase(citr);
            return;
        }
    }
}


void DatabasePager::RequestQueue::addNoLock(DatabasePager::DatabaseRequest* databaseRequest)
{
    _requestList.push_back(databaseRequest);
    updateBlock();
}

void DatabasePager::RequestQueue::swap(RequestList& requestList)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    _requestList.swap(requestList);
}

void DatabasePager::RequestQueue::takeFirst(osg::ref_ptr<DatabaseRequest>& databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    if (!_requestList.empty())
    {
        DatabasePager::SortFileRequestFunctor highPriority;

        RequestQueue::RequestList::iterator selected_itr = _requestList.end();

        int frameNumber = _pager->_frameNumber;

        for(RequestQueue::RequestList::iterator citr = _requestList.begin();
            citr != _requestList.end();
            )
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
            if ((*citr)->isRequestCurrent(frameNumber))
            {
                if (selected_itr==_requestList.end() || highPriority(*citr, *selected_itr))
                {
                    selected_itr = citr;
                }

                ++citr;
            }
            else
            {
                invalidate(citr->get());

                OSG_INFO<<"DatabasePager::RequestQueue::takeFirst(): Pruning "<<(*citr)<<std::endl;
                citr = _requestList.erase(citr);
            }

        }

        _frameNumberLastPruned = frameNumber;

        if (selected_itr != _requestList.end())
        {
            databaseRequest = *selected_itr;
            _requestList.erase(selected_itr);
            OSG_INFO<<" DatabasePager::RequestQueue::takeFirst() Found DatabaseRequest size()="<<_requestList.size()<<std::endl;
        }
        else
        {
            OSG_INFO<<" DatabasePager::RequestQueue::takeFirst() No suitable DatabaseRequest found size()="<<_requestList.size()<<std::endl;
        }

        updateBlock();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ReadQueue
//
DatabasePager::ReadQueue::ReadQueue(DatabasePager* pager, const std::string& name):
    RequestQueue(pager),
    _name(name)
{
    _block = new osg::RefBlock;
}

void DatabasePager::ReadQueue::updateBlock()
{
    _block->set((!_requestList.empty() || !_childrenToDeleteList.empty()) &&
                !_pager->_databasePagerThreadPaused);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DatabaseThread
//
DatabasePager::DatabaseThread::DatabaseThread(DatabasePager* pager, Mode mode, const std::string& name):
    _done(false),
    _active(false),
    _pager(pager),
    _mode(mode),
    _name(name)
{
}

DatabasePager::DatabaseThread::DatabaseThread(const DatabaseThread& dt, DatabasePager* pager):
    _done(false),
    _active(false),
    _pager(pager),
    _mode(dt._mode),
    _name(dt._name)
{
}

DatabasePager::DatabaseThread::~DatabaseThread()
{
    cancel();
}

int DatabasePager::DatabaseThread::cancel()
{
    int result = 0;

    if( isRunning() )
    {
        setDone(true);

        switch(_mode)
        {
            case(HANDLE_ALL_REQUESTS):
                _pager->_fileRequestQueue->release();
                break;
            case(HANDLE_NON_HTTP):
                _pager->_fileRequestQueue->release();
                break;
            case(HANDLE_ONLY_HTTP):
                _pager->_httpRequestQueue->release();
                break;
        }

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            // commenting out debug info as it was cashing crash on exit, presumable
            // due to OSG_NOTICE or std::cout destructing earlier than this destructor.
            // OSG_INFO<<"Waiting for DatabasePager::DatabaseThread to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }

        // _startThreadCalled = false;
    }

    //OSG_NOTICE<<"DatabasePager::DatabaseThread stopped running"<<std::endl;
    return result;

}

void DatabasePager::DatabaseThread::run()
{
    OSG_INFO<<_name<<": DatabasePager::DatabaseThread::run"<<std::endl;


    bool firstTime = true;

    osg::ref_ptr<DatabasePager::ReadQueue> read_queue;
    osg::ref_ptr<DatabasePager::ReadQueue> out_queue;

    switch(_mode)
    {
        case(HANDLE_ALL_REQUESTS):
            read_queue = _pager->_fileRequestQueue;
            break;
        case(HANDLE_NON_HTTP):
            read_queue = _pager->_fileRequestQueue;
            out_queue = _pager->_httpRequestQueue;
            break;
        case(HANDLE_ONLY_HTTP):
            read_queue = _pager->_httpRequestQueue;
            break;
    }


    do
    {
        _active = false;

        read_queue->block();

        if (_done)
        {
            break;
        }

        _active = true;

        OSG_INFO<<_name<<": _pager->size()= "<<read_queue->size()<<" to delete = "<<read_queue->_childrenToDeleteList.size()<<std::endl;



        //
        // delete any children if required.
        //
        if (_pager->_deleteRemovedSubgraphsInDatabaseThread/* && !(read_queue->_childrenToDeleteList.empty())*/)
        {
            ObjectList deleteList;
            {
                // Don't hold lock during destruction of deleteList
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(read_queue->_requestMutex);
                if (!read_queue->_childrenToDeleteList.empty())
                {
                    deleteList.swap(read_queue->_childrenToDeleteList);
                    read_queue->updateBlock();
                }
            }
        }

        //
        // load any subgraphs that are required.
        //
        osg::ref_ptr<DatabaseRequest> databaseRequest;
        read_queue->takeFirst(databaseRequest);

        bool readFromFileCache = false;

        osg::ref_ptr<FileCache> fileCache = osgDB::Registry::instance()->getFileCache();
        osg::ref_ptr<FileLocationCallback> fileLocationCallback = osgDB::Registry::instance()->getFileLocationCallback();
        osg::ref_ptr<Options> dr_loadOptions;
        std::string fileName;
        int frameNumberLastRequest = 0;
        if (databaseRequest.valid())
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
                dr_loadOptions = databaseRequest->_loadOptions;
                fileName = databaseRequest->_fileName;
                frameNumberLastRequest = databaseRequest->_frameNumberLastRequest;
            }
            if (dr_loadOptions.valid())
            {
                if (dr_loadOptions->getFileCache()) fileCache = dr_loadOptions->getFileCache();
                if (dr_loadOptions->getFileLocationCallback()) fileLocationCallback = dr_loadOptions->getFileLocationCallback();

                dr_loadOptions = dr_loadOptions->cloneOptions();
            }
            else
            {
                dr_loadOptions = new osgDB::Options;
            }

            dr_loadOptions->setTerrain(databaseRequest->_terrain);

            // disable the FileCache if the fileLocationCallback tells us that it isn't required for this request.
            if (fileLocationCallback.valid() && !fileLocationCallback->useFileCache()) fileCache = 0;


            // check if databaseRequest is still relevant
            if ((_pager->_frameNumber-frameNumberLastRequest)<=1)
            {

                // now check to see if this request is appropriate for this thread
                switch(_mode)
                {
                    case(HANDLE_ALL_REQUESTS):
                    {
                        // do nothing as this thread can handle the load
                        if (fileCache.valid() && fileCache->isFileAppropriateForFileCache(fileName))
                        {
                            if (fileCache->existsInCache(fileName))
                            {
                                readFromFileCache = true;
                            }
                        }
                        break;
                    }
                    case(HANDLE_NON_HTTP):
                    {
                        // check the cache first
                        bool isHighLatencyFileRequest = false;

                        if (fileLocationCallback.valid())
                        {
                            isHighLatencyFileRequest = fileLocationCallback->fileLocation(fileName, dr_loadOptions.get()) == FileLocationCallback::REMOTE_FILE;
                        }
                        else  if (fileCache.valid() && fileCache->isFileAppropriateForFileCache(fileName))
                        {
                            isHighLatencyFileRequest = true;
                        }

                        if (isHighLatencyFileRequest)
                        {
                            if (fileCache.valid() && fileCache->existsInCache(fileName))
                            {
                                readFromFileCache = true;
                            }
                            else
                            {
                                OSG_INFO<<_name<<": Passing http requests over "<<fileName<<std::endl;
                                out_queue->add(databaseRequest.get());
                                databaseRequest = 0;
                            }
                        }
                        break;
                    }
                    case(HANDLE_ONLY_HTTP):
                    {
                        // accept all requests, as we'll assume only high latency requests will have got here.
                        break;
                    }
                }
            }
            else
            {
                databaseRequest = 0;
            }
        }


        if (databaseRequest.valid())
        {

            // load the data, note safe to write to the databaseRequest since once
            // it is created this thread is the only one to write to the _loadedModel pointer.
            //OSG_NOTICE<<"In DatabasePager thread readNodeFile("<<databaseRequest->_fileName<<")"<<std::endl;
            //osg::Timer_t before = osg::Timer::instance()->tick();


            // assume that readNode is thread safe...
            ReaderWriter::ReadResult rr = readFromFileCache ?
                        fileCache->readNode(fileName, dr_loadOptions.get(), false) :
                        Registry::instance()->readNode(fileName, dr_loadOptions.get(), false);

            osg::ref_ptr<osg::Node> loadedModel;
            if (rr.validNode()) loadedModel = rr.getNode();
            if (rr.error()) OSG_WARN<<"Error in reading file "<<fileName<<" : "<<rr.message() << std::endl;
            if (rr.notEnoughMemory()) OSG_INFO<<"Not enought memory to load file "<<fileName << std::endl;

            if (loadedModel.valid() &&
                fileCache.valid() &&
                fileCache->isFileAppropriateForFileCache(fileName) &&
                !readFromFileCache)
            {
                fileCache->writeNode(*(loadedModel), fileName, dr_loadOptions.get());
            }

            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
                if ((_pager->_frameNumber-databaseRequest->_frameNumberLastRequest)>1)
                {
                    OSG_INFO<<_name<<": Warning DatabaseRquest no longer required."<<std::endl;
                    loadedModel = 0;
                }
            }

            //OSG_NOTICE<<"     node read in "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms"<<std::endl;

            if (loadedModel.valid())
            {
                loadedModel->getBound();

                // find all the compileable rendering objects
                DatabasePager::FindCompileableGLObjectsVisitor stateToCompile(_pager);
                loadedModel->accept(stateToCompile);

                bool loadedObjectsNeedToBeCompiled = _pager->_doPreCompile &&
                                                     _pager->_incrementalCompileOperation.valid() &&
                                                     _pager->_incrementalCompileOperation->requiresCompile(stateToCompile);

                // move the databaseRequest from the front of the fileRequest to the end of
                // dataToCompile or dataToMerge lists.
                osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet> compileSet = 0;
                if (loadedObjectsNeedToBeCompiled)
                {
                    // OSG_NOTICE<<"Using IncrementalCompileOperation"<<std::endl;

                    compileSet = new osgUtil::IncrementalCompileOperation::CompileSet(loadedModel.get());
                    compileSet->buildCompileMap(_pager->_incrementalCompileOperation->getContextSet(), stateToCompile);
                    compileSet->_compileCompletedCallback = new DatabasePagerCompileCompletedCallback(_pager, databaseRequest.get());
                    _pager->_incrementalCompileOperation->add(compileSet.get(), false);
                }
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
                    databaseRequest->_loadedModel = loadedModel;
                    databaseRequest->_compileSet = compileSet;
                }
                // Dereference the databaseRequest while the queue is
                // locked. This prevents the request from being
                // deleted at an unpredictable time within
                // addLoadedDataToSceneGraph.
                if (loadedObjectsNeedToBeCompiled)
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> listLock(
                        _pager->_dataToCompileList->_requestMutex);
                    _pager->_dataToCompileList->addNoLock(databaseRequest.get());
                    databaseRequest = 0;
                }
                else
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> listLock(
                        _pager->_dataToMergeList->_requestMutex);
                    _pager->_dataToMergeList->addNoLock(databaseRequest.get());
                    databaseRequest = 0;
                }

            }

            // _pager->_dataToCompileList->pruneOldRequestsAndCheckIfEmpty();
        }
        else
        {
            OpenThreads::Thread::YieldCurrentThread();
        }


        // go to sleep till our the next time our thread gets scheduled.

        if (firstTime)
        {
            // do a yield to get round a peculiar thread hang when testCancel() is called
            // in certain circumstances - of which there is no particular pattern.
            YieldCurrentThread();
            firstTime = false;
        }

    } while (!testCancel() && !_done);
}


DatabasePager::DatabasePager()
{
    //OSG_INFO<<"Constructing DatabasePager()"<<std::endl;

    _startThreadCalled = false;

    _done = false;
    _acceptNewRequests = true;
    _databasePagerThreadPaused = false;

    _numFramesActive = 0;
    _frameNumber.exchange(0);


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

    _assignPBOToImages = false;
    if( (str = getenv("OSG_ASSIGN_PBO_TO_IMAGES")) != 0)
    {
        _assignPBOToImages = strcmp(str,"yes")==0 || strcmp(str,"YES")==0 ||
                             strcmp(str,"on")==0 || strcmp(str,"ON")==0;

        OSG_NOTICE<<"OSG_ASSIGN_PBO_TO_IMAGES set to "<<_assignPBOToImages<<std::endl;
    }

    _changeAutoUnRef = true;
    _valueAutoUnRef = false;

    _changeAnisotropy = false;
    _valueAnisotropy = 1.0f;


    _deleteRemovedSubgraphsInDatabaseThread = true;
    if( (str = getenv("OSG_DELETE_IN_DATABASE_THREAD")) != 0)
    {
        _deleteRemovedSubgraphsInDatabaseThread = strcmp(str,"yes")==0 || strcmp(str,"YES")==0 ||
                                                  strcmp(str,"on")==0 || strcmp(str,"ON")==0;

    }

    _targetMaximumNumberOfPageLOD = 300;
    if( (str = getenv("OSG_MAX_PAGEDLOD")) != 0)
    {
        _targetMaximumNumberOfPageLOD = atoi(str);
        OSG_NOTICE<<"_targetMaximumNumberOfPageLOD = "<<_targetMaximumNumberOfPageLOD<<std::endl;
    }


    _doPreCompile = true;
    if( (str = getenv("OSG_DO_PRE_COMPILE")) != 0)
    {
        _doPreCompile = strcmp(str,"yes")==0 || strcmp(str,"YES")==0 ||
                        strcmp(str,"on")==0 || strcmp(str,"ON")==0;
    }

    // initialize the stats variables
    resetStats();

    _fileRequestQueue = new ReadQueue(this,"fileRequestQueue");
    _httpRequestQueue = new ReadQueue(this,"httpRequestQueue");

    _dataToCompileList = new RequestQueue(this);
    _dataToMergeList = new RequestQueue(this);

    setUpThreads(
        osg::DisplaySettings::instance()->getNumOfDatabaseThreadsHint(),
        osg::DisplaySettings::instance()->getNumOfHttpDatabaseThreadsHint());

    str = getenv("OSG_DATABASE_PAGER_PRIORITY");
    if (str)
    {
        if (strcmp(str,"DEFAULT")==0)
        {
            setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_DEFAULT);
        }
        else if (strcmp(str,"MIN")==0)
        {
            setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_MIN);
        }
        else if (strcmp(str,"LOW")==0)
        {
            setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_LOW);
        }
        else if (strcmp(str,"NOMINAL")==0)
        {
            setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_NOMINAL);
        }
        else if (strcmp(str,"HIGH")==0)
        {
            setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_HIGH);
        }
        else if (strcmp(str,"MAX")==0)
        {
            setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_MAX);
        }
    }

    _activePagedLODList = new SetBasedPagedLODList;
}

DatabasePager::DatabasePager(const DatabasePager& rhs)
{
    //OSG_INFO<<"Constructing DatabasePager(const DatabasePager& )"<<std::endl;

    _startThreadCalled = false;

    _done = false;
    _acceptNewRequests = true;
    _databasePagerThreadPaused = false;

    _numFramesActive = 0;
    _frameNumber.exchange(0);

    _drawablePolicy = rhs._drawablePolicy;

    _assignPBOToImages = rhs._assignPBOToImages;

    _changeAutoUnRef = rhs._changeAutoUnRef;
    _valueAutoUnRef = rhs._valueAutoUnRef;
    _changeAnisotropy = rhs._changeAnisotropy;
    _valueAnisotropy = rhs._valueAnisotropy;

    _deleteRemovedSubgraphsInDatabaseThread = rhs._deleteRemovedSubgraphsInDatabaseThread;

    _targetMaximumNumberOfPageLOD = rhs._targetMaximumNumberOfPageLOD;

    _doPreCompile = rhs._doPreCompile;

    _fileRequestQueue = new ReadQueue(this,"fileRequestQueue");
    _httpRequestQueue = new ReadQueue(this,"httpRequestQueue");

    _dataToCompileList = new RequestQueue(this);
    _dataToMergeList = new RequestQueue(this);

    for(DatabaseThreadList::const_iterator dt_itr = rhs._databaseThreads.begin();
        dt_itr != rhs._databaseThreads.end();
        ++dt_itr)
    {
        _databaseThreads.push_back(new DatabaseThread(**dt_itr,this));
    }

    _activePagedLODList = rhs._activePagedLODList->clone();

#if 1
    // need to set the display list manager to be able to reuse display lists
    osg::Drawable::setMinimumNumberOfDisplayListsToRetainInCache(100);
#else
    // need to set the display list manager to be able to reuse display lists
    osg::Drawable::setMinimumNumberOfDisplayListsToRetainInCache(0);
#endif

    // initialize the stats variables
    resetStats();
}


void DatabasePager::setIncrementalCompileOperation(osgUtil::IncrementalCompileOperation* ico)
{
    _incrementalCompileOperation = ico;
}

DatabasePager::~DatabasePager()
{
    // cancel the threads
    cancel();

    // destruct all the threads
    _databaseThreads.clear();

    // destruct all the queues
    _fileRequestQueue = 0;
    _httpRequestQueue = 0;
    _dataToCompileList = 0;
    _dataToMergeList = 0;

    // remove reference to the ICO
    _incrementalCompileOperation = 0;

    //_activePagedLODList;
    //_inactivePagedLODList;
}

osg::ref_ptr<DatabasePager>& DatabasePager::prototype()
{
    static osg::ref_ptr<DatabasePager> s_DatabasePager = new DatabasePager;
    return s_DatabasePager;
}

DatabasePager* DatabasePager::create()
{
    return DatabasePager::prototype().valid() ?
           DatabasePager::prototype()->clone() :
           new DatabasePager;
}

void DatabasePager::setUpThreads(unsigned int totalNumThreads, unsigned int numHttpThreads)
{
    _databaseThreads.clear();

    unsigned int numGeneralThreads = numHttpThreads < totalNumThreads ?
        totalNumThreads - numHttpThreads :
        1;

    if (numHttpThreads==0)
    {
        for(unsigned int i=0; i<numGeneralThreads; ++i)
        {
            addDatabaseThread(DatabaseThread::HANDLE_ALL_REQUESTS,"HANDLE_ALL_REQUESTS");
        }
    }
    else
    {
        for(unsigned int i=0; i<numGeneralThreads; ++i)
        {
            addDatabaseThread(DatabaseThread::HANDLE_NON_HTTP, "HANDLE_NON_HTTP");
        }

        for(unsigned int i=0; i<numHttpThreads; ++i)
        {
            addDatabaseThread(DatabaseThread::HANDLE_ONLY_HTTP, "HANDLE_ONLY_HTTP");
        }
    }
}

unsigned int DatabasePager::addDatabaseThread(DatabaseThread::Mode mode, const std::string& name)
{
    OSG_INFO<<"DatabasePager::addDatabaseThread() "<<name<<std::endl;

    unsigned int pos = _databaseThreads.size();

    DatabaseThread* thread = new DatabaseThread(this, mode,name);
    _databaseThreads.push_back(thread);

    if (_startThreadCalled)
    {
        OSG_INFO<<"DatabasePager::startThread()"<<std::endl;
        thread->startThread();
    }

    return pos;
}

int DatabasePager::setSchedulePriority(OpenThreads::Thread::ThreadPriority priority)
{
    int result = 0;
    for(DatabaseThreadList::iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        result = (*dt_itr)->setSchedulePriority(priority);
    }
    return result;
}

bool DatabasePager::isRunning() const
{
    for(DatabaseThreadList::const_iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        if ((*dt_itr)->isRunning()) return true;
    }

    return false;
}

int DatabasePager::cancel()
{
    int result = 0;

    for(DatabaseThreadList::iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        (*dt_itr)->setDone(true);
    }

    // release the queue blocks in case they are holding up thread cancellation.
    _fileRequestQueue->release();
    _httpRequestQueue->release();

    for(DatabaseThreadList::iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        (*dt_itr)->cancel();
    }

    _done = true;
    _startThreadCalled = false;

    return result;
}

void DatabasePager::clear()
{
    _fileRequestQueue->clear();
    _httpRequestQueue->clear();

    _dataToCompileList->clear();
    _dataToMergeList->clear();

    // note, no need to use a mutex as the list is only accessed from the update thread.
    _activePagedLODList->clear();

    // ??
    // _activeGraphicsContexts
}

void DatabasePager::resetStats()
{
    // initialize the stats variables
    _minimumTimeToMergeTile = DBL_MAX;
    _maximumTimeToMergeTile = -DBL_MAX;
    _totalTimeToMergeTiles = 0.0;
    _numTilesMerges = 0;
}

bool DatabasePager::getRequestsInProgress() const
{
    if (getFileRequestListSize()>0) return true;

    if (getDataToCompileListSize()>0)
    {
        return true;
    }

    if (getDataToMergeListSize()>0) return true;

    for(DatabaseThreadList::const_iterator itr = _databaseThreads.begin();
        itr != _databaseThreads.end();
        ++itr)
    {
        if ((*itr)->getActive()) return true;
    }
    return false;
}


void DatabasePager::requestNodeFile(const std::string& fileName, osg::NodePath& nodePath,
                                    float priority, const osg::FrameStamp* framestamp,
                                    osg::ref_ptr<osg::Referenced>& databaseRequestRef,
                                    const osg::Referenced* options)
{
    osgDB::Options* loadOptions = dynamic_cast<osgDB::Options*>(const_cast<osg::Referenced*>(options));
    if (!loadOptions)
    {
       loadOptions = Registry::instance()->getOptions();

        // OSG_NOTICE<<"Using options from Registry "<<std::endl;
    }
    else
    {
        // OSG_NOTICE<<"options from requestNodeFile "<<std::endl;
    }


    if (!_acceptNewRequests) return;


    if (nodePath.empty())
    {
        OSG_NOTICE<<"Warning: DatabasePager::requestNodeFile(..) passed empty NodePath, so nowhere to attach new subgraph to."<<std::endl;
        return;
    }

    osg::Group* group = nodePath.back()->asGroup();
    if (!group)
    {
        OSG_NOTICE<<"Warning: DatabasePager::requestNodeFile(..) passed NodePath without group as last node in path, so nowhere to attach new subgraph to."<<std::endl;
        return;
    }

    osg::Node* terrain = 0;
    for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
        itr != nodePath.rend();
        ++itr)
    {
        if ((*itr)->asTerrain()) terrain = *itr;
    }

    double timestamp = framestamp?framestamp->getReferenceTime():0.0;
    unsigned int frameNumber = framestamp?framestamp->getFrameNumber():static_cast<unsigned int>(_frameNumber);

// #define WITH_REQUESTNODEFILE_TIMING
#ifdef WITH_REQUESTNODEFILE_TIMING
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    static int previousFrame = -1;
    static double totalTime = 0.0;

    if (previousFrame!=frameNumber)
    {
        OSG_NOTICE<<"requestNodeFiles for "<<previousFrame<<" time = "<<totalTime<<std::endl;

        previousFrame = frameNumber;
        totalTime = 0.0;
    }
#endif

    // search to see if filename already exist in the file loaded list.
    bool foundEntry = false;

    if (databaseRequestRef.valid())
    {
        DatabaseRequest* databaseRequest = dynamic_cast<DatabaseRequest*>(databaseRequestRef.get());
        bool requeue = false;
        if (databaseRequest)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_dr_mutex);
            if (!(databaseRequest->valid()))
            {
                OSG_INFO<<"DatabaseRequest has been previously invalidated whilst still attached to scene graph."<<std::endl;
                databaseRequest = 0;
            }
            else
            {
                OSG_INFO<<"DatabasePager::requestNodeFile("<<fileName<<") updating already assigned."<<std::endl;


                databaseRequest->_valid = true;
                databaseRequest->_frameNumberLastRequest = frameNumber;
                databaseRequest->_timestampLastRequest = timestamp;
                databaseRequest->_priorityLastRequest = priority;
                ++(databaseRequest->_numOfRequests);

                foundEntry = true;

                if (databaseRequestRef->referenceCount()==1)
                {
                    OSG_INFO<<"DatabasePager::requestNodeFile("<<fileName<<") orphaned, resubmitting."<<std::endl;

                    databaseRequest->_frameNumberLastRequest = frameNumber;
                    databaseRequest->_timestampLastRequest = timestamp;
                    databaseRequest->_priorityLastRequest = priority;
                    databaseRequest->_group = group;
                    databaseRequest->_terrain = terrain;
                    databaseRequest->_loadOptions = loadOptions;
                    requeue = true;
                }

            }
        }
        if (requeue)
            _fileRequestQueue->add(databaseRequest);
    }

    if (!foundEntry)
    {
        OSG_INFO<<"In DatabasePager::requestNodeFile("<<fileName<<")"<<std::endl;

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_requestMutex);

        if (!databaseRequestRef.valid() || databaseRequestRef->referenceCount()==1)
        {
            osg::ref_ptr<DatabaseRequest> databaseRequest = new DatabaseRequest;

            databaseRequestRef = databaseRequest.get();

            databaseRequest->_valid = true;
            databaseRequest->_fileName = fileName;
            databaseRequest->_frameNumberFirstRequest = frameNumber;
            databaseRequest->_timestampFirstRequest = timestamp;
            databaseRequest->_priorityFirstRequest = priority;
            databaseRequest->_frameNumberLastRequest = frameNumber;
            databaseRequest->_timestampLastRequest = timestamp;
            databaseRequest->_priorityLastRequest = priority;
            databaseRequest->_group = group;
            databaseRequest->_terrain = terrain;
            databaseRequest->_loadOptions = loadOptions;

            _fileRequestQueue->addNoLock(databaseRequest.get());
        }
    }

    if (!_startThreadCalled)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_run_mutex);

        if (!_startThreadCalled)
        {
            _startThreadCalled = true;
            _done = false;
            OSG_INFO<<"DatabasePager::startThread()"<<std::endl;

            if (_databaseThreads.empty())
            {
                setUpThreads(
                    osg::DisplaySettings::instance()->getNumOfDatabaseThreadsHint(),
                    osg::DisplaySettings::instance()->getNumOfHttpDatabaseThreadsHint());
            }

            for(DatabaseThreadList::const_iterator dt_itr = _databaseThreads.begin();
                dt_itr != _databaseThreads.end();
                ++dt_itr)
            {
                (*dt_itr)->startThread();
            }
        }
    }

#ifdef WITH_REQUESTNODEFILE_TIMING
    totalTime += osg::Timer::instance()->delta_m(start_tick, osg::Timer::instance()->tick());
#endif
}

void DatabasePager::signalBeginFrame(const osg::FrameStamp* framestamp)
{
#if 0
    OSG_NOTICE<<"DatabasePager : _fileRequestQueue->size()="<<_fileRequestQueue->size()
               <<", _httpRequestQueue->size()= "<<_httpRequestQueue->size()
               <<", _dataToCompileList->size()= "<<_dataToCompileList->size()
               <<", _dataToMergeList->size()= "<<_dataToMergeList->size()<<std::endl;
#endif
    if (framestamp)
    {
        _dataToCompileList->pruneOldRequestsAndCheckIfEmpty();

        //OSG_INFO << "signalBeginFrame "<<framestamp->getFrameNumber()<<">>>>>>>>>>>>>>>>"<<std::endl;
        _frameNumber.exchange(framestamp->getFrameNumber());

    } //else OSG_INFO << "signalBeginFrame >>>>>>>>>>>>>>>>"<<std::endl;
}

void DatabasePager::signalEndFrame()
{
    //OSG_INFO << "signalEndFrame <<<<<<<<<<<<<<<<<<<< "<<std::endl;
}

void DatabasePager::setDatabasePagerThreadPause(bool pause)
{
    if (_databasePagerThreadPaused == pause) return;

    _databasePagerThreadPaused = pause;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_requestMutex);
        _fileRequestQueue->updateBlock();
    }
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_httpRequestQueue->_requestMutex);
        _httpRequestQueue->updateBlock();
    }
}


bool DatabasePager::requiresUpdateSceneGraph() const
{
    return !(_dataToMergeList->empty());
}

void DatabasePager::updateSceneGraph(const osg::FrameStamp& frameStamp)
{

#define UPDATE_TIMING 0
#if UPDATE_TIMING
    osg::ElapsedTime timer;
    double timeFor_removeExpiredSubgraphs, timeFor_addLoadedDataToSceneGraph;
#endif

    {
        removeExpiredSubgraphs(frameStamp);

#if UPDATE_TIMING
        timeFor_removeExpiredSubgraphs = timer.elapsedTime_m();
#endif

        addLoadedDataToSceneGraph(frameStamp);

#if UPDATE_TIMING
        timeFor_addLoadedDataToSceneGraph = timer.elapsedTime_m() - timeFor_removeExpiredSubgraphs;
#endif

    }

#if UPDATE_TIMING
    double elapsedTime = timer.elapsedTime_m();
    if (elapsedTime>0.4)
    {
        OSG_NOTICE<<"DatabasePager::updateSceneGraph() total time = "<<elapsedTime<<"ms"<<std::endl;
        OSG_NOTICE<<"   timeFor_removeExpiredSubgraphs    = "<<timeFor_removeExpiredSubgraphs<<"ms"<<std::endl;
        OSG_NOTICE<<"   timeFor_addLoadedDataToSceneGraph = "<<timeFor_addLoadedDataToSceneGraph<<"ms"<<std::endl;
        OSG_NOTICE<<"   _activePagedLODList.size()        = "<<_activePagedLODList->size()<<std::endl;
        OSG_NOTICE<<"   _inactivePagedLODList.size()      = "<<_inactivePagedLODList->size()<<std::endl;
        OSG_NOTICE<<"   total                             = "<<_activePagedLODList->size() + _inactivePagedLODList->size()<<std::endl;
    }
#endif
}


void DatabasePager::addLoadedDataToSceneGraph(const osg::FrameStamp &frameStamp)
{
    double timeStamp = frameStamp.getReferenceTime();
    unsigned int frameNumber = frameStamp.getFrameNumber();

    osg::Timer_t before = osg::Timer::instance()->tick(), mid, last;

    RequestQueue::RequestList localFileLoadedList;

    // get the data from the _dataToMergeList, leaving it empty via a std::vector<>.swap.
    _dataToMergeList->swap(localFileLoadedList);

    mid = osg::Timer::instance()->tick();

    // add the loaded data into the scene graph.
    for(RequestQueue::RequestList::iterator itr=localFileLoadedList.begin();
        itr!=localFileLoadedList.end();
        ++itr)
    {
        DatabaseRequest* databaseRequest = itr->get();

        // No need to take _dr_mutex. The pager threads are done with
        // the request; the cull traversal -- which might redispatch
        // the request -- can't run at the sametime as this update traversal.
        osg::ref_ptr<osg::Group> group;
        if (!databaseRequest->_groupExpired && databaseRequest->_group.lock(group))
        {
            if (osgDB::Registry::instance()->getSharedStateManager())
                osgDB::Registry::instance()->getSharedStateManager()->share(databaseRequest->_loadedModel.get());

            osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(group.get());
            if (plod)
            {
                plod->setTimeStamp(plod->getNumChildren(), timeStamp);
                plod->setFrameNumber(plod->getNumChildren(), frameNumber);
                plod->getDatabaseRequest(plod->getNumChildren()) = 0;
            }
            else
            {
                osg::ProxyNode* proxyNode = dynamic_cast<osg::ProxyNode*>(group.get());
                if (proxyNode)
                {
                    proxyNode->getDatabaseRequest(proxyNode->getNumChildren()) = 0;
                }
            }

            group->addChild(databaseRequest->_loadedModel.get());

            // Check if parent plod was already registered if not start visitor from parent
            if( plod &&
                !_activePagedLODList->containsPagedLOD( plod ) )
            {
                registerPagedLODs(plod, frameNumber);
            }
            else
            {
                registerPagedLODs(databaseRequest->_loadedModel.get(), frameNumber);
            }

            // OSG_NOTICE<<"merged subgraph"<<databaseRequest->_fileName<<" after "<<databaseRequest->_numOfRequests<<" requests and time="<<(timeStamp-databaseRequest->_timestampFirstRequest)*1000.0<<std::endl;

            double timeToMerge = timeStamp-databaseRequest->_timestampFirstRequest;

            if (timeToMerge<_minimumTimeToMergeTile) _minimumTimeToMergeTile = timeToMerge;
            if (timeToMerge>_maximumTimeToMergeTile) _maximumTimeToMergeTile = timeToMerge;

            _totalTimeToMergeTiles += timeToMerge;
            ++_numTilesMerges;
        }
        else
        {
            OSG_INFO<<"DatabasePager::addLoadedDataToSceneGraph() node in parental chain deleted, discarding subgaph."<<std::endl;
        }

        // reset the loadedModel pointer
        databaseRequest->_loadedModel = 0;

        // OSG_NOTICE<<"curr = "<<timeToMerge<<" min "<<getMinimumTimeToMergeTile()*1000.0<<" max = "<<getMaximumTimeToMergeTile()*1000.0<<" average = "<<getAverageTimToMergeTiles()*1000.0<<std::endl;
    }

    last = osg::Timer::instance()->tick();

    if (!localFileLoadedList.empty())
    {
        OSG_INFO<<"Done DatabasePager::addLoadedDataToSceneGraph"<<
            osg::Timer::instance()->delta_m(before,mid)<<"ms,\t"<<
            osg::Timer::instance()->delta_m(mid,last)<<"ms"<<
            "  objects"<<localFileLoadedList.size()<<std::endl<<std::endl;
    }

}



void DatabasePager::removeExpiredSubgraphs(const osg::FrameStamp& frameStamp)
{

    static double s_total_iter_stage_a = 0.0;
    static double s_total_time_stage_a = 0.0;
    static double s_total_max_stage_a = 0.0;

    static double s_total_iter_stage_b = 0.0;
    static double s_total_time_stage_b = 0.0;
    static double s_total_max_stage_b = 0.0;

    static double s_total_iter_stage_c = 0.0;
    static double s_total_time_stage_c = 0.0;
    static double s_total_max_stage_c = 0.0;

    if (frameStamp.getFrameNumber()==0)
    {
        // No need to remove anything on first frame.
        return;
    }


    osg::Timer_t startTick = osg::Timer::instance()->tick();

    // numPagedLODs >= actual number of PagedLODs. There can be
    // invalid observer pointers in _activePagedLODList.
    unsigned int numPagedLODs = _activePagedLODList->size();

    osg::Timer_t end_a_Tick = osg::Timer::instance()->tick();
    double time_a = osg::Timer::instance()->delta_m(startTick,end_a_Tick);

    s_total_iter_stage_a += 1.0;
    s_total_time_stage_a += time_a;
    if (s_total_max_stage_a<time_a) s_total_max_stage_a = time_a;


    if (numPagedLODs <= _targetMaximumNumberOfPageLOD)
    {
        // nothing to do
        return;
    }

    int numToPrune = numPagedLODs - _targetMaximumNumberOfPageLOD;

    ObjectList childrenRemoved;

    double expiryTime = frameStamp.getReferenceTime() - 0.1;
    unsigned int expiryFrame = frameStamp.getFrameNumber() - 1;

    // First traverse inactive PagedLODs, as their children will
    // certainly have expired. Then traverse active nodes if we still
    // need to prune.
    //OSG_NOTICE<<"numToPrune "<<numToPrune;
    if (numToPrune>0)
        _activePagedLODList->removeExpiredChildren(
            numToPrune, expiryTime, expiryFrame, childrenRemoved, false);
    numToPrune = _activePagedLODList->size() - _targetMaximumNumberOfPageLOD;
    if (numToPrune>0)
        _activePagedLODList->removeExpiredChildren(
            numToPrune, expiryTime, expiryFrame, childrenRemoved, true);

    osg::Timer_t end_b_Tick = osg::Timer::instance()->tick();
    double time_b = osg::Timer::instance()->delta_m(end_a_Tick,end_b_Tick);

    s_total_iter_stage_b += 1.0;
    s_total_time_stage_b += time_b;
    if (s_total_max_stage_b<time_b) s_total_max_stage_b = time_b;

    //OSG_NOTICE<<" childrenRemoved.size()="<<childrenRemoved.size()<<std::endl;

    if (!childrenRemoved.empty())
    {
        // pass the objects across to the database pager delete list
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_requestMutex);
            // splice transfers the entire list in constant time.
            _fileRequestQueue->_childrenToDeleteList.splice(
                _fileRequestQueue->_childrenToDeleteList.end(),
                childrenRemoved);
            _fileRequestQueue->updateBlock();
        }
        else
            childrenRemoved.clear();
    }

    osg::Timer_t end_c_Tick = osg::Timer::instance()->tick();
    double time_c = osg::Timer::instance()->delta_m(end_b_Tick,end_c_Tick);

    s_total_iter_stage_c += 1.0;
    s_total_time_stage_c += time_c;
    if (s_total_max_stage_c<time_c) s_total_max_stage_c = time_c;

    OSG_INFO<<"active="<<_activePagedLODList->size()<<" overall = "<<osg::Timer::instance()->delta_m(startTick,end_c_Tick)<<
                              " A="<<time_a<<" avg="<<s_total_time_stage_a/s_total_iter_stage_a<<" max = "<<s_total_max_stage_a<<
                              " B="<<time_b<<" avg="<<s_total_time_stage_b/s_total_iter_stage_b<<" max = "<<s_total_max_stage_b<<
                              " C="<<time_c<<" avg="<<s_total_time_stage_c/s_total_iter_stage_c<<" max = "<<s_total_max_stage_c<<std::endl;
}

class DatabasePager::FindPagedLODsVisitor : public osg::NodeVisitor
{
public:

    FindPagedLODsVisitor(DatabasePager::PagedLODList& pagedLODList, unsigned int frameNumber):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _activePagedLODList(pagedLODList),
        _frameNumber(frameNumber)
    {
    }

    META_NodeVisitor("osgDB","FindPagedLODsVisitor")

    virtual void apply(osg::PagedLOD& plod)
    {
        plod.setFrameNumberOfLastTraversal(_frameNumber);

        osg::observer_ptr<osg::PagedLOD> obs_ptr(&plod);
        _activePagedLODList.insertPagedLOD(obs_ptr);

        traverse(plod);
    }

    DatabasePager::PagedLODList& _activePagedLODList;
    unsigned int _frameNumber;

protected:

    FindPagedLODsVisitor& operator = (const FindPagedLODsVisitor&) { return *this; }
};


void DatabasePager::registerPagedLODs(osg::Node* subgraph, unsigned int frameNumber)
{
    if (!subgraph) return;

    FindPagedLODsVisitor fplv(*_activePagedLODList, frameNumber);
    subgraph->accept(fplv);
}
