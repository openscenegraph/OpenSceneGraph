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
static osg::ApplicationUsageProxy DatabasePager_e1(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MINIMUM_COMPILE_TIME_PER_FRAME <float>","minimum compile time alloted to compiling OpenGL objects per frame in database pager.");
static osg::ApplicationUsageProxy DatabasePager_e2(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MAXIMUM_OBJECTS_TO_COMPILE_PER_FRAME <int>","maximum number of OpenGL objects to compile per frame in database pager.");
static osg::ApplicationUsageProxy DatabasePager_e3(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DATABASE_PAGER_DRAWABLE <mode>","Set the drawable policy for setting of loaded drawable to specified type.  mode can be one of DoNotModify, DisplayList, VBO or VertexArrays>.");
static osg::ApplicationUsageProxy DatabasePager_e4(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DATABASE_PAGER_PRIORITY <mode>", "Set the thread priority to DEFAULT, MIN, LOW, NOMINAL, HIGH or MAX.");
static osg::ApplicationUsageProxy DatabasePager_e7(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_EXPIRY_DELAY <float> ","Set the length of time a PagedLOD child is kept in memory, without being used, before its tagged as expired, and ear marked to deletion.");
static osg::ApplicationUsageProxy DatabasePager_e8(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_EXPIRY_FRAMES <int> ","Set number of frames a PagedLOD child is kept in memory, without being used, before its tagged as expired, and ear marked to deletion.");
static osg::ApplicationUsageProxy DatabasePager_e9(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_RELEASE_DELAY <float> ","Set the length of time a PagedLOD child's OpenGL objects are kept in memory, without being used, before be released (setting to OFF disables this feature.)");
static osg::ApplicationUsageProxy DatabasePager_e10(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_RELEASE_FRAMES <int> ","Set number of frames a PagedLOD child's OpenGL objects are kept in memory, without being used, before be released.");
static osg::ApplicationUsageProxy DatabasePager_e11(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MAX_PAGEDLOD <num>","Set the target maximum number of PagedLOD to maintain.");

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
//  FindCompileableGLObjectsVisitor
//
class DatabasePager::FindCompileableGLObjectsVisitor : public osg::NodeVisitor
{
public:
    FindCompileableGLObjectsVisitor(DatabasePager::DataToCompile* dataToCompile,
                               bool changeAutoUnRef, bool valueAutoUnRef,
                               bool changeAnisotropy, float valueAnisotropy,
                               DatabasePager::DrawablePolicy drawablePolicy,
                               const DatabasePager* pager):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _dataToCompile(dataToCompile),
            _changeAutoUnRef(changeAutoUnRef), _valueAutoUnRef(valueAutoUnRef),
            _changeAnisotropy(changeAnisotropy), _valueAnisotropy(valueAnisotropy),
            _drawablePolicy(drawablePolicy), _pager(pager)
    {
        if (osgDB::Registry::instance()->getBuildKdTreesHint()==osgDB::ReaderWriter::Options::BUILD_KDTREES &&
            osgDB::Registry::instance()->getKdTreeBuilder())
        {
            _kdTreeBuilder = osgDB::Registry::instance()->getKdTreeBuilder()->clone();
        }
    }

    META_NodeVisitor("osgDB","FindCompileableGLObjectsVisitor")

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

        if (_kdTreeBuilder.valid())
        {
            geode.accept(*_kdTreeBuilder);
        }
    }

    inline void apply(osg::StateSet* stateset)
    {
        if (stateset)
        {
            // search for the existance of any texture object
            // attributes
            // if texture object attributes exist and need to be
            // compiled, add the state to the list for later
            // compilation.
            bool compileStateSet = false;
            for(unsigned int i=0;i<stateset->getTextureAttributeList().size();++i)
            {
                osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset->getTextureAttribute(i,osg::StateAttribute::TEXTURE));
                // Has this texture already been encountered?
                if (texture && !_textureSet.count(texture))
                {
                    _textureSet.insert(texture);
                    if (_changeAutoUnRef) texture->setUnRefImageDataAfterApply(_valueAutoUnRef);
                    if ((_changeAnisotropy
                         && texture->getMaxAnisotropy() != _valueAnisotropy))
                    {
                        if (_changeAnisotropy)
                            texture->setMaxAnisotropy(_valueAnisotropy);
                    }

                    if (!_pager->isCompiled(texture))
                    {
                        compileStateSet = true;
                        if (osg::getNotifyLevel() >= osg::DEBUG_INFO)
                        {
                            osg::notify(osg::DEBUG_INFO)
                                <<"Found compilable texture " << texture << " ";
                            osg::Image* image = texture->getImage(0);
                            if (image) osg::notify(osg::DEBUG_INFO) << image->getFileName();
                            osg::notify(osg::DEBUG_INFO) << std:: endl;
                        }
                        break;
                    }
                }
            }
            if (compileStateSet && _dataToCompile)
            {
                _dataToCompile->first.insert(stateset);
            }

        }
    }

    inline void apply(osg::Drawable* drawable)
    {
        if (_drawableSet.count(drawable))
            return;

        _drawableSet.insert(drawable);

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
        // Don't compile if already compiled. This can happen if the
        // subgraph is shared with already-loaded nodes.
        //
        // XXX This "compiles" VBOs too, but compilation doesn't do
        // anything for VBOs, does it?
        if (_dataToCompile && drawable->getUseDisplayList() && !_pager->isCompiled(drawable))
        {
            // osg::notify(osg::NOTICE)<<"  Found compilable drawable"<<std::endl;
            _dataToCompile->second.push_back(drawable);
        }
    }
    
    DatabasePager::DataToCompile*           _dataToCompile;
    bool                                    _changeAutoUnRef;
    bool                                    _valueAutoUnRef;
    bool                                    _changeAnisotropy;
    float                                   _valueAnisotropy;
    DatabasePager::DrawablePolicy           _drawablePolicy;
    const DatabasePager*                    _pager;
    std::set<osg::ref_ptr<osg::Texture> >   _textureSet;
    std::set<osg::ref_ptr<osg::Drawable> >  _drawableSet;
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
//  ReadQueue
//
void DatabasePager::RequestQueue::sort()
{
    std::sort(_requestList.begin(),_requestList.end(),SortFileRequestFunctor());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ReadQueue
//
DatabasePager::ReadQueue::ReadQueue(DatabasePager* pager, const std::string& name):
    _pager(pager),
    _name(name)
{
    _block = new osg::RefBlock;
}

void DatabasePager::ReadQueue::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    for(RequestList::iterator citr = _requestList.begin();
        citr != _requestList.end();
        ++citr)
    {
        (*citr)->_loadedModel = 0;
        (*citr)->_requestQueue = 0;
    }

    _requestList.clear();

    updateBlock();
}

void DatabasePager::ReadQueue::add(DatabasePager::DatabaseRequest* databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    _requestList.push_back(databaseRequest);
    databaseRequest->_requestQueue = this;

    updateBlock();
}

void DatabasePager::ReadQueue::takeFirst(osg::ref_ptr<DatabaseRequest>& databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    if (!_requestList.empty())
    {
        sort();
    
        databaseRequest = _requestList.front();
        databaseRequest->_requestQueue = 0;
        _requestList.erase(_requestList.begin());

        updateBlock();
    }
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
    
        _done = true;
        
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

        // release the frameBlock and _databasePagerThreadBlock in case its holding up thread cancellation.
        // _databasePagerThreadBlock->release();

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            // commenting out debug info as it was cashing crash on exit, presumable
            // due to osg::notify or std::cout destructing earlier than this destructor.
            // osg::notify(osg::DEBUG_INFO)<<"Waiting for DatabasePager to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
        
        // _startThreadCalled = false;
    }
    //std::cout<<"DatabasePager::~DatabasePager() stopped running"<<std::endl;
    return result;

}

void DatabasePager::DatabaseThread::run()
{
    osg::notify(osg::INFO)<<_name<<": DatabasePager::DatabaseThread::run"<<std::endl;
    
#if 1
    // need to set the texture object manager to be able to reuse textures
    osg::Texture::setMinimumNumberOfTextureObjectsToRetainInCache(100);
    
    // need to set the display list manager to be able to reuse display lists
    osg::Drawable::setMinimumNumberOfDisplayListsToRetainInCache(100);
#else
    // need to set the texture object manager to be able to reuse textures
    osg::Texture::setMinimumNumberOfTextureObjectsToRetainInCache(0);
    
    // need to set the display list manager to be able to reuse display lists
    osg::Drawable::setMinimumNumberOfDisplayListsToRetainInCache(0);
#endif

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
    
    //Getting CURL Environment Variables (If found rewrite OSG Options)
    osg::ref_ptr<FileCache> fileCache = osgDB::Registry::instance()->getFileCache();

    do
    {
        _active = false;

        read_queue->block();

        _active = true;

        osg::notify(osg::INFO)<<_name<<": _pager->_requestList.size()= "<<read_queue->_requestList.size()<<" to delete = "<<read_queue->_childrenToDeleteList.size()<<std::endl;


        //
        // delete any children if required.
        //
        if (_pager->_deleteRemovedSubgraphsInDatabaseThread && !(read_queue->_childrenToDeleteList.empty()))
        {
            ObjectList deleteList;

            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(read_queue->_childrenToDeleteListMutex);
                deleteList.swap(read_queue->_childrenToDeleteList);
                read_queue->updateBlock();
            }
        }

        //
        // load any subgraphs that are required.
        //
        osg::ref_ptr<DatabaseRequest> databaseRequest;
        read_queue->takeFirst(databaseRequest);

        bool readFromFileCache = false;
                
        if (databaseRequest.valid())
        {
            // check if databaseRequest is still relevant
            if ((_pager->_frameNumber-databaseRequest->_frameNumberLastRequest)<=1)
            {

                // now check to see if this request is appropriate for this thread
                switch(_mode)
                {
                    case(HANDLE_ALL_REQUESTS):
                        // do nothing as this thread can handle the load
                        if (osgDB::containsServerAddress(databaseRequest->_fileName))
                        {
                            if (fileCache.valid() && fileCache->existsInCache(databaseRequest->_fileName))
                            {
                                readFromFileCache = true;
                            }
                        }
                        break;
                        
                    case(HANDLE_NON_HTTP):
                        // check the cache first
                        if (osgDB::containsServerAddress(databaseRequest->_fileName))
                        {
                            if (fileCache.valid() && fileCache->existsInCache(databaseRequest->_fileName))
                            {
                                readFromFileCache = true;
                            }
                            else
                            {
                                osg::notify(osg::INFO)<<_name<<": Passing http requests over "<<databaseRequest->_fileName<<std::endl;
                                out_queue->add(databaseRequest.get());
                                databaseRequest = 0;
                            }
                        }
                        break;
                        
                    case(HANDLE_ONLY_HTTP):
                        // make sure the request is a http request
                        if (!osgDB::containsServerAddress(databaseRequest->_fileName))
                        {
                            osg::notify(osg::NOTICE)<<_name<<": Help we have request we shouldn't have "<<databaseRequest->_fileName<<std::endl;
                            databaseRequest = 0;
                        }
                        break;
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
            //osg::notify(osg::NOTICE)<<"In DatabasePager thread readNodeFile("<<databaseRequest->_fileName<<")"<<std::endl;
            //osg::Timer_t before = osg::Timer::instance()->tick();


            // assume that readNode is thread safe...
            ReaderWriter::ReadResult rr = readFromFileCache ?
                        fileCache->readNode(databaseRequest->_fileName, databaseRequest->_loadOptions.get(), false) :
                        Registry::instance()->readNode(databaseRequest->_fileName, databaseRequest->_loadOptions.get(), false);

            if (rr.validNode()) databaseRequest->_loadedModel = rr.getNode();
            if (rr.error()) osg::notify(osg::WARN)<<"Error in reading file "<<databaseRequest->_fileName<<" : "<<rr.message() << std::endl;

            if (databaseRequest->_loadedModel.valid() &&
                osgDB::containsServerAddress(databaseRequest->_fileName) &&
                fileCache.valid() &&
                !readFromFileCache)
            {
                fileCache->writeNode(*(databaseRequest->_loadedModel), databaseRequest->_fileName, databaseRequest->_loadOptions.get());
            }

            if ((_pager->_frameNumber-databaseRequest->_frameNumberLastRequest)>1)
            {
                osg::notify(osg::INFO)<<_name<<": Warning DatabaseRquest no longer required."<<std::endl;
                databaseRequest->_loadedModel = 0;
            }
            
            osg::ref_ptr<osg::Group> groupForAddingLoadedSubgraph = databaseRequest->_groupForAddingLoadedSubgraph.get();

            if (!groupForAddingLoadedSubgraph)
            {
                osg::notify(osg::INFO)<<_name<<": Warning parent of loaded subgraph, deleted."<<std::endl;
                databaseRequest->_loadedModel = 0;
            }

            //osg::notify(osg::NOTICE)<<"     node read in "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms"<<std::endl;

            bool loadedObjectsNeedToBeCompiled = false;

            if (databaseRequest->_loadedModel.valid())
            {            
                databaseRequest->_loadedModel->getBound();

                osg::NodePath nodePath;
                osg::NodePathList nodePathList = groupForAddingLoadedSubgraph->getParentalNodePaths();
                if (!nodePathList.empty()) nodePath = nodePathList.front();
                nodePath.push_back(groupForAddingLoadedSubgraph.get());

#if 1
                // force a compute of the loaded model's bounding volume, so that when the subgraph
                // merged with the main scene graph and large computeBound() isn't incurred.
                ActiveGraphicsContexts::iterator itr = _pager->_activeGraphicsContexts.begin();

                DataToCompile* dtc = 0;
                if (itr != _pager->_activeGraphicsContexts.end())
                {
                    dtc = &(databaseRequest->_dataToCompileMap[*itr]);
                    ++itr;
                }

                // find all the compileable rendering objects
                DatabasePager::FindCompileableGLObjectsVisitor frov(dtc, 
                                                     _pager->_changeAutoUnRef, _pager->_valueAutoUnRef,
                                                     _pager->_changeAnisotropy, _pager->_valueAnisotropy,
                                                     _pager->_drawablePolicy, 
                                                     _pager);

                // push the soon to be parent on the nodepath of the NodeVisitor so that 
                // during traversal one can test for where it'll be in the overall scene graph                
                for(osg::NodePath::iterator nitr = nodePath.begin();
                    nitr != nodePath.end();
                    ++nitr)
                {
                    frov.pushOntoNodePath(*nitr);
                }

                databaseRequest->_loadedModel->accept(frov);

                if (_pager->_doPreCompile &&
                    !_pager->_activeGraphicsContexts.empty())
                {
                    if (!dtc->first.empty() || !dtc->second.empty())
                    {
                        loadedObjectsNeedToBeCompiled = true;                

                        // copy the objects from the compile list to the other graphics context list.
                        for(;
                            itr != _pager->_activeGraphicsContexts.end();
                            ++itr)
                        {
                            databaseRequest->_dataToCompileMap[*itr] = *dtc;
                        }
                    }
                }
#else
                if (_pager->_doPreCompile &&
                    !_pager->_activeGraphicsContexts.empty())
                {
                    // force a compute of the loaded model's bounding volume, so that when the subgraph
                    // merged with the main scene graph and large computeBound() isn't incurred.
                    ActiveGraphicsContexts::iterator itr = _pager->_activeGraphicsContexts.begin();

                    DataToCompile& dtc = databaseRequest->_dataToCompileMap[*itr];
                    ++itr;                

                    // find all the compileable rendering objects
                    DatabasePager::FindCompileableGLObjectsVisitor frov(&dtc,
                                                         _pager->_changeAutoUnRef, _pager->_valueAutoUnRef,
                                                         _pager->_changeAnisotropy, _pager->_valueAnisotropy,
                                                         _pager->_drawablePolicy, 
                                                         _pager);

                    // push the soon to be parent on the nodepath of the NodeVisitor so that 
                    // during traversal one can test for where it'll be in the overall scene graph                
                    for(osg::NodePath::iterator nitr = nodePath.begin();
                        nitr != nodePath.end();
                        ++nitr)
                    {
                        frov.pushOntoNodePath(*nitr);
                    }

                    databaseRequest->_loadedModel->accept(frov);

                    if (!dtc.first.empty() || !dtc.second.empty())
                    {
                        loadedObjectsNeedToBeCompiled = true;                

                        // copy the objects from the compile list to the other graphics context list.
                        for(;
                            itr != _pager->_activeGraphicsContexts.end();
                            ++itr)
                        {
                            databaseRequest->_dataToCompileMap[*itr] = dtc;
                        }
                    }
                }
                else
                {
                    // check to see if we need to run the KdTreeBuilder
                    if (osgDB::Registry::instance()->getBuildKdTreesHint()==osgDB::ReaderWriter::Options::BUILD_KDTREES &&
                        osgDB::Registry::instance()->getKdTreeBuilder())
                    {
                        //osg::Timer_t before = osg::Timer::instance()->tick();
                        //osg::notify(osg::NOTICE)<<"osgTerrain::GeometryTechnique::build kd tree"<<std::endl;
                        osg::ref_ptr<osg::KdTreeBuilder> builder = osgDB::Registry::instance()->getKdTreeBuilder()->clone();

                        for(osg::NodePath::iterator nitr = nodePath.begin();
                            nitr != nodePath.end();
                            ++nitr)
                        {
                            builder->pushOntoNodePath(*nitr);
                        }

                        databaseRequest->_loadedModel->accept(*builder);
                        //osg::Timer_t after = osg::Timer::instance()->tick();
                        //osg::notify(osg::NOTICE)<<"KdTree build time "<<osg::Timer::instance()->delta_m(before, after)<<std::endl;
                    }
                }
#endif

                // move the databaseRequest from the front of the fileRequest to the end of
                // dataToCompile or dataToMerge lists.
                if (loadedObjectsNeedToBeCompiled)
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_pager->_dataToCompileList->_requestMutex);
                    databaseRequest->_requestQueue = _pager->_dataToCompileList.get();
                    _pager->_dataToCompileList->_requestList.push_back(databaseRequest);
                }
                else
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_pager->_dataToMergeList->_requestMutex);
                    databaseRequest->_requestQueue = _pager->_dataToMergeList.get();
                    _pager->_dataToMergeList->_requestList.push_back(databaseRequest);
                }
            }        

            // Prepare and prune the to-be-compiled list here in
            // the pager thread rather than in the draw or
            // graphics context thread(s).
            if (loadedObjectsNeedToBeCompiled)
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_pager->_dataToCompileList->_requestMutex);

                _pager->_dataToCompileList->sort();

                // Prune all the old entries.
                RequestQueue::RequestList::iterator tooOld
                    = std::find_if(_pager->_dataToCompileList->_requestList.begin(),
                                   _pager->_dataToCompileList->_requestList.end(),
                                   refPtrAdapt(std::not1(std::bind2nd(std::mem_fun(&DatabaseRequest::isRequestCurrent),
                                   _pager->_frameNumber))));

                // This is the database thread, so just delete
                for(RequestQueue::RequestList::iterator citr = tooOld;
                    citr != _pager->_dataToCompileList->_requestList.end();
                    ++citr)
                {
                    osg::notify(osg::INFO)<<_name<<": pruning from compile list"<<std::endl;
                    (*citr)->_loadedModel = 0;
                    (*citr)->_requestQueue = 0;
                }

                _pager->_dataToCompileList->_requestList.erase(tooOld, _pager->_dataToCompileList->_requestList.end());

                loadedObjectsNeedToBeCompiled = !_pager->_dataToCompileList->_requestList.empty();
            }

            if (loadedObjectsNeedToBeCompiled && !_pager->_activeGraphicsContexts.empty())
            {
                for(ActiveGraphicsContexts::iterator itr = _pager->_activeGraphicsContexts.begin();
                    itr != _pager->_activeGraphicsContexts.end();
                    ++itr)
                {
                    osg::GraphicsContext* gc = osg::GraphicsContext::getCompileContext(*itr);
                    if (gc)
                    {   
                        osg::GraphicsThread* gt = gc->getGraphicsThread();
                        if (gt)
                        {
                            gt->add(new DatabasePager::CompileOperation(_pager));
                        }
                        else
                        {
                            gc->makeCurrent();

                            _pager->compileAllGLObjects(*(gc->getState()));

                            gc->releaseContext();
                        }
                    }
                }

                // osg::notify(osg::NOTICE)<<"Done compiling in paging thread"<<std::endl;                   
            }
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
    //osg::notify(osg::INFO)<<"Constructing DatabasePager()"<<std::endl;
    
    _startThreadCalled = false;

    _done = false;
    _acceptNewRequests = true;
    _databasePagerThreadPaused = false;
    
    _numFramesActive = 0;
    _frameNumber = 0;
    
    const char* str = getenv("OSG_DATABASE_PAGER_PRIORITY");
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

#if __APPLE__
    // OSX really doesn't like compiling display lists, and performs poorly when they are used,
    // so apply this hack to make up for its short comings.
    _drawablePolicy = USE_VERTEX_ARRAYS;
#else
    _drawablePolicy = DO_NOT_MODIFY_DRAWABLE_SETTINGS;
#endif    
    
    str = getenv("OSG_DATABASE_PAGER_GEOMETRY");
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

    _changeAnisotropy = false;
    _valueAnisotropy = 1.0f;


    
    const char* ptr=0;

    _deleteRemovedSubgraphsInDatabaseThread = true;
    if( (ptr = getenv("OSG_DELETE_IN_DATABASE_THREAD")) != 0)
    {
        _deleteRemovedSubgraphsInDatabaseThread = strcmp(ptr,"yes")==0 || strcmp(ptr,"YES")==0 ||
                        strcmp(ptr,"on")==0 || strcmp(ptr,"ON")==0;

    }

    _expiryDelay = 10.0;
    if( (ptr = getenv("OSG_EXPIRY_DELAY")) != 0)
    {
        _expiryDelay = osg::asciiToDouble(ptr);
        osg::notify(osg::NOTICE)<<"DatabasePager: Expiry delay = "<<_expiryDelay<<std::endl;
    }

    _expiryFrames = 1; // Last frame will not be expired
    if( (ptr = getenv("OSG_EXPIRY_FRAMES")) != 0)
    {
        _expiryFrames = atoi(ptr);
        osg::notify(osg::NOTICE)<<"DatabasePager: Expiry frames = "<<_expiryFrames<<std::endl;
    }

    if( (ptr = getenv("OSG_RELEASE_DELAY")) != 0)
    {
        if (strcmp(ptr,"OFF")==0 || strcmp(ptr,"Off")==0  || strcmp(ptr,"off")==0)
        {
            setReleaseDelay(DBL_MAX);
        }
        else
        {
            setReleaseDelay(osg::asciiToDouble(ptr));
        }
            
        osg::notify(osg::NOTICE)<<"DatabasePager: Release delay = "<<_releaseDelay<<std::endl;
    }
    else
    {
        setReleaseDelay(DBL_MAX);
    }
    

    _releaseFrames = 1; // Last frame will not be release
    if( (ptr = getenv("OSG_RELEASE_FRAMES")) != 0)
    {
        _releaseFrames = atoi(ptr);
        osg::notify(osg::NOTICE)<<"Release frames = "<<_releaseFrames<<std::endl;
    }


    _targetMaximumNumberOfPageLOD = 300;
    if( (ptr = getenv("OSG_MAX_PAGEDLOD")) != 0)
    {
        _targetMaximumNumberOfPageLOD = atoi(ptr);
        osg::notify(osg::NOTICE)<<"_targetMaximumNumberOfPageLOD = "<<_targetMaximumNumberOfPageLOD<<std::endl;
    }


    _doPreCompile = false;
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
        _minimumTimeAvailableForGLCompileAndDeletePerFrame = osg::asciiToDouble(ptr);
    }

    if( (ptr = getenv("OSG_MAXIMUM_OBJECTS_TO_COMPILE_PER_FRAME")) != 0)
    {
        _maximumNumOfObjectsToCompilePerFrame = atoi(ptr);
    }

    // initialize the stats variables
    resetStats();

    // make sure a SharedStateManager exists.
    //osgDB::Registry::instance()->getOrCreateSharedStateManager();
    
    //if (osgDB::Registry::instance()->getSharedStateManager())
        //osgDB::Registry::instance()->setUseObjectCacheHint(true);
        
    _fileRequestQueue = new ReadQueue(this,"fileRequestQueue");
    _httpRequestQueue = new ReadQueue(this,"httpRequestQueue");
    
    _dataToCompileList = new RequestQueue;
    _dataToMergeList = new RequestQueue;
    
    setUpThreads(
        osg::DisplaySettings::instance()->getNumOfDatabaseThreadsHint(),
        osg::DisplaySettings::instance()->getNumOfHttpDatabaseThreadsHint());

}

DatabasePager::DatabasePager(const DatabasePager& rhs)
{
    //osg::notify(osg::INFO)<<"Constructing DatabasePager(const DatabasePager& )"<<std::endl;
    
    _startThreadCalled = false;

    _done = false;
    _acceptNewRequests = true;
    _databasePagerThreadPaused = false;
    
    _numFramesActive = 0;
    _frameNumber = 0;

    _drawablePolicy = rhs._drawablePolicy;

    _changeAutoUnRef = rhs._changeAutoUnRef;
    _valueAutoUnRef = rhs._valueAutoUnRef;
    _changeAnisotropy = rhs._changeAnisotropy;
    _valueAnisotropy = rhs._valueAnisotropy;


    _deleteRemovedSubgraphsInDatabaseThread = rhs._deleteRemovedSubgraphsInDatabaseThread;
    
    _expiryDelay = rhs._expiryDelay;
    _expiryFrames = rhs._expiryFrames;

    _releaseDelay = rhs._releaseDelay;
    _releaseFrames = rhs._releaseFrames;

    _targetMaximumNumberOfPageLOD = rhs._targetMaximumNumberOfPageLOD;

    _doPreCompile = rhs._doPreCompile;
    _targetFrameRate = rhs._targetFrameRate;
    _minimumTimeAvailableForGLCompileAndDeletePerFrame = rhs._minimumTimeAvailableForGLCompileAndDeletePerFrame;
    _maximumNumOfObjectsToCompilePerFrame = rhs._maximumNumOfObjectsToCompilePerFrame;

    _fileRequestQueue = new ReadQueue(this,"fileRequestQueue");
    _httpRequestQueue = new ReadQueue(this,"httpRequestQueue");
    
    _dataToCompileList = new RequestQueue;
    _dataToMergeList = new RequestQueue;

    for(DatabaseThreadList::const_iterator dt_itr = rhs._databaseThreads.begin();
        dt_itr != rhs._databaseThreads.end();
        ++dt_itr)
    {
        _databaseThreads.push_back(new DatabaseThread(**dt_itr,this));
    }

    // initialize the stats variables
    resetStats();
}


DatabasePager::~DatabasePager()
{
    cancel();
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
    osg::notify(osg::INFO)<<"DatabasePager::addDatabaseThread() "<<name<<std::endl;

    unsigned int pos = _databaseThreads.size();
    
    DatabaseThread* thread = new DatabaseThread(this, mode,name);
    _databaseThreads.push_back(thread);
    
    if (_startThreadCalled)
    {
        osg::notify(osg::DEBUG_INFO)<<"DatabasePager::startThread()"<<std::endl;
        thread->startThread();
    }
    
    return pos;
}

void DatabasePager::setReleaseDelay(double releaseDelay)
{
    _releaseDelay = releaseDelay;

    if (_releaseDelay==DBL_MAX)
    {
        _changeAutoUnRef = true;
        _valueAutoUnRef = true;
    }
    else
    {
        // when GLObject release is used make sure Images aren't unref'd as they may be needed later.
        _changeAutoUnRef = true;
        _valueAutoUnRef = false;
    }
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

    // release the frameBlock and _databasePagerThreadBlock in case its holding up thread cancellation.
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

    //std::cout<<"DatabasePager::~DatabasePager() stopped running"<<std::endl;
    return result;
}

void DatabasePager::clear()
{
    _fileRequestQueue->clear();
    _httpRequestQueue->clear();
        
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileList->_requestMutex);
        for(RequestQueue::RequestList::iterator citr = _dataToCompileList->_requestList.begin();
            citr != _dataToCompileList->_requestList.end();
            ++citr)
        {
            (*citr)->_loadedModel = 0;
            (*citr)->_requestQueue = 0;
        }
        _dataToCompileList->_requestList.clear();
    }

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeList->_requestMutex);
        for(RequestQueue::RequestList::iterator citr = _dataToMergeList->_requestList.begin();
            citr != _dataToMergeList->_requestList.end();
            ++citr)
        {
            (*citr)->_loadedModel = 0;
            (*citr)->_requestQueue = 0;
        }
        _dataToMergeList->_requestList.clear();
    }

    // note, no need to use a mutex as the list is only accessed from the update thread.
    _activePagedLODList.clear();
    _inactivePagedLODList.clear();

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


void DatabasePager::requestNodeFile(const std::string& fileName,osg::Group* group,
                                    float priority, const osg::FrameStamp* framestamp,
                                    osg::ref_ptr<osg::Referenced>& databaseRequest)
{
    requestNodeFile(fileName,group,priority,framestamp,databaseRequest,Registry::instance()->getOptions());
}

void DatabasePager::requestNodeFile(const std::string& fileName,osg::Group* group,
                                    float priority, const osg::FrameStamp* framestamp,
                                    osg::ref_ptr<osg::Referenced>& databaseRequestRef,
                                    ReaderWriter::Options* loadOptions)
{
    if (!_acceptNewRequests) return;
    
    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    double timestamp = framestamp?framestamp->getReferenceTime():0.0;
    int frameNumber = framestamp?framestamp->getFrameNumber():_frameNumber;
    
    static int previousFrame = -1;
    static double totalTime = 0.0;
    

    if (previousFrame!=frameNumber)
    {
        // osg::notify(osg::NOTICE)<<"requestNodeFiles for "<<previousFrame<<" time = "<<totalTime<<std::endl;

        previousFrame = frameNumber;
        totalTime = 0.0;
    }

    
    // search to see if filename already exist in the file loaded list.
    bool foundEntry = false;

    if (databaseRequestRef.valid())
    {
        DatabaseRequest* databaseRequest = dynamic_cast<DatabaseRequest*>(databaseRequestRef.get());
        if (databaseRequest)
        {
            osg::notify(osg::INFO)<<"DatabasePager::fileRequest("<<fileName<<") updating already assigned."<<std::endl;

            RequestQueue* requestQueue = databaseRequest->_requestQueue;
            if (requestQueue)
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(requestQueue->_requestMutex);

                databaseRequest->_frameNumberLastRequest = frameNumber;
                databaseRequest->_timestampLastRequest = timestamp;
                databaseRequest->_priorityLastRequest = priority;
                ++(databaseRequest->_numOfRequests);
            }
            else
            {
                databaseRequest->_frameNumberLastRequest = frameNumber;
                databaseRequest->_timestampLastRequest = timestamp;
                databaseRequest->_priorityLastRequest = priority;
                ++(databaseRequest->_numOfRequests);
            }
            
            foundEntry = true;

            if (databaseRequestRef->referenceCount()==1)
            {
                osg::notify(osg::INFO)<<"DatabasePager::fileRquest("<<fileName<<") orphaned, resubmitting."<<std::endl;

                databaseRequest->_frameNumberFirstRequest = frameNumber;
                databaseRequest->_timestampFirstRequest = timestamp;
                databaseRequest->_priorityFirstRequest = priority;
                databaseRequest->_frameNumberLastRequest = frameNumber;
                databaseRequest->_timestampLastRequest = timestamp;
                databaseRequest->_priorityLastRequest = priority;
                databaseRequest->_groupForAddingLoadedSubgraph = group;
                databaseRequest->_loadOptions = loadOptions;
                databaseRequest->_requestQueue = _fileRequestQueue.get();

                _fileRequestQueue->add(databaseRequest);
            }
            
        }
    }

    if (!foundEntry)
    {
        osg::notify(osg::INFO)<<"In DatabasePager::fileRquest("<<fileName<<")"<<std::endl;
        
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_requestMutex);
        
        if (!databaseRequestRef.valid() || databaseRequestRef->referenceCount()==1)
        {
            osg::ref_ptr<DatabaseRequest> databaseRequest = new DatabaseRequest;

            databaseRequestRef = databaseRequest.get();

            databaseRequest->_fileName = fileName;
            databaseRequest->_frameNumberFirstRequest = frameNumber;
            databaseRequest->_timestampFirstRequest = timestamp;
            databaseRequest->_priorityFirstRequest = priority;
            databaseRequest->_frameNumberLastRequest = frameNumber;
            databaseRequest->_timestampLastRequest = timestamp;
            databaseRequest->_priorityLastRequest = priority;
            databaseRequest->_groupForAddingLoadedSubgraph = group;
            databaseRequest->_loadOptions = loadOptions;
            databaseRequest->_requestQueue = _fileRequestQueue.get();

            _fileRequestQueue->_requestList.push_back(databaseRequest.get());

            _fileRequestQueue->updateBlock();
        }
        
    }
    
    if (!_startThreadCalled)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_run_mutex);
        
        if (!_startThreadCalled)
        {
            _startThreadCalled = true;
            _done = false;
            osg::notify(osg::DEBUG_INFO)<<"DatabasePager::startThread()"<<std::endl;
            
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

    totalTime += osg::Timer::instance()->delta_m(start_tick, osg::Timer::instance()->tick());
}

void DatabasePager::signalBeginFrame(const osg::FrameStamp* framestamp)
{
    if (framestamp)
    {
        //osg::notify(osg::INFO) << "signalBeginFrame "<<framestamp->getFrameNumber()<<">>>>>>>>>>>>>>>>"<<std::endl;
        _frameNumber = framestamp->getFrameNumber();
        
    } //else osg::notify(osg::INFO) << "signalBeginFrame >>>>>>>>>>>>>>>>"<<std::endl;
}

void DatabasePager::signalEndFrame()
{
    //osg::notify(osg::INFO) << "signalEndFrame <<<<<<<<<<<<<<<<<<<< "<<std::endl;
}

void DatabasePager::setDatabasePagerThreadPause(bool pause)
{
    if (_databasePagerThreadPaused == pause) return;
    
    _databasePagerThreadPaused = pause;
    _fileRequestQueue->updateBlock();
    _httpRequestQueue->updateBlock();
}


bool DatabasePager::requiresUpdateSceneGraph() const
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeList->_requestMutex);
        if (!_dataToMergeList->_requestList.empty()) return true;
    }
    
    return false;
    
}
 

void DatabasePager::addLoadedDataToSceneGraph(const osg::FrameStamp &frameStamp)
{
    double timeStamp = frameStamp.getReferenceTime();
    int frameNumber = frameStamp.getFrameNumber();

    osg::Timer_t before = osg::Timer::instance()->tick();

    RequestQueue::RequestList localFileLoadedList;

    // get the data for the _dataToCompileList, leaving it empty via a std::vector<>.swap.
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeList->_requestMutex);
        localFileLoadedList.swap(_dataToMergeList->_requestList);
    }
        
    osg::Timer_t mid = osg::Timer::instance()->tick();

    // add the loaded data into the scene graph.
    for(RequestQueue::RequestList::iterator itr=localFileLoadedList.begin();
        itr!=localFileLoadedList.end();
        ++itr)
    {
        DatabaseRequest* databaseRequest = itr->get();

        // osg::notify(osg::NOTICE)<<"Merging "<<_frameNumber-(*itr)->_frameNumberLastRequest<<std::endl;
        
        if (osgDB::Registry::instance()->getSharedStateManager()) 
            osgDB::Registry::instance()->getSharedStateManager()->share(databaseRequest->_loadedModel.get());

        
        registerPagedLODs(databaseRequest->_loadedModel.get(), frameStamp.getFrameNumber());
        
        osg::ref_ptr<osg::Group> group = databaseRequest->_groupForAddingLoadedSubgraph.get();
        if (group.valid())
        {
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

            // osg::notify(osg::NOTICE)<<"merged subgraph"<<databaseRequest->_fileName<<" after "<<databaseRequest->_numOfRequests<<" requests and time="<<(timeStamp-databaseRequest->_timestampFirstRequest)*1000.0<<std::endl;

            double timeToMerge = timeStamp-databaseRequest->_timestampFirstRequest;

            if (timeToMerge<_minimumTimeToMergeTile) _minimumTimeToMergeTile = timeToMerge;
            if (timeToMerge>_maximumTimeToMergeTile) _maximumTimeToMergeTile = timeToMerge;

            _totalTimeToMergeTiles += timeToMerge;
            ++_numTilesMerges;
        }
                
        // reset the loadedModel pointer
        databaseRequest->_loadedModel = 0;

        // osg::notify(osg::NOTICE)<<"curr = "<<timeToMerge<<" min "<<getMinimumTimeToMergeTile()*1000.0<<" max = "<<getMaximumTimeToMergeTile()*1000.0<<" average = "<<getAverageTimToMergeTiles()*1000.0<<std::endl;
    }

    osg::Timer_t last = osg::Timer::instance()->tick();

    osg::notify(osg::DEBUG_INFO)<<"Done DatabasePager::addLoadedDataToSceneGraph"<<
        osg::Timer::instance()->delta_m(before,mid)<<"ms,\t"<<
        osg::Timer::instance()->delta_m(mid,last)<<"ms"<<
        "  objects"<<localFileLoadedList.size()<<std::endl<<std::endl;
    
}

class DatabasePager::MarkPagedLODsVisitor : public osg::NodeVisitor
{
public:
    MarkPagedLODsVisitor(const std::string& marker):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _marker(marker),
        _numPagedLODsMarked(0)
    {
    }

    META_NodeVisitor("osgDB","MarkPagedLODsVisitor")

    virtual void apply(osg::PagedLOD& plod)
    {
        if (plod.getName()!=_marker)
        {
            ++_numPagedLODsMarked;
            plod.setName(_marker);
    
            traverse(plod);
        }
    }
    
    std::string _marker;
    int _numPagedLODsMarked;
};

void DatabasePager::removeExpiredSubgraphs(const osg::FrameStamp& frameStamp)
{
    if (_targetMaximumNumberOfPageLOD>0)
    {
        capped_removeExpiredSubgraphs(frameStamp);
    }
    else
    {
        expiry_removeExpiredSubgraphs(frameStamp);
    }
}

void DatabasePager::capped_removeExpiredSubgraphs(const osg::FrameStamp& frameStamp)
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

    osg::Timer_t startTick = osg::Timer::instance()->tick();


    unsigned int numPagedLODs = _activePagedLODList.size() + _inactivePagedLODList.size();
    

    PagedLODList::iterator itr = _activePagedLODList.begin();
    for(PagedLODList::iterator itr = _activePagedLODList.begin();
        itr != _activePagedLODList.end();
        )
    {
        osg::PagedLOD* plod = itr->get();
        
        int delta = frameStamp.getFrameNumber() - plod->getFrameNumberOfLastTraversal();
        if (delta>1)
        {
            if (_releaseDelay!=DBL_MAX)
            {
                plod->releaseGLObjects();
                osg::notify(osg::NOTICE)<<"DatabasePager::removeExpiredSubgraphs(), releasing gl objects"<<std::endl;
            }

            _inactivePagedLODList.push_back(plod);
            
            itr = _activePagedLODList.erase(itr);
        }
        else
        {
            ++itr;
        }
    }

    for(PagedLODList::iterator itr = _inactivePagedLODList.begin();
        itr != _inactivePagedLODList.end();
        )
    {
        osg::PagedLOD* plod = itr->get();
        
        int delta = frameStamp.getFrameNumber() - plod->getFrameNumberOfLastTraversal();
        if (delta>1)
        {
            ++itr;
        }
        else
        {
            _activePagedLODList.push_back(plod);
            
            itr = _inactivePagedLODList.erase(itr);
        }
    }

    int inactivePLOD = _inactivePagedLODList.size();

    
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
    if (numToPrune > inactivePLOD)
    {
        numToPrune = inactivePLOD;
    }


    osg::NodeList childrenRemoved;
    
    double expiryTime = frameStamp.getReferenceTime() - 0.1;
    int expiryFrame = frameStamp.getFrameNumber() - 1;

    MarkPagedLODsVisitor markerVistor("NeedToRemove");

    for(PagedLODList::iterator itr = _inactivePagedLODList.begin();
        itr!=_inactivePagedLODList.end() && markerVistor._numPagedLODsMarked<numToPrune;
        ++itr)
    {
        osg::PagedLOD* plod = itr->get();

        osg::NodeList localChildrenRemoved;
        plod->removeExpiredChildren(expiryTime, expiryFrame, localChildrenRemoved);
        if (!localChildrenRemoved.empty())
        {
            for(osg::NodeList::iterator critr = localChildrenRemoved.begin();
                critr!=localChildrenRemoved.end();
                ++critr)
            {
                (*critr)->accept(markerVistor);
            }
            
            std::copy(localChildrenRemoved.begin(),localChildrenRemoved.end(),std::back_inserter(childrenRemoved));
        }
    }
    
    for(PagedLODList::iterator itr = _activePagedLODList.begin();
        itr!=_activePagedLODList.end() && markerVistor._numPagedLODsMarked<numToPrune;
        ++itr)
    {
        osg::PagedLOD* plod = itr->get();

        osg::NodeList localChildrenRemoved;
        plod->removeExpiredChildren(expiryTime, expiryFrame, localChildrenRemoved);
        if (!localChildrenRemoved.empty())
        {
            for(osg::NodeList::iterator critr = localChildrenRemoved.begin();
                critr!=localChildrenRemoved.end();
                ++critr)
            {
                (*critr)->accept(markerVistor);
            }
            
            std::copy(localChildrenRemoved.begin(),localChildrenRemoved.end(),std::back_inserter(childrenRemoved));
        }
    }
    
    osg::Timer_t end_b_Tick = osg::Timer::instance()->tick();
    double time_b = osg::Timer::instance()->delta_m(end_a_Tick,end_b_Tick);

    s_total_iter_stage_b += 1.0;
    s_total_time_stage_b += time_b;
    if (s_total_max_stage_b<time_b) s_total_max_stage_b = time_b;



    //osg::notify(osg::NOTICE)<<"numToPrune "<<numToPrune<< " markerVistor._numPagedLODsMarked="<<markerVistor._numPagedLODsMarked<< " childrenRemoved.size()="<<childrenRemoved.size()<<std::endl;

    if (!childrenRemoved.empty())
    { 
        bool updateBlock = false;

        // pass the objects across to the database pager delete list
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_childrenToDeleteListMutex);
            for (osg::NodeList::iterator critr = childrenRemoved.begin();
                 critr!=childrenRemoved.end();
                 ++critr)
            {
                _fileRequestQueue->_childrenToDeleteList.push_back(critr->get());
            }
            
            updateBlock = true;

        }

        int numRemoved = 0;
        int numToRemove = markerVistor._numPagedLODsMarked;
        
        
        //osg::notify(osg::NOTICE)<<"Children to remove "<<childrenRemoved.size()<<" numToRemove="<<numToRemove<<std::endl;

        // osg::notify(osg::NOTICE)<<"   time 2 "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms "<<std::endl;
        int numSkipped = 0;
        for(PagedLODList::iterator itr = _inactivePagedLODList.begin();
            itr!=_inactivePagedLODList.end() && numRemoved<numToRemove;
            )
        {
            osg::PagedLOD* plod = itr->get();
            if (plod && plod->getName() != markerVistor._marker)
            {
                ++itr;
                
                ++numSkipped;
                
                // osg::notify(osg::NOTICE)<<"skipping"<<std::endl;
            }
            else
            {
                // osg::notify(osg::NOTICE)<<"removing"<<std::endl;

                ++numRemoved;

                itr = _inactivePagedLODList.erase(itr);
            }
        }

        osg::notify(osg::INFO)<<"Number of PagedLODs skipped="<<numSkipped<<" removed "<<numRemoved<<std::endl;


        childrenRemoved.clear();


        if (updateBlock)
        {
            _fileRequestQueue->updateBlock();
        }
    }
    
    osg::Timer_t end_c_Tick = osg::Timer::instance()->tick();
    double time_c = osg::Timer::instance()->delta_m(end_b_Tick,end_c_Tick);

    s_total_iter_stage_c += 1.0;
    s_total_time_stage_c += time_c;
    if (s_total_max_stage_c<time_c) s_total_max_stage_c = time_c;



    osg::notify(osg::INFO)<<"active="<<_activePagedLODList.size()<<" inactive="<<_inactivePagedLODList.size()<<" overall = "<<osg::Timer::instance()->delta_m(startTick,end_c_Tick)<<
                              " A="<<time_a<<" avg="<<s_total_time_stage_a/s_total_iter_stage_a<<" max = "<<s_total_max_stage_a<<
                              " B="<<time_b<<" avg="<<s_total_time_stage_b/s_total_iter_stage_b<<" max = "<<s_total_max_stage_b<<
                              " C="<<time_c<<" avg="<<s_total_time_stage_c/s_total_iter_stage_c<<" max = "<<s_total_max_stage_c<<std::endl;
}

void DatabasePager::expiry_removeExpiredSubgraphs(const osg::FrameStamp& frameStamp)
{
//    osg::notify(osg::NOTICE)<<"DatabasePager::new_removeExpiredSubgraphs()"<<std::endl;

    static double s_total_iter = 0.0;
    static double s_total_time = 0.0;
    static double s_total_max = 0.0;
    
    osg::Timer_t startTick = osg::Timer::instance()->tick();

    double expiryTime = frameStamp.getReferenceTime() - _expiryDelay;
    int expiryFrame = frameStamp.getFrameNumber() - _expiryFrames;

    double releaseTime = frameStamp.getReferenceTime() - _releaseDelay;
    int releaseFrame = frameStamp.getFrameNumber() - _releaseFrames;

    osg::NodeList childrenRemoved;
    
    for(PagedLODList::iterator itr = _activePagedLODList.begin();
        itr!=_activePagedLODList.end();
        ++itr)
    {
        osg::PagedLOD* plod = itr->get();
        
        
        if (_releaseDelay!=DBL_MAX && plod->releaseGLObjectsOnExpiredChildren(releaseTime, releaseFrame))
        {
            osg::notify(osg::INFO)<<"DatabasePager::removeExpiredSubgraphs(), releasing gl objects"<<std::endl;
        }
        
        plod->removeExpiredChildren(expiryTime, expiryFrame, childrenRemoved);
    }
    
    if (!childrenRemoved.empty())
    { 
        MarkPagedLODsVisitor markerVistor("NeedToRemove");
        for(osg::NodeList::iterator critr = childrenRemoved.begin();
            critr!=childrenRemoved.end();
            ++critr)
        {
            (*critr)->accept(markerVistor);
        }    
    
        // osg::notify(osg::NOTICE)<<"Children to remove "<<childrenRemoved.size()<<std::endl;
    
        // pass the objects across to the database pager delete list
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_childrenToDeleteListMutex);
            for (osg::NodeList::iterator critr = childrenRemoved.begin();
                 critr!=childrenRemoved.end();
                 ++critr)
            {
                _fileRequestQueue->_childrenToDeleteList.push_back(critr->get());
            }

            _fileRequestQueue->updateBlock();
        }

        // osg::notify(osg::NOTICE)<<"   time 2 "<<osg::Timer::instance()->delta_m(before,osg::Timer::instance()->tick())<<" ms "<<std::endl;
        for(PagedLODList::iterator itr = _activePagedLODList.begin();
            itr!=_activePagedLODList.end();
            )
        {
            osg::PagedLOD* plod = itr->get();
            if (plod && plod->getName() != markerVistor._marker)
            {
                ++itr;
            }
            else
            {
                PagedLODList::iterator itr_to_erase = itr;
                ++itr;

                _activePagedLODList.erase(itr_to_erase);            
            }
        }

        childrenRemoved.clear();


    }

    osg::Timer_t endTick = osg::Timer::instance()->tick();
    double time = osg::Timer::instance()->delta_m(startTick,endTick);

    s_total_iter += 1.0;
    s_total_time += time;
    if (s_total_max<time) s_total_max = time;

    osg::notify(osg::INFO)<<"_activePagedLODList.size()="<<_activePagedLODList.size()<<" overall = "<<time<<
                              " avg="<<s_total_time/s_total_iter<<" max = "<<s_total_max<<std::endl;
}

class DatabasePager::FindPagedLODsVisitor : public osg::NodeVisitor
{
public:

    FindPagedLODsVisitor(DatabasePager::PagedLODList& pagedLODList, int frameNumber):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _activePagedLODList(pagedLODList),
        _frameNumber(frameNumber)
    {
    }
    
    META_NodeVisitor("osgDB","FindPagedLODsVisitor")

    virtual void apply(osg::PagedLOD& plod)
    {
        plod.setFrameNumberOfLastTraversal(_frameNumber);
        
        _activePagedLODList.push_back(&plod);
    
        traverse(plod);
    }
    
    DatabasePager::PagedLODList& _activePagedLODList;
    int _frameNumber;
        
protected:
    
    FindPagedLODsVisitor& operator = (const FindPagedLODsVisitor&) { return *this; }
};

void DatabasePager::registerPagedLODs(osg::Node* subgraph, int frameNumber)
{
    if (!subgraph) return;
    
    FindPagedLODsVisitor fplv(_activePagedLODList, frameNumber);
    subgraph->accept(fplv);
}

bool DatabasePager::requiresCompileGLObjects() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileList->_requestMutex);
    return !_dataToCompileList->_requestList.empty();
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


DatabasePager::CompileOperation::CompileOperation(osgDB::DatabasePager* databasePager):
    osg::GraphicsOperation("DatabasePager::CompileOperation",false),
    _databasePager(databasePager)
{
}

void DatabasePager::CompileOperation::operator () (osg::GraphicsContext* context)
{
    // osg::notify(osg::NOTICE)<<"Background thread compiling"<<std::endl;

    if (_databasePager.valid()) _databasePager->compileAllGLObjects(*(context->getState()));
    
}

bool DatabasePager::requiresExternalCompileGLObjects(unsigned int contextID) const
{
    if (_activeGraphicsContexts.count(contextID)==0) return false;

    return osg::GraphicsContext::getCompileContext(contextID)==0;
}

void DatabasePager::compileAllGLObjects(osg::State& state)
{
    double availableTime = DBL_MAX;
    compileGLObjects(state, availableTime);
}

void DatabasePager::compileGLObjects(osg::State& state, double& availableTime)
{
    // osg::notify(osg::NOTICE)<<"DatabasePager::compileGLObjects "<<_frameNumber<<std::endl;

    bool compileAll = (availableTime==DBL_MAX);

    SharedStateManager *sharedManager
        = Registry::instance()->getSharedStateManager();
    osg::RenderInfo renderInfo;
    renderInfo.setState(&state);

    if (availableTime>0.0)
    {

        const osg::Timer& timer = *osg::Timer::instance();
        osg::Timer_t start_tick = timer.tick();
        double elapsedTime = 0.0;
        double estimatedTextureDuration = 0.0001;
        double estimatedDrawableDuration = 0.0001;

        osg::ref_ptr<DatabaseRequest> databaseRequest;

        // get the first compilable entry.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileList->_requestMutex);
        
            // advance to the next entry to compile if one is available.
            databaseRequest = _dataToCompileList->_requestList.empty() ? 0 : _dataToCompileList->_requestList.front().get();
        };

        unsigned int numObjectsCompiled = 0;

        // while there are valid databaseRequest's in the to compile list and there is
        // sufficient time left compile each databaseRequest's stateset and drawables.
        while (databaseRequest.valid() && (compileAll || (elapsedTime<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame)) ) 
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
                unsigned int objTemp = numObjectsCompiled;
                for(;
                    itr!=sslist.end() && (compileAll || ((elapsedTime+estimatedTextureDuration)<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame));
                    ++itr)
                {
                    //osg::notify(osg::INFO)<<"    Compiling stateset "<<(*itr).get()<<std::endl;
                    if (isCompiled(itr->get(), state.getContextID())
                        || (sharedManager && sharedManager->isShared(itr->get())))
                    {
                        elapsedTime = timer.delta_s(start_tick,timer.tick());
                        continue;
                    }
                    double startCompileTime = timer.delta_s(start_tick,timer.tick());

                    (*itr)->compileGLObjects(state);

                    GLint p;
                    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_RESIDENT, &p);

                    elapsedTime = timer.delta_s(start_tick,timer.tick());


                    // estimate the duration of the compile based on current compile duration.
                    estimatedTextureDuration = (elapsedTime-startCompileTime);

                    ++numObjectsCompiled;
                }
                if (osg::getNotifyLevel() >= osg::DEBUG_INFO
                    && numObjectsCompiled > objTemp)
                    osg::notify(osg::DEBUG_INFO)<< _frameNumber << " compiled "
                                                << numObjectsCompiled - objTemp
                                                << " StateSets" << std::endl;
                // remove the compiled statesets from the list.
                sslist.erase(sslist.begin(),itr);
            }

            if (!dtc.second.empty() && (compileAll || ((elapsedTime+estimatedDrawableDuration)<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame)))
            {
                // we have Drawable's to compile
                //osg::notify(osg::INFO)<<"Compiling drawables"<<std::endl;
                DrawableList& dwlist = dtc.second;
                DrawableList::iterator itr=dwlist.begin();
                unsigned int objTemp = numObjectsCompiled;
                for(;
                    itr!=dwlist.end() && (compileAll || ((elapsedTime+estimatedDrawableDuration)<availableTime && numObjectsCompiled<_maximumNumOfObjectsToCompilePerFrame));
                    ++itr)
                {
                    //osg::notify(osg::INFO)<<"    Compiling drawable "<<(*itr).get()<<std::endl;
                    if (isCompiled(itr->get(), state.getContextID()))
                    {
                        elapsedTime = timer.delta_s(start_tick,timer.tick());
                        continue;
                    }
                    double startCompileTime = timer.delta_s(start_tick,timer.tick());
                    (*itr)->compileGLObjects(renderInfo);
                    elapsedTime = timer.delta_s(start_tick,timer.tick());

                    // estimate the duration of the compile based on current compile duration.
                    estimatedDrawableDuration = (elapsedTime-startCompileTime);

                    ++numObjectsCompiled;

                }
                if (osg::getNotifyLevel() >= osg::DEBUG_INFO
                    && numObjectsCompiled > objTemp)
                    osg::notify(osg::DEBUG_INFO)<< _frameNumber << " compiled "
                                                << numObjectsCompiled - objTemp
                                                << " Drawables" << std::endl;
                // remove the compiled drawables from the list.
                dwlist.erase(dwlist.begin(),itr);
            }

            //osg::notify(osg::INFO)<<"Checking if compiled"<<std::endl;

            // now check the to compile entries for all active graphics contexts
            // to make sure that all have been compiled. They won't be
            // if we ran out of time or if another thread is still
            // compiling for its graphics context.
            bool allCompiled = true;
            for(DataToCompileMap::iterator itr=dcm.begin();
                itr!=dcm.end() && allCompiled;
                ++itr)
            {
                if (!(itr->second.first.empty())) allCompiled=false;
                if (!(itr->second.second.empty())) allCompiled=false;
            }

            //if (numObjectsCompiled > 0)
            //osg::notify(osg::NOTICE)<< _frameNumber << "compiled " << numObjectsCompiled << " objects" << std::endl;
            
            if (allCompiled)
            {
                // we've compiled all of the current databaseRequest so we can now pop it off the
                // to compile list and place it on the merge list.
                // osg::notify(osg::NOTICE)<<"All compiled"<<std::endl;


                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToCompileList->_requestMutex);

                // The request might have been removed from the
                // _dataToCompile list by another graphics thread, in
                // which case it's already on the merge list, or by
                // the pager, which means that the request has
                // expired. Also, the compile list might have been
                // shuffled by the pager, so the current request is
                // not guaranteed to still be at the beginning of the
                // list.
                
                RequestQueue::RequestList::iterator requestIter
                    = std::find(_dataToCompileList->_requestList.begin(), _dataToCompileList->_requestList.end(),
                           databaseRequest);
                if (requestIter != _dataToCompileList->_requestList.end())
                {
                    {
                        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeList->_requestMutex);
                        databaseRequest->_requestQueue = _dataToMergeList.get();
                        _dataToMergeList->_requestList.push_back(databaseRequest);
                    }
                    _dataToCompileList->_requestList.erase(requestIter);
                }
                
                if (!_dataToCompileList->_requestList.empty()) databaseRequest = _dataToCompileList->_requestList.front();
                else databaseRequest = 0;

            }
            else 
            {
                // osg::notify(osg::NOTICE)<<"Not all compiled"<<std::endl;
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

