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

#include <osgGA/EventQueue>
#include <osg/Notify>

using namespace osgGA;

EventQueue::EventQueue(GUIEventAdapter::MouseYOrientation mouseYOrientation)
{
    _useFixedMouseInputRange = false;

    _startTick = osg::Timer::instance()->getStartTick();

    _accumulateEventState = new GUIEventAdapter();
    _accumulateEventState->setMouseYOrientation(mouseYOrientation);
}

EventQueue::~EventQueue()
{
}

void EventQueue::setEvents(Events& events)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    _eventQueue = events;
}

void EventQueue::appendEvents(Events& events)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    _eventQueue.insert(_eventQueue.end(), events.begin(), events.end());
}

void EventQueue::addEvent(GUIEventAdapter* event)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    _eventQueue.push_back(event);
}

bool EventQueue::takeEvents(Events& events)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    if (!_eventQueue.empty())
    {
        events.insert(events.end(),_eventQueue.begin(),_eventQueue.end());
        _eventQueue.clear();
        return true;
    }
    else
    {
        return false;
    }
}

bool EventQueue::copyEvents(Events& events) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    if (!_eventQueue.empty())
    {
        events.insert(events.end(),_eventQueue.begin(),_eventQueue.end());
        return true;
    }
    else
    {
        return false;
    }
}


void EventQueue::windowResize(int x, int y, int width, int height, double time)
{
    _accumulateEventState->setWindowRectangle(x, y, width, height, !_useFixedMouseInputRange);

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::RESIZE);
    event->setTime(time);

    addEvent(event);
}

void EventQueue::penPressure(float pressure, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::PEN_PRESSURE);
    event->setPenPressure(pressure);
    event->setTime(time);
    
    addEvent(event);
}

void EventQueue::penOrientation(float tiltX, float tiltY, float rotation, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::PEN_ORIENTATION);
    event->setPenTiltX(tiltX);
    event->setPenTiltY(tiltY);
    event->setPenRotation(rotation);
    event->setTime(time);
    
    addEvent(event);
}

void EventQueue::penProximity(GUIEventAdapter::TabletPointerType pt, bool isEntering, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType( (isEntering) ? GUIEventAdapter::PEN_PROXIMITY_ENTER : GUIEventAdapter::PEN_PROXIMITY_LEAVE);
    event->setTabletPointerType(pt);
    event->setTime(time);
    
    addEvent(event);
}

void EventQueue::mouseScroll(GUIEventAdapter::ScrollingMotion sm, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::SCROLL);
    event->setScrollingMotion(sm);
    event->setTime(time);
    
    addEvent(event);
}

void EventQueue::mouseScroll2D(float x, float y, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::SCROLL);
    event->setScrollingMotionDelta(x,y);
    event->setTime(time);
    
    addEvent(event);
}


void EventQueue::mouseWarped(float x, float y)
{
    _accumulateEventState->setX(x);
    _accumulateEventState->setY(y);
}


void EventQueue::mouseMotion(float x, float y, double time)
{
    _accumulateEventState->setX(x);
    _accumulateEventState->setY(y);

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(event->getButtonMask() ? GUIEventAdapter::DRAG : GUIEventAdapter::MOVE);
    event->setTime(time);
    
    addEvent(event);
}

void EventQueue::mouseButtonPress(float x, float y, unsigned int button, double time)
{
    _accumulateEventState->setX(x);
    _accumulateEventState->setY(y);

    switch(button)
    {
        case(1): 
            _accumulateEventState->setButtonMask(GUIEventAdapter::LEFT_MOUSE_BUTTON | _accumulateEventState->getButtonMask());
            break;
        case(2): 
            _accumulateEventState->setButtonMask(GUIEventAdapter::MIDDLE_MOUSE_BUTTON | _accumulateEventState->getButtonMask());
            break;
        case(3): 
            _accumulateEventState->setButtonMask(GUIEventAdapter::RIGHT_MOUSE_BUTTON | _accumulateEventState->getButtonMask());
            break;
    }

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::PUSH);
    event->setTime(time);

    switch(button)
    {
        case(1): 
            event->setButton(GUIEventAdapter::LEFT_MOUSE_BUTTON);
            break;
        case(2): 
            event->setButton(GUIEventAdapter::MIDDLE_MOUSE_BUTTON);
            break;
        case(3): 
            event->setButton(GUIEventAdapter::RIGHT_MOUSE_BUTTON);
            break;
    }
    
    addEvent(event);
}

void EventQueue::mouseDoubleButtonPress(float x, float y, unsigned int button, double time)
{
    _accumulateEventState->setX(x);
    _accumulateEventState->setY(y);

    switch(button)
    {
        case(1): 
            _accumulateEventState->setButtonMask(GUIEventAdapter::LEFT_MOUSE_BUTTON | _accumulateEventState->getButtonMask());
            break;
        case(2): 
            _accumulateEventState->setButtonMask(GUIEventAdapter::MIDDLE_MOUSE_BUTTON | _accumulateEventState->getButtonMask());
            break;
        case(3): 
            _accumulateEventState->setButtonMask(GUIEventAdapter::RIGHT_MOUSE_BUTTON | _accumulateEventState->getButtonMask());
            break;
    }

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::DOUBLECLICK);
    event->setTime(time);

    switch(button)
    {
        case(1): 
            event->setButton(GUIEventAdapter::LEFT_MOUSE_BUTTON);
            break;
        case(2): 
            event->setButton(GUIEventAdapter::MIDDLE_MOUSE_BUTTON);
            break;
        case(3): 
            event->setButton(GUIEventAdapter::RIGHT_MOUSE_BUTTON);
            break;
    }
    
    addEvent(event);
}

void EventQueue::mouseButtonRelease(float x, float y, unsigned int button, double time)
{
    _accumulateEventState->setX(x);
    _accumulateEventState->setY(y);

    switch(button)
    {
        case(1): 
            _accumulateEventState->setButtonMask(~GUIEventAdapter::LEFT_MOUSE_BUTTON & _accumulateEventState->getButtonMask());
            break;
        case(2): 
            _accumulateEventState->setButtonMask(~GUIEventAdapter::MIDDLE_MOUSE_BUTTON & _accumulateEventState->getButtonMask());
            break;
        case(3): 
            _accumulateEventState->setButtonMask(~GUIEventAdapter::RIGHT_MOUSE_BUTTON & _accumulateEventState->getButtonMask());
            break;
    }

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::RELEASE);
    event->setTime(time);

    switch(button)
    {
        case(1): 
            event->setButton(GUIEventAdapter::LEFT_MOUSE_BUTTON);
            break;
        case(2): 
            event->setButton(GUIEventAdapter::MIDDLE_MOUSE_BUTTON);
            break;
        case(3): 
            event->setButton(GUIEventAdapter::RIGHT_MOUSE_BUTTON);
            break;
    }
    
    addEvent(event);
}

void EventQueue::keyPress(int key, double time)
{
    switch(key)
    {
        case(GUIEventAdapter::KEY_Shift_L):      _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_LEFT_SHIFT | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Shift_R):      _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_RIGHT_SHIFT | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Control_L):    _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_LEFT_CTRL | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Control_R):    _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_RIGHT_CTRL | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Meta_L):       _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_LEFT_META | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Meta_R):       _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_RIGHT_META | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Alt_L):        _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_LEFT_ALT | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Alt_R):        _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_RIGHT_ALT | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Super_L):      _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_LEFT_SUPER | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Super_R):      _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_RIGHT_SUPER | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Hyper_L):      _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_LEFT_HYPER | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Hyper_R):      _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_RIGHT_HYPER | _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Caps_Lock):
        {
            if ((_accumulateEventState->getModKeyMask() & GUIEventAdapter::MODKEY_CAPS_LOCK)!=0) 
                _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_CAPS_LOCK & _accumulateEventState->getModKeyMask());
            else 
                _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_CAPS_LOCK | _accumulateEventState->getModKeyMask()); 
            break;
        }
        case(GUIEventAdapter::KEY_Num_Lock):
        {
            if ((_accumulateEventState->getModKeyMask() & GUIEventAdapter::MODKEY_NUM_LOCK)!=0)
                 _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_NUM_LOCK & _accumulateEventState->getModKeyMask());
            else
                 _accumulateEventState->setModKeyMask(GUIEventAdapter::MODKEY_NUM_LOCK | _accumulateEventState->getModKeyMask());
            break;
        }
        default: break;
    }        

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::KEYDOWN);
    event->setKey(key);
    event->setTime(time);
    
    addEvent(event);
}

void EventQueue::keyRelease(int key, double time)
{
    switch(key)
    {
        case(GUIEventAdapter::KEY_Shift_L):      _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_LEFT_SHIFT & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Shift_R):      _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_RIGHT_SHIFT & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Control_L):    _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_LEFT_CTRL & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Control_R):    _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_RIGHT_CTRL & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Meta_L):       _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_LEFT_META & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Meta_R):       _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_RIGHT_META & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Alt_L):        _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_LEFT_ALT & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Alt_R):        _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_RIGHT_ALT & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Super_L):      _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_LEFT_SUPER & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Super_R):      _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_RIGHT_SUPER & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Hyper_L):      _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_LEFT_HYPER & _accumulateEventState->getModKeyMask()); break;
        case(GUIEventAdapter::KEY_Hyper_R):      _accumulateEventState->setModKeyMask(~GUIEventAdapter::MODKEY_RIGHT_HYPER & _accumulateEventState->getModKeyMask()); break;
        default: break;
    }        

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::KEYUP);
    event->setKey(key);
    event->setTime(time);
    
    addEvent(event);
}

void EventQueue::closeWindow(double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::CLOSE_WINDOW);
    event->setTime(time);
  
    addEvent(event);
}

void EventQueue::quitApplication(double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::QUIT_APPLICATION);
    event->setTime(time);
  
    addEvent(event);
}


void EventQueue::frame(double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::FRAME);
    event->setTime(time);
    
    addEvent(event);
}

GUIEventAdapter* EventQueue::createEvent()
{
    if (_accumulateEventState.valid()) return new GUIEventAdapter(*_accumulateEventState.get()); 
    else return new GUIEventAdapter();    
}

void EventQueue::userEvent(osg::Referenced* userEventData, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::USER);
    event->setUserData(userEventData);
    event->setTime(time);
    
    addEvent(event);
}


