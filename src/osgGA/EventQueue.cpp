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

    _firstTouchEmulatesMouse = true;
}

EventQueue::~EventQueue()
{
}

void EventQueue::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    _eventQueue.clear();
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

void EventQueue::addEvent(Event* event)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    _eventQueue.push_back(event);
}

bool EventQueue::takeEvents(Events& events)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    if (!_eventQueue.empty())
    {
        events.splice(events.end(), _eventQueue);
        return true;
    }
    else
    {
        return false;
    }
}

bool EventQueue::takeEvents(Events& events, double cutOffTime)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
    if (!_eventQueue.empty())
    {
        // find last event if queue that came in before the cuttof.
        Events::reverse_iterator ritr = _eventQueue.rbegin();
        for(; ritr != _eventQueue.rend() && ((*ritr)->getTime() > cutOffTime); ++ritr) {}

        if (ritr==_eventQueue.rend()) return false;

        for(Events::iterator itr = _eventQueue.begin();
            itr != ritr.base();
            ++itr)
        {
            events.push_back(*itr);
        }

        // make sure that the events are in ascending time order, and any out of out events
        // have their time reset to the next valid time after them in the events list.
        double previousTime = cutOffTime;
        for(Events::reverse_iterator itr = events.rbegin();
            itr != events.rend();
            ++itr)
        {
            if ((*itr)->getTime() > previousTime)
            {
                OSG_INFO<<"Reset event time from "<<(*itr)->getTime()<<" to "<<previousTime<<std::endl;
                (*itr)->setTime(previousTime);
            }
            else
            {
                previousTime = (*itr)->getTime();
            }
        }

        // remove the events we are taking from the original event queue.
        _eventQueue.erase(_eventQueue.begin(), ritr.base());

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

void EventQueue::syncWindowRectangleWithGraphicsContext()
{
    const osg::GraphicsContext::Traits* traits = (getGraphicsContext()!=0) ? getGraphicsContext()->getTraits() : 0;
    if (traits) _accumulateEventState->setWindowRectangle(traits->x, traits->y, traits->width, traits->height, !_useFixedMouseInputRange);
}

osgGA::GUIEventAdapter* EventQueue::windowResize(int x, int y, int width, int height, double time)
{
    _accumulateEventState->setWindowRectangle(x, y, width, height, !_useFixedMouseInputRange);

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::RESIZE);
    event->setTime(time);

    addEvent(event);
    
    return event;
}

osgGA::GUIEventAdapter* EventQueue::penPressure(float pressure, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::PEN_PRESSURE);
    event->setPenPressure(pressure);
    event->setTime(time);

    addEvent(event);

    return event;
}

osgGA::GUIEventAdapter* EventQueue::penOrientation(float tiltX, float tiltY, float rotation, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::PEN_ORIENTATION);
    event->setPenTiltX(tiltX);
    event->setPenTiltY(tiltY);
    event->setPenRotation(rotation);
    event->setTime(time);

    addEvent(event);

    return event;
}

osgGA::GUIEventAdapter* EventQueue::penProximity(GUIEventAdapter::TabletPointerType pt, bool isEntering, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType( (isEntering) ? GUIEventAdapter::PEN_PROXIMITY_ENTER : GUIEventAdapter::PEN_PROXIMITY_LEAVE);
    event->setTabletPointerType(pt);
    event->setTime(time);

    addEvent(event);

    return event;
}

osgGA::GUIEventAdapter* EventQueue::mouseScroll(GUIEventAdapter::ScrollingMotion sm, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::SCROLL);
    event->setScrollingMotion(sm);
    event->setTime(time);

    addEvent(event);

    return event;
}

osgGA::GUIEventAdapter* EventQueue::mouseScroll2D(float x, float y, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::SCROLL);
    event->setScrollingMotionDelta(x,y);
    event->setTime(time);

    addEvent(event);

    return event;
}


void EventQueue::mouseWarped(float x, float y)
{
    _accumulateEventState->setX(x);
    _accumulateEventState->setY(y);
}


osgGA::GUIEventAdapter* EventQueue::mouseMotion(float x, float y, double time)
{
    _accumulateEventState->setX(x);
    _accumulateEventState->setY(y);

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(event->getButtonMask() ? GUIEventAdapter::DRAG : GUIEventAdapter::MOVE);
    event->setTime(time);

    addEvent(event);

    return event;
}

osgGA::GUIEventAdapter* EventQueue::mouseButtonPress(float x, float y, unsigned int button, double time)
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

    return event;
}

osgGA::GUIEventAdapter* EventQueue::mouseDoubleButtonPress(float x, float y, unsigned int button, double time)
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

    return event;
}

osgGA::GUIEventAdapter* EventQueue::mouseButtonRelease(float x, float y, unsigned int button, double time)
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

    return event;
}

osgGA::GUIEventAdapter* EventQueue::keyPress(int key, double time, int unmodifiedKey)
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
    event->setUnmodifiedKey(unmodifiedKey);
    event->setTime(time);

    addEvent(event);
    
    return event;
}

osgGA::GUIEventAdapter* EventQueue::keyRelease(int key, double time, int unmodifiedKey)
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
    event->setUnmodifiedKey(unmodifiedKey);
    event->setTime(time);

    addEvent(event);

    return event;
}


GUIEventAdapter* EventQueue::touchBegan(unsigned int id, GUIEventAdapter::TouchPhase phase, float x, float y, double time)
{
    if(_firstTouchEmulatesMouse)
    {
        // emulate left mouse button press

        _accumulateEventState->setButtonMask(GUIEventAdapter::LEFT_MOUSE_BUTTON | _accumulateEventState->getButtonMask());
        _accumulateEventState->setX(x);
        _accumulateEventState->setY(y);
    }

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::PUSH);
    event->setTime(time);
    event->addTouchPoint(id, phase, x, y, 0);
    if(_firstTouchEmulatesMouse)
        event->setButton(GUIEventAdapter::LEFT_MOUSE_BUTTON);

    addEvent(event);

    return event;
}


GUIEventAdapter* EventQueue::touchMoved(unsigned int id, GUIEventAdapter::TouchPhase phase, float x, float y, double time)
{
    if(_firstTouchEmulatesMouse)
    {
        _accumulateEventState->setX(x);
        _accumulateEventState->setY(y);
    }

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::DRAG);
    event->setTime(time);
    event->addTouchPoint(id, phase, x, y, 0);
    addEvent(event);

    return event;
}

GUIEventAdapter* EventQueue::touchEnded(unsigned int id, GUIEventAdapter::TouchPhase phase, float x, float y, unsigned int tap_count, double time)
{
    if (_firstTouchEmulatesMouse)
    {
        _accumulateEventState->setButtonMask(~GUIEventAdapter::LEFT_MOUSE_BUTTON & _accumulateEventState->getButtonMask());
        _accumulateEventState->setX(x);
        _accumulateEventState->setY(y);
    }

    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::RELEASE);
    event->setTime(time);
    event->addTouchPoint(id, phase, x, y, tap_count);
    if(_firstTouchEmulatesMouse)
        event->setButton(GUIEventAdapter::LEFT_MOUSE_BUTTON);

    addEvent(event);

    return event;
}


osgGA::GUIEventAdapter* EventQueue::closeWindow(double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::CLOSE_WINDOW);
    event->setTime(time);

    addEvent(event);
    
    return event;
}

osgGA::GUIEventAdapter* EventQueue::quitApplication(double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::QUIT_APPLICATION);
    event->setTime(time);

    addEvent(event);
    
    return event;
}


osgGA::GUIEventAdapter* EventQueue::frame(double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::FRAME);
    event->setTime(time);

    addEvent(event);
    
    return event;
}

GUIEventAdapter* EventQueue::createEvent()
{
    if (_accumulateEventState.valid()) return new GUIEventAdapter(*_accumulateEventState.get());
    else return new GUIEventAdapter();
}

GUIEventAdapter* EventQueue::userEvent(osg::Referenced* userEventData, double time)
{
    GUIEventAdapter* event = new GUIEventAdapter(*_accumulateEventState);
    event->setEventType(GUIEventAdapter::USER);
    event->setUserData(userEventData);
    event->setTime(time);

    addEvent(event);

    return event;
}


