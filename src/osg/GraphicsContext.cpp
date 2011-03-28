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

#include <stdlib.h>

#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/View>
#include <osg/GLObjects>

#include <osg/FrameBufferObject>
#include <osg/Program>
#include <osg/Drawable>
#include <osg/FragmentProgram>
#include <osg/VertexProgram>

#include <OpenThreads/ReentrantMutex>

#include <osg/Notify>

#include <map>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace osg;

/////////////////////////////////////////////////////////////////////////////


// Use a static reference pointer to hold the window system interface.
// Wrap this within a function, in order to control the order in which
// the static pointer's constructor is executed. 

static ref_ptr<GraphicsContext::WindowingSystemInterface> &windowingSystemInterfaceRef()
{
    static ref_ptr<GraphicsContext::WindowingSystemInterface> s_WindowingSystemInterface;
    return s_WindowingSystemInterface;
}


//  GraphicsContext static method implementations

void GraphicsContext::setWindowingSystemInterface(WindowingSystemInterface* callback)
{
    ref_ptr<GraphicsContext::WindowingSystemInterface> &wsref = windowingSystemInterfaceRef();
    wsref = callback;
    osg::notify(osg::INFO)<<"GraphicsContext::setWindowingSystemInterface() "<<wsref.get()<<"\t"<<&wsref<<std::endl;
}

GraphicsContext::WindowingSystemInterface* GraphicsContext::getWindowingSystemInterface()
{
    ref_ptr<GraphicsContext::WindowingSystemInterface> &wsref = windowingSystemInterfaceRef();
    osg::notify(osg::INFO)<<"GraphicsContext::getWindowingSystemInterface() "<<wsref.get()<<"\t"<<&wsref<<std::endl;
    return wsref.get();
}

GraphicsContext* GraphicsContext::createGraphicsContext(Traits* traits)
{
    ref_ptr<GraphicsContext::WindowingSystemInterface> &wsref = windowingSystemInterfaceRef();
    if ( wsref.valid())
    {
        // catch any undefined values.
        if (traits) traits->setUndefinedScreenDetailsToDefaultScreen();
        
        return wsref->createGraphicsContext(traits);
    }
    else
        return 0;    
}

GraphicsContext::ScreenIdentifier::ScreenIdentifier():
    displayNum(0),
    screenNum(0) {}

GraphicsContext::ScreenIdentifier::ScreenIdentifier(int in_screenNum):
    displayNum(0),
    screenNum(in_screenNum) {}

GraphicsContext::ScreenIdentifier::ScreenIdentifier(const std::string& in_hostName,int in_displayNum, int in_screenNum):
    hostName(in_hostName),
    displayNum(in_displayNum),
    screenNum(in_screenNum) {}

std::string GraphicsContext::ScreenIdentifier::displayName() const
{
    std::stringstream ostr;
    ostr<<hostName<<":"<<displayNum<<"."<<screenNum;
    return ostr.str();
}

void GraphicsContext::ScreenIdentifier::readDISPLAY()
{
    const char* ptr = 0;
    if ((ptr=getenv("DISPLAY")) != 0)
    {
        setScreenIdentifier(ptr);
    }
}

void GraphicsContext::ScreenIdentifier::setScreenIdentifier(const std::string& displayName)
{
    std::string::size_type colon = displayName.find_last_of(':');
    std::string::size_type point = displayName.find_last_of('.');
    
    if (point!=std::string::npos && 
        colon==std::string::npos && 
        point < colon) point = std::string::npos;

    if (colon==std::string::npos)
    {
        hostName = "";
    }
    else
    {
        hostName = displayName.substr(0,colon);
    }
    
    std::string::size_type startOfDisplayNum = (colon==std::string::npos) ? 0 : colon+1;
    std::string::size_type endOfDisplayNum = (point==std::string::npos) ?  displayName.size() : point;

    if (startOfDisplayNum<endOfDisplayNum)
    {
        displayNum = atoi(displayName.substr(startOfDisplayNum,endOfDisplayNum-startOfDisplayNum).c_str());
    }
    else
    {
        displayNum = -1;
    }

    if (point!=std::string::npos && point+1<displayName.size())
    {
        screenNum = atoi(displayName.substr(point+1,displayName.size()-point-1).c_str());
    }
    else
    {
        screenNum = -1;
    }

#if 0    
    osg::notify(osg::NOTICE)<<"   hostName ["<<hostName<<"]"<<std::endl;
    osg::notify(osg::NOTICE)<<"   displayNum "<<displayNum<<std::endl;
    osg::notify(osg::NOTICE)<<"   screenNum "<<screenNum<<std::endl;
#endif
}

class ContextData
{
public:

    ContextData():
        _numContexts(0) {}

    unsigned int _numContexts;
    
    void incrementUsageCount() {  ++_numContexts; }

    void decrementUsageCount()
    {
        --_numContexts;

        osg::notify(osg::INFO)<<"decrementUsageCount()"<<_numContexts<<std::endl;

        if (_numContexts <= 1 && _compileContext.valid())
        {
            osg::notify(osg::INFO)<<"resetting compileContext "<<_compileContext.get()<<" refCount "<<_compileContext->referenceCount()<<std::endl;
            
            _compileContext = 0;
        }
    }
    
    osg::ref_ptr<osg::GraphicsContext> _compileContext;

};


typedef std::map<unsigned int, ContextData>  ContextIDMap;
static ContextIDMap s_contextIDMap;
static OpenThreads::ReentrantMutex s_contextIDMapMutex;
static GraphicsContext::GraphicsContexts s_registeredContexts;

unsigned int GraphicsContext::createNewContextID()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    
    // first check to see if we can reuse contextID;
    for(ContextIDMap::iterator itr = s_contextIDMap.begin();
        itr != s_contextIDMap.end();
        ++itr)
    {
        if (itr->second._numContexts == 0)
        {

            // reuse contextID;
            itr->second._numContexts = 1;

            osg::notify(osg::INFO)<<"GraphicsContext::createNewContextID() reusing contextID="<<itr->first<<std::endl;

            return itr->first;
        }
    }

    unsigned int contextID = s_contextIDMap.size();
    s_contextIDMap[contextID]._numContexts = 1;
    
    osg::notify(osg::INFO)<<"GraphicsContext::createNewContextID() creating contextID="<<contextID<<std::endl;
    osg::notify(osg::INFO)<<"Updating the MaxNumberOfGraphicsContexts to "<<contextID+1<<std::endl;

    // update the the maximum number of graphics contexts, 
    // to ensure that texture objects and display buffers are configured to the correct size.
    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts( contextID + 1 );

    return contextID;    
}

unsigned int GraphicsContext::getMaxContextID()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    unsigned int maxContextID = 0;
    for(ContextIDMap::iterator itr = s_contextIDMap.begin();
        itr != s_contextIDMap.end();
        ++itr)
    {
        if (itr->first > maxContextID) maxContextID = itr->first;
    }
    return maxContextID;
}


void GraphicsContext::incrementContextIDUsageCount(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    
    osg::notify(osg::INFO)<<"GraphicsContext::incrementContextIDUsageCount("<<contextID<<") to "<<s_contextIDMap[contextID]._numContexts<<std::endl;

    s_contextIDMap[contextID].incrementUsageCount();
}

void GraphicsContext::decrementContextIDUsageCount(unsigned int contextID)
{

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    if (s_contextIDMap[contextID]._numContexts!=0)
    {
        s_contextIDMap[contextID].decrementUsageCount();
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Warning: decrementContextIDUsageCount("<<contextID<<") called on expired contextID."<<std::endl;
    } 

    osg::notify(osg::INFO)<<"GraphicsContext::decrementContextIDUsageCount("<<contextID<<") to "<<s_contextIDMap[contextID]._numContexts<<std::endl;

}


void GraphicsContext::registerGraphicsContext(GraphicsContext* gc)
{
    osg::notify(osg::INFO)<<"GraphicsContext::registerGraphicsContext "<<gc<<std::endl;

    if (!gc) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    GraphicsContexts::iterator itr = std::find(s_registeredContexts.begin(), s_registeredContexts.end(), gc);
    if (itr != s_registeredContexts.end()) s_registeredContexts.erase(itr);

    s_registeredContexts.push_back(gc);
}

void GraphicsContext::unregisterGraphicsContext(GraphicsContext* gc)
{
    osg::notify(osg::INFO)<<"GraphicsContext::unregisterGraphicsContext "<<gc<<std::endl;

    if (!gc) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    GraphicsContexts::iterator itr = std::find(s_registeredContexts.begin(), s_registeredContexts.end(), gc);
    if (itr != s_registeredContexts.end()) s_registeredContexts.erase(itr);
}

GraphicsContext::GraphicsContexts GraphicsContext::getAllRegisteredGraphicsContexts()
{
    osg::notify(osg::INFO)<<"GraphicsContext::getAllRegisteredGraphicsContexts s_registeredContexts.size()="<<s_registeredContexts.size()<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    return s_registeredContexts;
}

GraphicsContext::GraphicsContexts GraphicsContext::getRegisteredGraphicsContexts(unsigned int contextID)
{
    GraphicsContexts contexts;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    for(GraphicsContexts::iterator itr = s_registeredContexts.begin();
        itr != s_registeredContexts.end();
        ++itr)
    {
        GraphicsContext* gc = *itr;
        if (gc->getState() && gc->getState()->getContextID()==contextID) contexts.push_back(gc);
    }

    osg::notify(osg::INFO)<<"GraphicsContext::getRegisteredGraphicsContexts "<<contextID<<" contexts.size()="<<contexts.size()<<std::endl;
    
    return contexts;
}

GraphicsContext* GraphicsContext::getOrCreateCompileContext(unsigned int contextID)
{
    osg::notify(osg::INFO)<<"GraphicsContext::createCompileContext."<<std::endl;

    {    
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
        if (s_contextIDMap[contextID]._compileContext.valid()) return s_contextIDMap[contextID]._compileContext.get();
    }
    
    GraphicsContext::GraphicsContexts contexts = GraphicsContext::getRegisteredGraphicsContexts(contextID);
    if (contexts.empty()) return 0;
    
    GraphicsContext* src_gc = contexts.front();
    const osg::GraphicsContext::Traits* src_traits = src_gc->getTraits();

    osg::GraphicsContext::Traits* traits = new osg::GraphicsContext::Traits;
    traits->screenNum = src_traits->screenNum;
    traits->displayNum = src_traits->displayNum;
    traits->hostName = src_traits->hostName;
    traits->width = 100;
    traits->height = 100;
    traits->red = src_traits->red;
    traits->green = src_traits->green;
    traits->blue = src_traits->blue;
    traits->alpha = src_traits->alpha;
    traits->depth = src_traits->depth;
    traits->sharedContext = src_gc;
    traits->pbuffer = true;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);
    if (gc.valid() && gc->realize())
    {    
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
        s_contextIDMap[contextID]._compileContext = gc;
        osg::notify(osg::INFO)<<"   succeeded GraphicsContext::createCompileContext."<<std::endl;
        return gc.release();
    }
    else
    {
        return 0;
    }
    
}

void GraphicsContext::setCompileContext(unsigned int contextID, GraphicsContext* gc)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    s_contextIDMap[contextID]._compileContext = gc;
}

GraphicsContext* GraphicsContext::getCompileContext(unsigned int contextID)
{
    // osg::notify(osg::NOTICE)<<"GraphicsContext::getCompileContext "<<contextID<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    ContextIDMap::iterator itr = s_contextIDMap.find(contextID);
    if (itr != s_contextIDMap.end()) return itr->second._compileContext.get();
    else return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
//  GraphicsContext standard method implementations
//
GraphicsContext::GraphicsContext():
    _clearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearMask(0),
    _threadOfLastMakeCurrent(0)
{
    setThreadSafeRefUnref(true);
    _operationsBlock = new RefBlock;

    registerGraphicsContext(this);
}

GraphicsContext::GraphicsContext(const GraphicsContext&, const osg::CopyOp&):
    _clearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearMask(0),
    _threadOfLastMakeCurrent(0)
{
    setThreadSafeRefUnref(true);
    _operationsBlock = new RefBlock;

    registerGraphicsContext(this);
}

GraphicsContext::~GraphicsContext()
{
    close(false);

    unregisterGraphicsContext(this);
}

void GraphicsContext::clear()
{
    if (_clearMask==0 || !_traits) return;

    glViewport(0, 0, _traits->width, _traits->height);
    glScissor(0, 0, _traits->width, _traits->height);

    glClearColor( _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);

    glClear( _clearMask );
}

bool GraphicsContext::realize()
{
    if (realizeImplementation())
    {
        return true;
    }
    else
    {   
        return false;
    }
}

void GraphicsContext::close(bool callCloseImplementation)
{
    osg::notify(osg::INFO)<<"close("<<callCloseImplementation<<")"<<this<<std::endl;

    // switch off the graphics thread...
    setGraphicsThread(0);
    
    
    bool sharedContextExists = false;
    
    if (_state.valid())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
        if (s_contextIDMap[_state->getContextID()]._numContexts>1) sharedContextExists = true;
    }

    // release all the OpenGL objects in the scene graphs associated with this 
    for(Cameras::iterator itr = _cameras.begin();
        itr != _cameras.end();
        ++itr)
    {
        Camera* camera = (*itr);
        if (camera)
        {
            osg::notify(osg::INFO)<<"Releasing GL objects for Camera="<<camera<<" _state="<<_state.get()<<std::endl;
            camera->releaseGLObjects(_state.get());
        }
    }


    if (callCloseImplementation && _state.valid() && isRealized())
    {
        osg::notify(osg::INFO)<<"Closing still viable window "<<sharedContextExists<<" _state->getContextID()="<<_state->getContextID()<<std::endl;

        if (makeCurrent())
        {
        
            osg::notify(osg::INFO)<<"Doing Flush"<<std::endl;

            osg::flushAllDeletedGLObjects(_state->getContextID());

            osg::notify(osg::INFO)<<"Done Flush "<<std::endl;

            _state->reset();

            releaseContext();
        }
        else
        {
            osg::notify(osg::INFO)<<"makeCurrent did not succeed, could not do flush/deletion of OpenGL objects."<<std::endl;
        }
    }
    
    if (callCloseImplementation) closeImplementation();


    // now discard any deleted deleted OpenGL objects that the are still hanging around - such as due to 
    // the the flushDelete*() methods not being invoked, such as when using GraphicContextEmbedded where makeCurrent
    // does not work.
    if (_state.valid())
    {
        osg::notify(osg::INFO)<<"Doing discard of deleted OpenGL objects."<<std::endl;

        osg::discardAllDeletedGLObjects(_state->getContextID());
    }

    if (_state.valid())
    {
        decrementContextIDUsageCount(_state->getContextID());
        
        _state = 0;
    }
}


bool GraphicsContext::makeCurrent()
{
    bool result = makeCurrentImplementation();
    
    if (result)
    {
        _threadOfLastMakeCurrent = OpenThreads::Thread::CurrentThread();

        // initialize extension process, not only initializes on first
        // call, will be a non-op on subsequent calls.        
        getState()->initializeExtensionProcs();
    }
    
    return result;
}

bool GraphicsContext::makeContextCurrent(GraphicsContext* readContext)
{
    bool result = makeContextCurrentImplementation(readContext);

    if (result)
    {
        _threadOfLastMakeCurrent = OpenThreads::Thread::CurrentThread();

        // initialize extension process, not only initializes on first
        // call, will be a non-op on subsequent calls.        
        getState()->initializeExtensionProcs();
    }
    
    return result;
}

bool GraphicsContext::releaseContext()
{
    bool result = releaseContextImplementation();
    
    _threadOfLastMakeCurrent = (OpenThreads::Thread*)(-1);
    
    return result;
}

void GraphicsContext::swapBuffers()
{
    if (isCurrent())
    {
        swapBuffersImplementation();
        clear();
    }
    else if (_graphicsThread.valid() && 
             _threadOfLastMakeCurrent == _graphicsThread.get())
    {
        _graphicsThread->add(new SwapBuffersOperation);
    }
    else
    {
        makeCurrent();
        swapBuffersImplementation();
        clear();
    }
}



void GraphicsContext::createGraphicsThread()
{
    if (!_graphicsThread)
    {
        setGraphicsThread(new GraphicsThread);
    }
}

void GraphicsContext::setGraphicsThread(GraphicsThread* gt)
{
    if (_graphicsThread==gt) return; 

    if (_graphicsThread.valid()) 
    {
        // need to kill the thread in some way...
        _graphicsThread->cancel();
        _graphicsThread->setParent(0);
    }

    _graphicsThread = gt;
    
    if (_graphicsThread.valid()) 
    {
        _graphicsThread->setParent(this);
    }
}

void GraphicsContext::add(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing add"<<std::endl;

    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // add the operation to the end of the list
    _operations.push_back(operation);

    _operationsBlock->set(true);
}

void GraphicsContext::remove(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing remove operation"<<std::endl;

    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    for(OperationQueue::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)==operation) itr = _operations.erase(itr);
        else ++itr;
    }

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}

void GraphicsContext::remove(const std::string& name)
{
    osg::notify(osg::INFO)<<"Doing remove named operation"<<std::endl;
    
    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // find the remove all operations with specified name
    for(OperationQueue::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)->getName()==name) itr = _operations.erase(itr);
        else ++itr;
    }

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}

void GraphicsContext::removeAllOperations()
{
    osg::notify(osg::INFO)<<"Doing remove all operations"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
    _operations.clear();
    _operationsBlock->set(false);
}


struct CameraRenderOrderSortOp
{
    inline bool operator() (const Camera* lhs,const Camera* rhs) const
    {
        if (lhs->getRenderOrder()<rhs->getRenderOrder()) return true;
        if (rhs->getRenderOrder()<lhs->getRenderOrder()) return false;
        return lhs->getRenderOrderNum()<rhs->getRenderOrderNum();
    }
};


void GraphicsContext::runOperations()
{
    // sort the cameras into order
    typedef std::vector<Camera*> CameraVector;
    CameraVector camerasCopy;
    std::copy(_cameras.begin(), _cameras.end(), std::back_inserter(camerasCopy));
    std::sort(camerasCopy.begin(), camerasCopy.end(), CameraRenderOrderSortOp());
    
    for(CameraVector::iterator itr = camerasCopy.begin();
        itr != camerasCopy.end();
        ++itr)
    {
        osg::Camera* camera = *itr;
        if (camera->getRenderer()) (*(camera->getRenderer()))(this);
    }

    for(OperationQueue::iterator itr = _operations.begin();
        itr != _operations.end();
        )
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
            _currentOperation = *itr;

            if (!_currentOperation->getKeep())
            {
                itr = _operations.erase(itr);

                if (_operations.empty())
                {
                    _operationsBlock->set(false);
                }
            }
            else
            {
                ++itr;
            }
        }
                
        if (_currentOperation.valid())
        {
            // osg::notify(osg::INFO)<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

            // call the graphics operation.
            (*_currentOperation)(this);

            {            
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
                _currentOperation = 0;
            }
        }
    }
}

void GraphicsContext::addCamera(osg::Camera* camera)
{
    _cameras.push_back(camera);
}

void GraphicsContext::removeCamera(osg::Camera* camera)
{
    Cameras::iterator itr = std::find(_cameras.begin(), _cameras.end(), camera);
    if (itr != _cameras.end())
    {
        // find a set of nodes attached the camera that we are removing that isn't
        // shared by any other cameras on this GraphicsContext
        typedef std::set<Node*> NodeSet;
        NodeSet nodes;
        for(unsigned int i=0; i<camera->getNumChildren(); ++i)
        {
            nodes.insert(camera->getChild(i));            
        }
        
        for(Cameras::iterator citr = _cameras.begin();
            citr != _cameras.end();
            ++citr)
        {
            if (citr != itr)
            {
                osg::Camera* otherCamera = *citr;
                for(unsigned int i=0; i<otherCamera->getNumChildren(); ++i)
                {
                    NodeSet::iterator nitr = nodes.find(otherCamera->getChild(i));
                    if (nitr != nodes.end()) nodes.erase(nitr);
                }
            }            
        }
        
        // now release the GLobjects associated with these non shared nodes
        for(NodeSet::iterator nitr = nodes.begin();
            nitr != nodes.end();
            ++nitr)
        {
            const_cast<osg::Node*>(*nitr)->releaseGLObjects(_state.get());
        }

        // release the context of the any RenderingCache that the Camera has.
        if (camera->getRenderingCache())
        {
            camera->getRenderingCache()->releaseGLObjects(_state.get());
        }

        _cameras.erase(itr);

    }
}

void GraphicsContext::resizedImplementation(int x, int y, int width, int height)
{
    if (!_traits) return;
    
    double widthChangeRatio = double(width) / double(_traits->width);
    double heigtChangeRatio = double(height) / double(_traits->height);
    double aspectRatioChange = widthChangeRatio / heigtChangeRatio; 
    
    for(Cameras::iterator itr = _cameras.begin();
        itr != _cameras.end();
        ++itr)
    {
        Camera* camera = (*itr);
        
        // resize doesn't affect Cameras set up with FBO's.
        if (camera->getRenderTargetImplementation()==osg::Camera::FRAME_BUFFER_OBJECT) continue;
        
        Viewport* viewport = camera->getViewport();
        if (viewport)
        {
            if (viewport->x()==0 && viewport->y()==0 &&
                viewport->width()>=_traits->width && viewport->height()>=_traits->height)
            {
                viewport->setViewport(0,0,width,height);
            }
            else
            {
                viewport->x() = static_cast<osg::Viewport::value_type>(double(viewport->x())*widthChangeRatio);
                viewport->y() = static_cast<osg::Viewport::value_type>(double(viewport->y())*heigtChangeRatio);
                viewport->width() = static_cast<osg::Viewport::value_type>(double(viewport->width())*widthChangeRatio);
                viewport->height() = static_cast<osg::Viewport::value_type>(double(viewport->height())*heigtChangeRatio);
            }
        }

        // if aspect ratio adjusted change the project matrix to suit.
        if (aspectRatioChange != 1.0)
        {
            osg::View* view = camera->getView();
            osg::View::Slave* slave = view ? view->findSlaveForCamera(camera) : 0;


            if (slave)
            {
                if (camera->getReferenceFrame()==osg::Transform::RELATIVE_RF)
                {
                    switch(view->getCamera()->getProjectionResizePolicy())
                    {
                        case(osg::Camera::HORIZONTAL): slave->_projectionOffset *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0); break;
                        case(osg::Camera::VERTICAL): slave->_projectionOffset *= osg::Matrix::scale(1.0, aspectRatioChange,1.0); break;
                        default: break;
                    }
                }
                else
                {
                    switch(camera->getProjectionResizePolicy())
                    {
                        case(osg::Camera::HORIZONTAL): camera->getProjectionMatrix() *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0); break;
                        case(osg::Camera::VERTICAL): camera->getProjectionMatrix() *= osg::Matrix::scale(1.0, aspectRatioChange,1.0); break;
                        default: break;
                    }
                }
            }
            else
            {
                Camera::ProjectionResizePolicy policy = view ? view->getCamera()->getProjectionResizePolicy() : camera->getProjectionResizePolicy();
                switch(policy)
                {
                    case(osg::Camera::HORIZONTAL): camera->getProjectionMatrix() *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0); break;
                    case(osg::Camera::VERTICAL): camera->getProjectionMatrix() *= osg::Matrix::scale(1.0, aspectRatioChange,1.0); break;
                    default: break;
                }

                osg::Camera* master = view ? view->getCamera() : 0;
                if (view && camera==master)
                {
                    for(unsigned int i=0; i<view->getNumSlaves(); ++i)
                    {
                        osg::View::Slave& child = view->getSlave(i);
                        if (child._camera.valid() && child._camera->getReferenceFrame()==osg::Transform::RELATIVE_RF)
                        {
                            // scale the slaves by the inverse of the change that has been applied to master, to avoid them be
                            // scaled twice (such as when both master and slave are on the same GraphicsContexts) or by the wrong scale
                            // when master and slave are on different GraphicsContexts.
                            switch(policy)
                            {
                                case(osg::Camera::HORIZONTAL): child._projectionOffset *= osg::Matrix::scale(aspectRatioChange,1.0,1.0); break;
                                case(osg::Camera::VERTICAL): child._projectionOffset *= osg::Matrix::scale(1.0, 1.0/aspectRatioChange,1.0); break;
                                default: break;
                            }
                        }
                    }
                }


            }

        }

    }

    _traits->x = x;
    _traits->y = y;
    _traits->width = width;
    _traits->height = height;
}
