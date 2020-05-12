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
#include <osg/ContextData>
#include <osg/os_utils>

#include <osg/FrameBufferObject>
#include <osg/Program>
#include <osg/Drawable>
#include <osg/FragmentProgram>
#include <osg/VertexProgram>
#include <osg/GLExtensions>

#include <OpenThreads/ReentrantMutex>

#include <osg/Notify>

#include <map>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdio.h>

using namespace osg;

/////////////////////////////////////////////////////////////////////////////
//
// WindowSystemInterfaces
//
GraphicsContext::WindowingSystemInterfaces::WindowingSystemInterfaces()
{
}

GraphicsContext::WindowingSystemInterfaces::~WindowingSystemInterfaces()
{
}

void GraphicsContext::WindowingSystemInterfaces::addWindowingSystemInterface(GraphicsContext::WindowingSystemInterface* wsi)
{
    if (std::find(_interfaces.begin(), _interfaces.end(), wsi)==_interfaces.end())
    {
        _interfaces.push_back(wsi);
    }
}

void GraphicsContext::WindowingSystemInterfaces::removeWindowingSystemInterface(GraphicsContext::WindowingSystemInterface* wsi)
{
    Interfaces::iterator itr = std::find(_interfaces.begin(), _interfaces.end(), wsi);
    if (itr!=_interfaces.end())
    {
        _interfaces.erase(itr);
    }
}

GraphicsContext::WindowingSystemInterface* GraphicsContext::WindowingSystemInterfaces::getWindowingSystemInterface(const std::string& name)
{
    if (_interfaces.empty())
    {
        OSG_WARN<<"Warning: GraphicsContext::WindowingSystemInterfaces::getWindowingSystemInterface() failed, no interfaces available."<<std::endl;
        return 0;
    }

    if (!name.empty())
    {
        for(Interfaces::iterator itr = _interfaces.begin();
            itr != _interfaces.end();
            ++itr)
        {
            if ((*itr)->getName()==name)
            {
                return itr->get();
            }

            OSG_NOTICE<<"   tried interface "<<typeid(*itr).name()<<", name= "<<(*itr)->getName()<<std::endl;
        }

        OSG_WARN<<"Warning: GraphicsContext::WindowingSystemInterfaces::getWindowingSystemInterface() failed, no interfaces matches name : "<<name<<std::endl;
        return 0;
    }
    else
    {
        // no preference provided so just take the first available interface
        return _interfaces.front().get();
    }
}

// Use a static reference pointer to hold the window system interface.
// Wrap this within a function, in order to control the order in which
// the static pointer's constructor is executed.
osg::ref_ptr<GraphicsContext::WindowingSystemInterfaces>& GraphicsContext::getWindowingSystemInterfaces()
{
    static ref_ptr<GraphicsContext::WindowingSystemInterfaces> s_WindowingSystemInterface = new GraphicsContext::WindowingSystemInterfaces;
    return s_WindowingSystemInterface;
}

OSG_INIT_SINGLETON_PROXY(ProxyInitWindowingSystemInterfaces, GraphicsContext::getWindowingSystemInterfaces())


//  GraphicsContext static method implementations
GraphicsContext::WindowingSystemInterface* GraphicsContext::getWindowingSystemInterface(const std::string& name)
{
    return GraphicsContext::getWindowingSystemInterfaces()->getWindowingSystemInterface(name);
}

GraphicsContext* GraphicsContext::createGraphicsContext(Traits* traits)
{
    ref_ptr<GraphicsContext::WindowingSystemInterface> wsref = getWindowingSystemInterface(traits ? traits->windowingSystemPreference : "") ;
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
    std::string str;
    if (getEnvVar("DISPLAY", str))
    {
        setScreenIdentifier(str);
    }
}

void GraphicsContext::ScreenIdentifier::setScreenIdentifier(const std::string& displayName)
{
    std::string::size_type colon = displayName.find_last_of(':');
    std::string::size_type point = displayName.find_last_of('.');

    // handle the case where the host name is supplied with '.' such as 127.0.0.1:0  with only DisplayNum provided
    // here the point to picks up on the .1 from the host name, rather then demarking the DisplayNum/ScreenNum as
    // no ScreenNum is provided, hence no . in the rhs of the :
    if (point!=std::string::npos &&
        colon!=std::string::npos &&
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
    OSG_NOTICE<<"   hostName ["<<hostName<<"]"<<std::endl;
    OSG_NOTICE<<"   displayNum "<<displayNum<<std::endl;
    OSG_NOTICE<<"   screenNum "<<screenNum<<std::endl;
#endif
}

GraphicsContext::Traits::Traits(DisplaySettings* ds):
            x(0),
            y(0),
            width(0),
            height(0),
            windowDecoration(false),
            supportsResize(true),
            red(8),
            blue(8),
            green(8),
            alpha(0),
            depth(24),
            stencil(0),
            sampleBuffers(0),
            samples(0),
            pbuffer(false),
            quadBufferStereo(false),
            doubleBuffer(false),
            target(0),
            format(0),
            level(0),
            face(0),
            mipMapGeneration(false),
            vsync(true),
            swapGroupEnabled(false),
            swapGroup(0),
            swapBarrier(0),
            useMultiThreadedOpenGLEngine(false),
            useCursor(true),
            glContextVersion(OSG_GL_CONTEXT_VERSION),
            glContextFlags(0),
            glContextProfileMask(0),
            sharedContext(0),
            setInheritedWindowPixelFormat(false),
            overrideRedirect(false),
            swapMethod( DisplaySettings::SWAP_DEFAULT )
{
    if (ds)
    {
        alpha = ds->getMinimumNumAlphaBits();
        stencil = ds->getMinimumNumStencilBits();
        if (ds->getMultiSamples()!=0) sampleBuffers = 1;
        samples = ds->getNumMultiSamples();
        if (ds->getStereo())
        {
            switch(ds->getStereoMode())
            {
                case(osg::DisplaySettings::QUAD_BUFFER): quadBufferStereo = true; break;
                case(osg::DisplaySettings::VERTICAL_INTERLACE):
                case(osg::DisplaySettings::CHECKERBOARD):
                case(osg::DisplaySettings::HORIZONTAL_INTERLACE): stencil = 8; break;
                default: break;
            }
        }

        glContextVersion = ds->getGLContextVersion();
        glContextFlags = ds->getGLContextFlags();
        glContextProfileMask = ds->getGLContextProfileMask();

        swapMethod = ds->getSwapMethod();
    }
}

bool GraphicsContext::Traits::getContextVersion(unsigned int& major, unsigned int& minor) const
{
    if (glContextVersion.empty()) return false;

    std::istringstream istr( glContextVersion );
    unsigned char dot;
    istr >> major >> dot >> minor;

    return true;
}

#if 0
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

        OSG_INFO<<"decrementUsageCount()"<<_numContexts<<std::endl;

        if (_numContexts <= 1 && _compileContext.valid())
        {
            OSG_INFO<<"resetting compileContext "<<_compileContext.get()<<" refCount "<<_compileContext->referenceCount()<<std::endl;

            _compileContext = 0;
        }
    }

    osg::ref_ptr<osg::GraphicsContext> _compileContext;

};
#endif

unsigned int GraphicsContext::createNewContextID()
{
    return ContextData::createNewContextID();
}

unsigned int GraphicsContext::getMaxContextID()
{
    return ContextData::getMaxContextID();
}


void GraphicsContext::incrementContextIDUsageCount(unsigned int contextID)
{
    return ContextData::incrementContextIDUsageCount(contextID);
}

void GraphicsContext::decrementContextIDUsageCount(unsigned int contextID)
{
    return ContextData::decrementContextIDUsageCount(contextID);
}


void GraphicsContext::registerGraphicsContext(GraphicsContext* gc)
{
    ContextData::registerGraphicsContext(gc);
}

void GraphicsContext::unregisterGraphicsContext(GraphicsContext* gc)
{
    ContextData::unregisterGraphicsContext(gc);
}

GraphicsContext::GraphicsContexts GraphicsContext::getAllRegisteredGraphicsContexts()
{
    return ContextData::getAllRegisteredGraphicsContexts();
}

GraphicsContext::GraphicsContexts GraphicsContext::getRegisteredGraphicsContexts(unsigned int contextID)
{
    return ContextData::getRegisteredGraphicsContexts(contextID);
}

GraphicsContext* GraphicsContext::getOrCreateCompileContext(unsigned int contextID)
{
    return ContextData::getOrCreateCompileContext(contextID);
}

void GraphicsContext::setCompileContext(unsigned int contextID, GraphicsContext* gc)
{
    return ContextData::setCompileContext(contextID, gc);
}

GraphicsContext* GraphicsContext::getCompileContext(unsigned int contextID)
{
    return ContextData::getCompileContext(contextID);
}


/////////////////////////////////////////////////////////////////////////////
//
//  GraphicsContext standard method implementations
//
GraphicsContext::GraphicsContext():
    _clearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearMask(0),
    _threadOfLastMakeCurrent(0),
    _lastClearTick(0),
    _defaultFboId(0)
{
    setThreadSafeRefUnref(true);
    _operationsBlock = new RefBlock;

    registerGraphicsContext(this);
}

GraphicsContext::GraphicsContext(const GraphicsContext&, const osg::CopyOp&):
    _clearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearMask(0),
    _threadOfLastMakeCurrent(0),
    _lastClearTick(0),
    _defaultFboId(0)
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
    _lastClearTick = osg::Timer::instance()->tick();

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
    OSG_INFO<<"GraphicsContext::close("<<callCloseImplementation<<")"<<this<<std::endl;

    // switch off the graphics thread...
    setGraphicsThread(0);


    bool sharedContextExists = false;

    if (_state.valid())
    {
        osg::ContextData* cd = osg::getContextData(_state->getContextID());
        if (cd && cd->getNumContexts()>1) sharedContextExists = true;
    }

    // release all the OpenGL objects in the scene graphs associated with this
    for(Cameras::iterator itr = _cameras.begin();
        itr != _cameras.end();
        ++itr)
    {
        Camera* camera = (*itr);
        if (camera)
        {
            OSG_INFO<<"Releasing GL objects for Camera="<<camera<<" _state="<<_state.get()<<std::endl;
            camera->releaseGLObjects(_state.get());
        }
    }

    if (_state.valid())
    {
        _state->releaseGLObjects();
    }

    if (callCloseImplementation && _state.valid() && isRealized())
    {
        OSG_INFO<<"Closing still viable window "<<sharedContextExists<<" _state->getContextID()="<<_state->getContextID()<<std::endl;

        if (makeCurrent())
        {
            if ( !sharedContextExists )
            {
                OSG_INFO<<"Doing delete of GL objects"<<std::endl;

                osg::deleteAllGLObjects(_state->getContextID());

                osg::flushAllDeletedGLObjects(_state->getContextID());

                OSG_INFO<<"Done delete of GL objects"<<std::endl;
            }
            else
            {
                // If the GL objects are shared with other contexts then only flush those
                // which have already been deleted

                osg::flushAllDeletedGLObjects(_state->getContextID());
            }

            releaseContext();
        }
        else
        {
            OSG_INFO<<"makeCurrent did not succeed, could not do flush/deletion of OpenGL objects."<<std::endl;
        }
    }

    if (callCloseImplementation) closeImplementation();


    // now discard any deleted deleted OpenGL objects that the are still hanging around - such as due to
    // the flushDelete*() methods not being invoked, such as when using GraphicContextEmbedded where makeCurrent
    // does not work.
    if ( !sharedContextExists && _state.valid())
    {
        OSG_INFO<<"Doing discard of deleted OpenGL objects."<<std::endl;

        osg::discardAllGLObjects(_state->getContextID());
    }

    if (_state.valid())
    {
        decrementContextIDUsageCount(_state->getContextID());

        _state = 0;
    }
}


bool GraphicsContext::makeCurrent()
{
    _threadOfLastMakeCurrent = OpenThreads::Thread::CurrentThreadId();

    bool result = makeCurrentImplementation();

    if (result)
    {
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
        _threadOfLastMakeCurrent = OpenThreads::Thread::CurrentThreadId();

        // initialize extension process, not only initializes on first
        // call, will be a non-op on subsequent calls.
        getState()->initializeExtensionProcs();
    }

    return result;
}

bool GraphicsContext::releaseContext()
{
    bool result = releaseContextImplementation();

    _threadOfLastMakeCurrent = 0;

    return result;
}

void GraphicsContext::swapBuffers()
{
    if (isCurrent())
    {
        swapBuffersCallbackOrImplementation();
        clear();
    }
    else if (_graphicsThread.valid() &&
             _threadOfLastMakeCurrent == _graphicsThread->getThreadId())
    {
        _graphicsThread->add(new SwapBuffersOperation);
    }
    else
    {
        makeCurrent();
        swapBuffersCallbackOrImplementation();
        clear();
    }
}

void GraphicsContext::createGraphicsThread()
{
    if (!_graphicsThread)
    {
        setGraphicsThread(new GraphicsThread);

        if (_traits.valid())
        {
            _graphicsThread->setProcessorAffinity(_traits->affinity);
        }
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
    OSG_INFO<<"Doing add"<<std::endl;

    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // add the operation to the end of the list
    _operations.push_back(operation);

    _operationsBlock->set(true);
}

void GraphicsContext::remove(Operation* operation)
{
    OSG_INFO<<"Doing remove operation"<<std::endl;

    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    for(GraphicsOperationQueue::iterator itr = _operations.begin();
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
    OSG_INFO<<"Doing remove named operation"<<std::endl;

    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // find the remove all operations with specified name
    for(GraphicsOperationQueue::iterator itr = _operations.begin();
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
    OSG_INFO<<"Doing remove all operations"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
    _operations.clear();
    _operationsBlock->set(false);
}

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

    for(GraphicsOperationQueue::iterator itr = _operations.begin();
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
            // OSG_INFO<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

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
            (*nitr)->releaseGLObjects(_state.get());
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
    std::set<osg::Viewport*> processedViewports;

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
            // avoid processing a shared viewport twice
            if (processedViewports.count(viewport)==0)
            {
                processedViewports.insert(viewport);

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SyncSwapBuffersCallback
//
SyncSwapBuffersCallback::SyncSwapBuffersCallback()
{
    OSG_INFO<<"Created SyncSwapBuffersCallback."<<std::endl;
    _previousSync = 0;
}

void SyncSwapBuffersCallback::swapBuffersImplementation(osg::GraphicsContext* gc)
{
    // OSG_NOTICE<<"Before swap - place to do swap ready sync"<<std::endl;
    gc->swapBuffersImplementation();
    //glFinish();

    GLExtensions* ext = gc->getState()->get<GLExtensions>();

    if (ext->glClientWaitSync)
    {
        if (_previousSync)
        {
            unsigned int num_seconds = 1;
            GLuint64 timeout = num_seconds * ((GLuint64)1000 * 1000 * 1000);
            ext->glClientWaitSync(_previousSync, 0, timeout);
            ext->glDeleteSync(_previousSync);
        }

        _previousSync = ext->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
    //gc->getState()->checkGLErrors("after glWaitSync");

    //OSG_NOTICE<<"After swap"<<std::endl;
}

