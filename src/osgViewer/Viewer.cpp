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

using namespace osgViewer;

class ActionAdapter : public osgGA::GUIActionAdapter
{
public:
        virtual ~ActionAdapter() {}

        virtual void requestRedraw() { /*osg::notify(osg::NOTICE)<<"requestRedraw()"<<std::endl;*/ }
        virtual void requestContinuousUpdate(bool needed=true) { /*osg::notify(osg::NOTICE)<<"requestContinuousUpdate("<<needed<<")"<<std::endl;*/ }
        virtual void requestWarpPointer(float x,float y) { osg::notify(osg::NOTICE)<<"requestWarpPointer("<<x<<","<<y<<")"<<std::endl; }

};

Viewer::Viewer():
    _firstFrame(true),
    _done(false)
{
}

Viewer::~Viewer()
{
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

void Viewer::realize()
{
    osg::notify(osg::INFO)<<"Viewer::realize()"<<std::endl;

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        _camera->getGraphicsContext()->realize();
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osg::notify(osg::INFO)<<"  slave realize()"<<std::endl;
            slave._camera->getGraphicsContext()->realize();
        }
    }

    bool grabFocus = true;
    if (grabFocus)
    {
        if (_camera.valid() && _camera->getGraphicsContext())
        {
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());
            gw->grabFocusIfPointerInWindow();
        }

        for(unsigned int i=0; i<getNumSlaves(); ++i)
        {
            Slave& slave = getSlave(i);
            if (slave._camera.valid() && slave._camera->getGraphicsContext())
            {
                osg::notify(osg::INFO)<<"  slave realize()"<<std::endl;
                slave._camera->getGraphicsContext()->realize();
                osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(slave._camera->getGraphicsContext());
                gw->grabFocusIfPointerInWindow();
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

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());
        if (gw)
        {
            gw->checkEvents();
            gw->getEventQueue()->takeEvents(events);
        }
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(slave._camera->getGraphicsContext());
            if (gw)
            {
                gw->checkEvents();
                gw->getEventQueue()->takeEvents(events);
            }
        }
    }
    
    _eventQueue->frame( _scene->getFrameStamp()->getReferenceTime() );

    _eventQueue->takeEvents(events);

    osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState(); 
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
        event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
    }

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

   typedef std::set<osg::GraphicsContext*> GraphicsContexts;
   GraphicsContexts contexts;

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());
        if (gw)
        {
            contexts.insert(gw);
        }
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(slave._camera->getGraphicsContext());
            if (gw)
            {
                contexts.insert(gw);
            }
        }
    }
    
    for(GraphicsContexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        if (_done) return;
        const_cast<osg::GraphicsContext*>(*itr)->makeCurrent();
        const_cast<osg::GraphicsContext*>(*itr)->runOperations();
    }
   
    for(GraphicsContexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        const_cast<osg::GraphicsContext*>(*itr)->swapBuffers();
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

