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

#include <osgGA/SimpleViewer>
#include <osgGA/TrackballManipulator>

using namespace osgGA;

SimpleViewer::SimpleViewer():
    _firstFrame(true)
{
    _sceneView = new osgUtil::SceneView;
    _sceneView->setDefaults();
    _sceneView->getState()->setContextID(osg::GraphicsContext::createNewContextID());
    
    _startTick = osg::Timer::instance()->tick();
    _frameStamp = new osg::FrameStamp;
    _frameStamp->setFrameNumber(0);
    _frameStamp->setReferenceTime(0);
    
    _eventQueue = new osgGA::EventQueue;
    _eventQueue->setStartTick(_startTick);

    _eventVisitor = new osgGA::EventVisitor;
    
    setDatabasePager(new osgDB::DatabasePager);
}

SimpleViewer::~SimpleViewer()
{
    _sceneView->releaseAllGLObjects();
    osg::GraphicsContext::decrementContextIDUsageCount(_sceneView->getState()->getContextID());
}

void SimpleViewer::setSceneData(osg::Node* node)
{
    _sceneView->setSceneData(node);
    
    if (_cameraManipulator.valid())
    {
        _cameraManipulator->setNode(node);
        
        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();
        _cameraManipulator->home(*dummyEvent, *this);
    }

    if (_databasePager.valid())
    {    
        // register any PagedLOD that need to be tracked in the scene graph
        _databasePager->registerPagedLODs(node);
    }
}

osg::Node* SimpleViewer::getSceneData()
{
    return _sceneView->getSceneData();
}

const osg::Node* SimpleViewer::getSceneData() const
{
    return _sceneView->getSceneData();
}

osg::CameraNode* SimpleViewer::getCamera()
{
    return _sceneView->getCamera();
}

const osg::CameraNode* SimpleViewer::getCamera() const
{
    return _sceneView->getCamera();
}

void SimpleViewer::setCameraManipulator(MatrixManipulator* manipulator)
{
    if (_cameraManipulator == manipulator) return;
    
    _cameraManipulator = manipulator;
    if (_cameraManipulator.valid())
    {
        _cameraManipulator->setNode(_sceneView->getSceneData());
        
        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();
        _cameraManipulator->home(*dummyEvent, *this);
    }
}

void SimpleViewer::addEventHandler(GUIEventHandler* eventHandler)
{
    _eventHandlers.push_back(eventHandler);
}

void SimpleViewer::setDatabasePager(osgDB::DatabasePager* dp)
{
    _databasePager = dp;
    
    if (dp && _sceneView.valid())
    {
        // need to register the DatabasePager with the SceneView's CullVisitor so it can pass on request
        // for files to be loaded.
        _sceneView->getCullVisitor()->setDatabaseRequestHandler(_databasePager.get());
    }
}

void SimpleViewer::init()
{
    osg::ref_ptr<osgGA::GUIEventAdapter> initEvent = _eventQueue->createEvent();
    initEvent->setEventType(osgGA::GUIEventAdapter::FRAME);
    _cameraManipulator->init(*initEvent, *this);
}

void SimpleViewer::frame()
{
    if (_firstFrame)
    {
        init();
        _firstFrame = false;
    }

    frameAdvance();
    frameEventTraversal();
    frameUpdateTraversal();
    frameCullTraversal();
    frameDrawTraversal();
}

void SimpleViewer::frameAdvance()
{
    osg::Timer_t currentTick  = osg::Timer::instance()->tick();
    _frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(_startTick,currentTick));
    _frameStamp->setFrameNumber(_frameStamp->getFrameNumber()+1);

    _sceneView->setFrameStamp(_frameStamp.get());
}

void SimpleViewer::frameEventTraversal()
{
    _eventQueue->frame( _frameStamp->getReferenceTime() );

    osgGA::EventQueue::Events events;
    _eventQueue->takeEvents(events);
    
    if (_eventVisitor.valid())
    {
        _eventVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }
    
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
    
        bool handled = false;
        
        if (_eventVisitor.valid())
        {
            _eventVisitor->reset();
            _eventVisitor->addEvent( event );
            
            getSceneData()->accept(*_eventVisitor);
            
            if (_eventVisitor->getEventHandled())  handled = true;
        }
        
        if (_cameraManipulator.valid())
        {
            _cameraManipulator->handle( *event, *this );
        }
        
        for(EventHandlers::iterator hitr = _eventHandlers.begin();
            hitr != _eventHandlers.end() && !handled;
            ++hitr)
        {
            handled = (*hitr)->handle( *event, *this, 0, 0);
        }
    }
}

void SimpleViewer::frameUpdateTraversal()
{
    double previousAspectRatio = ( static_cast<double>(_sceneView->getViewport()->width())/
                                   static_cast<double>(_sceneView->getViewport()->height()) );

    // update the viewport
    int width = _eventQueue->getCurrentEventState()->getWindowWidth();
    int height = _eventQueue->getCurrentEventState()->getWindowHeight();
    _sceneView->setViewport(0,0,width,height);

    double newAspectRatio = ( static_cast<double>(_sceneView->getViewport()->width())/
                              static_cast<double>(_sceneView->getViewport()->height()) );
                              
                              
    // if aspect ratio adjusted change the project matrix to suit.
    if (previousAspectRatio != newAspectRatio)
    {
        osg::Matrixd& pm = _sceneView->getProjectionMatrix();
        bool orthographicCamera = (pm(0,3)==0.0) && (pm(0,3)==0.0) && (pm(0,3)==0.0) && (pm(0,3)==1.0); 
        if (orthographicCamera)
        {
            double left, right, bottom, top, zNear, zFar;
            _sceneView->getProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar);

            double mid = (right+left)*0.5;
            double halfWidth = (right-left)*0.5;
            left = mid - halfWidth * (newAspectRatio/previousAspectRatio);
            right = mid + halfWidth * (newAspectRatio/previousAspectRatio);
            _sceneView->setProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar);
        }
        else
        {
            double left, right, bottom, top, zNear, zFar;
            _sceneView->getProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);

            double mid = (right+left)*0.5;
            double halfWidth = (right-left)*0.5;
            left = mid - halfWidth * (newAspectRatio/previousAspectRatio);
            right = mid + halfWidth * (newAspectRatio/previousAspectRatio);
            _sceneView->setProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);
        }
    }    
    
    if (_cameraManipulator.valid())
    {
        _sceneView->setViewMatrix(_cameraManipulator->getInverseMatrix());
    }

    if (_databasePager.valid())
    {    
        // tell the DatabasePager the frame number of that the scene graph is being actively used to render a frame
        _databasePager->signalBeginFrame(_frameStamp.get());

        // syncronize changes required by the DatabasePager thread to the scene graph
        _databasePager->updateSceneGraph(_frameStamp->getReferenceTime());
    }
    
    _sceneView->update();
}

void SimpleViewer::frameCullTraversal()
{
    _sceneView->cull();
}

void SimpleViewer::frameDrawTraversal()
{
    _sceneView->draw();

    if (_databasePager.valid())
    {    
        // tell the DatabasePager the frame number of that the scene graph is being actively used to render a frame
        _databasePager->signalEndFrame();

        // clean up  and compile gl objects with a specified limit       
        double availableTime = 0.0025; // 2.5 ms

        // compile any GL objects that are required.
        _databasePager->compileGLObjects(*(_sceneView->getState()),availableTime);

        // flush deleted GL objects.
        _sceneView->flushDeletedGLObjects(availableTime);
    }
}

void SimpleViewer::cleanup()
{
    if (_databasePager.valid())
    {    
        // clear the database pager so its starts a fresh on the next update/cull/draw traversals    
        _databasePager->clear();
        
        // release the GL objects stored in the scene graph.
        _sceneView->releaseAllGLObjects();
        
        // do a flush to delete all the OpenGL objects that have been deleted or released from the scene graph.
        _sceneView->flushAllDeletedGLObjects();
    }
    
    _sceneView->releaseAllGLObjects();
    _sceneView->flushAllDeletedGLObjects();
}
