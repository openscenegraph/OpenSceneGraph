/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY
{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgViewer/Viewer>
#include <osgUtil/GLObjectsVisitor>
#include <osg/GLExtensions>

using namespace osgViewer;

class ActionAdapter : public osgGA::GUIActionAdapter
{
public:
        virtual ~ActionAdapter() {}

        virtual void requestRedraw() { /*osg::notify(osg::NOTICE)<<"requestRedraw()"<<std::endl;*/ }
        virtual void requestContinuousUpdate(bool =true) { /*osg::notify(osg::NOTICE)<<"requestContinuousUpdate("<<needed<<")"<<std::endl;*/ }
        virtual void requestWarpPointer(float x,float y) { osg::notify(osg::NOTICE)<<"requestWarpPointer("<<x<<","<<y<<")"<<std::endl; }

};

Viewer::Viewer():
    _firstFrame(true),
    _done(false)
{
}

Viewer::~Viewer()
{
#if 0
    Contexts contexts;
    getContexts(contexts);

    // cancel any graphics threads.
    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osg::GraphicsContext* gc = (*citr);
        gc->setGraphicsThread(0);
    }
    
    if (_scene.valid() && _scene->getDatabasePager())
    {
        _scene->getDatabasePager()->cancel();
        _scene->setDatabasePager(0);
    }
#endif    
}

void Viewer::init()
{
    osg::notify(osg::INFO)<<"Viewer::init()"<<std::endl;
    
    osg::ref_ptr<osgGA::GUIEventAdapter> initEvent = _eventQueue->createEvent();
    initEvent->setEventType(osgGA::GUIEventAdapter::FRAME);
    
    if (_cameraManipulator.valid())
    {
        ActionAdapter aa;
        _cameraManipulator->init(*initEvent, aa);
    }
}

void Viewer::getContexts(Contexts& contexts)
{
    typedef std::set<osg::GraphicsContext*> ContextSet;
    ContextSet contextSet;

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        contextSet.insert(_camera->getGraphicsContext());
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            contextSet.insert(slave._camera->getGraphicsContext());
        }
    }

    contexts.clear();
    contexts.reserve(contextSet.size());

    for(ContextSet::iterator itr = contextSet.begin();
        itr != contextSet.end();
        ++itr)
    {
        contexts.push_back(const_cast<osg::GraphicsContext*>(*itr));
    }
}

OpenThreads::Mutex mutex;

// Compile operation, that compile OpenGL objects.
struct CompileOperation : public osg::GraphicsOperation
{
    CompileOperation(osg::Node* scene):
        osg::GraphicsOperation("Compile",false),
        _scene(scene)
    {
    }
    
    virtual void operator () (osg::GraphicsContext* context)
    {
        // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
        // osg::notify(osg::NOTICE)<<"Compile "<<context<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;

        // context->makeCurrentImplementation();

        osgUtil::GLObjectsVisitor compileVisitor;
        compileVisitor.setState(context->getState());

        // do the compile traversal
        _scene->accept(compileVisitor);

        // osg::notify(osg::NOTICE)<<"Done Compile "<<context<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;
    }
    
    osg::ref_ptr<osg::Node> _scene;
};


// Draw operation, that does a draw on the scene graph.
struct RunOperations : public osg::GraphicsOperation
{
    RunOperations(osg::GraphicsContext* gc):
        osg::GraphicsOperation("RunOperation",true),
        _originalContext(gc)
    {
    }
    
    virtual void operator () (osg::GraphicsContext* gc)
    {
        gc->runOperations();
    }
    
    osg::GraphicsContext* _originalContext;
};


void Viewer::realize()
{
    osg::notify(osg::INFO)<<"Viewer::realize()"<<std::endl;

    Contexts::iterator citr;

    Contexts contexts;
    getContexts(contexts);

    bool multiThreaded = getNumSlaves() > 1;
    
    if (multiThreaded)
    {
        _startRenderingBarrier = new osg::BarrierOperation(contexts.size()+1, osg::BarrierOperation::NO_OPERATION);
        _endRenderingDispatchBarrier = new osg::BarrierOperation(contexts.size()+1, osg::BarrierOperation::NO_OPERATION);
        osg::ref_ptr<osg::SwapBuffersOperation> swapOp = new osg::SwapBuffersOperation();

        for(citr = contexts.begin();
            citr != contexts.end();
            ++citr)
        {
            osg::GraphicsContext* gc = (*citr);
                        
            // create the a graphics thread for this context
            gc->createGraphicsThread();
            
            gc->getGraphicsThread()->add(new CompileOperation(getSceneData()));
            
            // add the startRenderingBarrier
            gc->getGraphicsThread()->add(_startRenderingBarrier.get());

            // add the rendering operation itself.
            gc->getGraphicsThread()->add(new RunOperations(gc));
            
            // add the endRenderingDispatchBarrier
            gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());

            // add the swap buffers
            gc->getGraphicsThread()->add(swapOp.get());

        }
        
    }

    for(citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        (*citr)->realize();
        OpenThreads::Thread::YieldCurrentThread();
    }
    
    bool grabFocus = true;
    if (grabFocus)
    {
        for(citr = contexts.begin();
            citr != contexts.end();
            ++citr)
        {
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
            if (gw)
            {
                gw->grabFocusIfPointerInWindow();    
            }
        }
    }            

    // initialize the global timer to be relative to the current time.
    osg::Timer::instance()->setStartTick();


    if (multiThreaded)
    {
        for(citr = contexts.begin();
            citr != contexts.end();
            ++citr)
        {
            osg::GraphicsContext* gc = (*citr);
            if (!gc->getGraphicsThread()->isRunning())
            {
                gc->getGraphicsThread()->startThread();
                OpenThreads::Thread::YieldCurrentThread();
            }
        }
    }

}


void Viewer::frame()
{
    if (_done) return;

    if (_firstFrame)
    {
        init();
        _firstFrame = false;
    }
    frameAdvance();
    
    frameEventTraversal();
    frameUpdateTraversal();
    frameRenderingTraversals();
}

void Viewer::frameAdvance()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<"Viewer::frameAdvance()."<<std::endl;

    _scene->frameAdvance();
}

void Viewer::frameEventTraversal()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<"Viewer::frameEventTraversal()."<<std::endl;
    
    // need to copy events from the GraphicsWindow's into local EventQueue;
    osgGA::EventQueue::Events events;

    Contexts contexts;
    getContexts(contexts);

    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
        if (gw)
        {
            gw->checkEvents();
            gw->getEventQueue()->takeEvents(events);
        }
    }
    
    osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState(); 
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
        event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());

        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
            case(osgGA::GUIEventAdapter::RELEASE):
            case(osgGA::GUIEventAdapter::DRAG):
            case(osgGA::GUIEventAdapter::MOVE):
                eventState->setX(event->getX());
                eventState->setY(event->getY());
                eventState->setButtonMask(event->getButtonMask());
                break;
            default:
                break;
        }
    }

    _eventQueue->frame( _scene->getFrameStamp()->getReferenceTime() );

    _eventQueue->takeEvents(events);


#if 0
    // osg::notify(osg::NOTICE)<<"Events "<<events.size()<<std::endl;
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
                osg::notify(osg::NOTICE)<<"  PUSH "<<event->getButton()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::RELEASE):
                osg::notify(osg::NOTICE)<<"  RELEASE "<<event->getButton()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::DRAG):
                osg::notify(osg::NOTICE)<<"  DRAG "<<event->getButtonMask()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::MOVE):
                osg::notify(osg::NOTICE)<<"  MOVE "<<event->getButtonMask()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::SCROLL):
                osg::notify(osg::NOTICE)<<"  SCROLL "<<event->getScrollingMotion()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::KEYDOWN):
                osg::notify(osg::NOTICE)<<"  KEYDOWN '"<<(char)event->getKey()<<"'"<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::KEYUP):
                osg::notify(osg::NOTICE)<<"  KEYUP '"<<(char)event->getKey()<<"'"<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::FRAME):
                // osg::notify(osg::NOTICE)<<"  FRAME "<<std::endl;
                break;
            default:
                // osg::notify(osg::NOTICE)<<"  Event not handled"<<std::endl;
                break;
        }
    }
#endif

    // osg::notify(osg::NOTICE)<<"Events "<<events.size()<<std::endl;
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
                if (event->getKey()=='q') _done = true;
                break;
            default:
                break;
        }
    }
    
    if (_done) return;

    ActionAdapter aa;

    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();

        bool handled = false;
        
        if (_cameraManipulator.valid())
        {
            _cameraManipulator->handle( *event, aa);
        }
        
        for(EventHandlers::iterator hitr = _eventHandlers.begin();
            hitr != _eventHandlers.end() && !handled;
            ++hitr)
        {
            handled = (*hitr)->handle( *event, aa, 0, 0);
        }
    }
}

void Viewer::frameUpdateTraversal()
{
    if (_done) return;

    if (_scene.valid()) _scene->frameUpdateTraversal();

    if (_cameraManipulator.valid())
    {
        _camera->setViewMatrix(_cameraManipulator->getInverseMatrix());
    }

    updateSlaves();
}

void Viewer::frameRenderingTraversals()
{
    if (_done) return;

    osgDB::DatabasePager* dp = _scene->getDatabasePager();
    if (dp)
    {
        dp->signalBeginFrame(_scene->getFrameStamp());
    }

    bool multiThreaded = _startRenderingBarrier.valid();
    
    if (multiThreaded)
    {
        // sleep(1);
    
        // osg::notify(osg::NOTICE)<<std::endl<<"Joing _startRenderingBarrier block"<<std::endl;
    
        // dispatch the the rendering threads
        _startRenderingBarrier->block();
        
        // osg::notify(osg::NOTICE)<<"Joing _endRenderingDispatchBarrier block"<<std::endl;

        // wait till the rendering dispatch is done.
        _endRenderingDispatchBarrier->block();

        // osg::notify(osg::NOTICE)<<"Leaving _endRenderingDispatchBarrier block"<<std::endl<<std::endl;

    }
    else
    {
        Contexts contexts;
        getContexts(contexts);

        Contexts::iterator itr;
        for(itr = contexts.begin();
            itr != contexts.end();
            ++itr)
        {
            if (_done) return;
            (*itr)->makeCurrent();
            (*itr)->runOperations();
        }


        for(itr = contexts.begin();
            itr != contexts.end();
            ++itr)
        {
            (*itr)->makeCurrent();
            (*itr)->swapBuffers();
        }

    }

    if (dp)
    {
        dp->signalEndFrame();
    }
}

void Viewer::releaseAllGLObjects()
{
    osg::notify(osg::NOTICE)<<"Viewer::releaseAllGLObjects() not implemented yet."<<std::endl;
}

void Viewer::cleanup()
{
    osg::notify(osg::NOTICE)<<"Viewer::cleanup() not implemented yet."<<std::endl;
}

