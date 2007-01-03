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


#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/View>

#include <osg/Notify>

#include <map>
#include <sstream>

using namespace osg;

static ref_ptr<GraphicsContext::WindowingSystemInterface> s_WindowingSystemInterface;

void GraphicsContext::setWindowingSystemInterface(WindowingSystemInterface* callback)
{
    s_WindowingSystemInterface = callback;
}

GraphicsContext::WindowingSystemInterface* GraphicsContext::getWindowingSystemInterface()
{
    return s_WindowingSystemInterface.get();
}

GraphicsContext* GraphicsContext::createGraphicsContext(Traits* traits)
{
    if (s_WindowingSystemInterface.valid())
        return s_WindowingSystemInterface->createGraphicsContext(traits);
    else
        return 0;    
}


std::string GraphicsContext::ScreenIdentifier::displayName() const
{
    std::stringstream ostr;
    ostr<<hostName<<":"<<displayNum<<"."<<screenNum;
    return ostr.str();
}


typedef std::map<unsigned int, unsigned int>  ContextIDMap;
static ContextIDMap s_contextIDMap;
static OpenThreads::Mutex s_contextIDMapMutex;

unsigned int GraphicsContext::createNewContextID()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    
    // first check to see if we can reuse contextID;
    for(ContextIDMap::iterator itr = s_contextIDMap.begin();
        itr != s_contextIDMap.end();
        ++itr)
    {
        if (itr->second == 0)
        {

            // reuse contextID;
            itr->second = 1;

            osg::notify(osg::INFO)<<"GraphicsContext::createNewContextID() reusing contextID="<<itr->first<<std::endl;

            return itr->first;
        }
    }

    unsigned int contextID = s_contextIDMap.size();
    s_contextIDMap[contextID] = 1;
    
    osg::notify(osg::INFO)<<"GraphicsContext::createNewContextID() creating contextID="<<contextID<<std::endl;
    

    if ( (contextID+1) > osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts() )
    {
        osg::notify(osg::INFO)<<"Updating the MaxNumberOfGraphicsContexts to "<<contextID+1<<std::endl;

        // update the the maximum number of graphics contexts, 
        // to ensure that texture objects and display buffers are configured to the correct size.
        osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts( contextID + 1 );
    }
    

    return contextID;    
}

void GraphicsContext::incrementContextIDUsageCount(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    
    osg::notify(osg::INFO)<<"GraphicsContext::incrementContextIDUsageCount("<<contextID<<") to "<<s_contextIDMap[contextID]<<std::endl;

    ++s_contextIDMap[contextID];
}

void GraphicsContext::decrementContextIDUsageCount(unsigned int contextID)
{

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    

    if (s_contextIDMap[contextID]!=0)
    {
        --s_contextIDMap[contextID];
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Warning: decrementContextIDUsageCount("<<contextID<<") called on expired contextID."<<std::endl;
    } 

    osg::notify(osg::INFO)<<"GraphicsContext::decrementContextIDUsageCount("<<contextID<<") to "<<s_contextIDMap[contextID]<<std::endl;

}


GraphicsContext::GraphicsContext():
    _threadOfLastMakeCurrent(0)
{
    _operationsBlock = new Block;
}

GraphicsContext::~GraphicsContext()
{
    close(false);
}

/** Realise the GraphicsContext.*/
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
    // switch off the graphics thread...
    setGraphicsThread(0);
    
    if (callCloseImplementation) closeImplementation();

    if (_state.valid())
    {
        decrementContextIDUsageCount(_state->getContextID());
        
        _state = 0;
    }
}


void GraphicsContext::makeCurrent()
{
    ReleaseContext_Block_MakeCurrentOperation* rcbmco = 0;

    if (_graphicsThread.valid() && 
        _threadOfLastMakeCurrent == _graphicsThread.get() &&
        _threadOfLastMakeCurrent != OpenThreads::Thread::CurrentThread())
    {
        // create a relase contex, block and make current operation to stop the graphics thread while we use the graphics context for ourselves
        rcbmco = new ReleaseContext_Block_MakeCurrentOperation;
        _graphicsThread->add(rcbmco);
    }

    if (!isCurrent()) _mutex.lock();

    makeCurrentImplementation();
    
    _threadOfLastMakeCurrent = OpenThreads::Thread::CurrentThread();
    
    if (rcbmco)
    {
        // Let the "relase contex, block and make current operation" proceed, which will now move on to trying to aquire the graphics
        // contex itself with a makeCurrent(), this will then block on the GraphicsContext mutex till releaseContext() releases it.
        rcbmco->release();
    }
}

void GraphicsContext::makeContextCurrent(GraphicsContext* readContext)
{
    if (!isCurrent()) _mutex.lock();

    makeContextCurrentImplementation(readContext);

    _threadOfLastMakeCurrent = OpenThreads::Thread::CurrentThread();
}

void GraphicsContext::releaseContext()
{
    _mutex.unlock();
}

void GraphicsContext::swapBuffers()
{
    if (isCurrent())
    {
        swapBuffersImplementation();
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
        releaseContext();
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
        _graphicsThread->_graphicsContext = 0;
    }

    _graphicsThread = gt;
    
    if (_graphicsThread.valid()) 
    {
        _graphicsThread->_graphicsContext = this;
    }
}

void GraphicsContext::add(GraphicsOperation* operation)
{
    osg::notify(osg::INFO)<<"Doing add"<<std::endl;

    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // add the operation to the end of the list
    _operations.push_back(operation);

    _operationsBlock->set(true);
}

void GraphicsContext::remove(GraphicsOperation* operation)
{
    osg::notify(osg::INFO)<<"Doing remove operation"<<std::endl;

    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
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
    
    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // find the remove all operations with specificed name
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

void GraphicsContext::runOperations()
{
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
            osg::notify(osg::INFO)<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

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
    for(Cameras::iterator itr = _cameras.begin();
        itr != _cameras.end();
        ++itr)
    {
        if (*itr == camera)
        {
            _cameras.erase(itr);
            return;
        }
    }
}

void GraphicsContext::resized(int x, int y, int width, int height)
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

            if (slave && camera->getReferenceFrame()==osg::Transform::RELATIVE_RF)
            {
                slave->_projectionOffset *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
            }
            else
            {
#if 0            
                osg::Matrixd& pm = camera->getProjectionMatrix();
                bool orthographicCamera = (pm(0,3)==0.0) && (pm(1,3)==0.0) && (pm(2,3)==0.0) && (pm(3,3)==1.0); 

                if (orthographicCamera)
                {
                    double left, right, bottom, top, zNear, zFar;
                    camera->getProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar);

                    double mid = (right+left)*0.5;
                    double halfWidth = (right-left)*0.5;
                    left = mid - halfWidth * aspectRatioChange;
                    right = mid + halfWidth * aspectRatioChange;
                    camera->setProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar);
                }
                else
                {
                    double left, right, bottom, top, zNear, zFar;
                    camera->getProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);

                    double mid = (right+left)*0.5;
                    double halfWidth = (right-left)*0.5;
                    left = mid - halfWidth * aspectRatioChange;
                    right = mid + halfWidth * aspectRatioChange;
                    camera->setProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);
                }
#else
                camera->getProjectionMatrix() *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
#endif
            }

        }    

    }
    
    _traits->x = x;
    _traits->y = y;
    _traits->width = width;
    _traits->height = height;
}
