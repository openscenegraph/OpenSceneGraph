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

#ifndef OSGGA_EVENTQUEUE
#define OSGGA_EVENTQUEUE 1

#include <osgGA/GUIEventAdapter>

#include <osg/ref_ptr>
#include <osg/Timer>

#include <OpenThreads/Mutex>
#include <list>

namespace osgGA {

/**
 * EventQueue implementation for collecting and adapting windowing events
 */
class OSGGA_EXPORT EventQueue : public osg::Referenced
{
    public:

        EventQueue(GUIEventAdapter::MouseYOrientation mouseYOrientation=GUIEventAdapter::Y_INCREASING_DOWNWARDS);

        typedef std::list< osg::ref_ptr<Event> > Events;

        bool empty() const
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_eventQueueMutex);
            return _eventQueue.empty();
        }

        /** Set events.*/
        void setEvents(Events& events);

        /** Take the entire event queue leaving the EventQueue' event queue empty.*/
        bool takeEvents(Events& events);

        /** Take the events that were recorded before with specified time queue.*/
        bool takeEvents(Events& events, double cutOffTime);

        /** Take a copy the entire event queue leaving the EventQueue' event queue intact.*/
        bool copyEvents(Events& events) const;

        /** Add events to end of event queue.*/
        void appendEvents(Events& events);

        /** Add an event to the end of the event queue.*/
        void addEvent(Event* event);


        /** Specify if mouse coordinates should be transformed into a pre defined input range, or whether they
          * should be simply based on as local coordinates to the window that generated the mouse events.*/
        void setUseFixedMouseInputRange(bool useFixedMouseInputRange) { _useFixedMouseInputRange = useFixedMouseInputRange; }

        /** Get whether the mouse coordinates should be transformed into a pre defined input range.*/
        bool getUseFixedMouseInputRange() { return _useFixedMouseInputRange; }


        /** Set the graphics context associated with this event queue.*/
        void setGraphicsContext(osg::GraphicsContext* context) { getCurrentEventState()->setGraphicsContext(context); }

        osg::GraphicsContext* getGraphicsContext() { return getCurrentEventState()->getGraphicsContext(); }

        const osg::GraphicsContext* getGraphicsContext() const { return getCurrentEventState()->getGraphicsContext(); }

        /** Read the window record dimensions from the graphics context. */
        void syncWindowRectangleWithGraphicsContext();


        /** Set the mouse input range.*/
        void setMouseInputRange(float xMin, float yMin, float xMax, float yMax) { getCurrentEventState()->setInputRange(xMin, yMin, xMax, yMax); }


        /** Method for adapting window resize event, placing this event on the back of the event queue. */
        osgGA::GUIEventAdapter* windowResize(int x, int y, int width, int height) { return windowResize(x,y,width,height,getTime()); }

        /** Method for adapting window resize event, placing this event on the back of the event queue, with specified time. */
        osgGA::GUIEventAdapter* windowResize(int x, int y, int width, int height, double time);


        /** Method for adapting mouse scroll wheel events, placing this event on the back of the event queue. */
        osgGA::GUIEventAdapter* mouseScroll(GUIEventAdapter::ScrollingMotion sm) { return mouseScroll(sm,getTime()); }

        /** Method for adapting mouse scroll wheel events, placing this event on the back of the event queue, with specified time. */
        osgGA::GUIEventAdapter* mouseScroll(GUIEventAdapter::ScrollingMotion sm, double time);


        /** Method for adapting mouse scroll wheel events, placing this event on the back of the event queue. */
        osgGA::GUIEventAdapter* mouseScroll2D(float x, float y) { return mouseScroll2D(x, y, getTime()); }

        /** Method for adapting mouse scroll wheel events, placing this event on the back of the event queue. */
        osgGA::GUIEventAdapter* mouseScroll2D(float x, float y, double time);


        /** Method for adapting pen pressure events, placing this event on the back of the event queue.*/
        osgGA::GUIEventAdapter* penPressure(float pressure) { return penPressure(pressure, getTime()); }

        /** Method for adapting pen pressure events, placing this event on the back of the event queue, with specified time.*/
        osgGA::GUIEventAdapter* penPressure(float pressure, double time);

        /** Method for adapting pen orientation events, placing this event on the back of the event queue.*/
        osgGA::GUIEventAdapter* penOrientation(float tiltX, float tiltY, float rotation) { return penOrientation(tiltX, tiltY, rotation, getTime()); }

        /** Method for adapting pen orientation events, placing this event on the back of the event queue, with specified time.*/
        osgGA::GUIEventAdapter* penOrientation(float tiltX, float tiltY, float rotation, double time);

         /** Method for adapting pen proximity events, placing this event on the back of the event queue.*/
        osgGA::GUIEventAdapter* penProximity(GUIEventAdapter::TabletPointerType pt, bool isEntering) { return penProximity(pt, isEntering, getTime()); }

         /** Method for adapting pen proximity events, placing this event on the back of the event queue, with specified time.*/
        osgGA::GUIEventAdapter* penProximity(GUIEventAdapter::TabletPointerType pt, bool isEntering, double time);


        /** Method for updating in response to a mouse warp. Note, just moves the mouse position without creating a new event for it.*/
        void mouseWarped(float x, float y);


        /** Method for adapting mouse motion events, placing this event on the back of the event queue.*/
        osgGA::GUIEventAdapter* mouseMotion(float x, float y) { return mouseMotion(x,y, getTime()); }

        /** Method for adapting mouse motion events, placing this event on the back of the event queue, with specified time.*/
        osgGA::GUIEventAdapter* mouseMotion(float x, float y, double time);


        /** Method for adapting mouse button pressed events, placing this event on the back of the event queue.
          * Button numbering is 1 for left mouse button, 2 for middle, 3 for right. */
        osgGA::GUIEventAdapter* mouseButtonPress(float x, float y, unsigned int button) { return mouseButtonPress(x, y, button, getTime()); }

        /** Method for adapting mouse button pressed events, placing this event on the back of the event queue, with specified time.
          * Button numbering is 1 for left mouse button, 2 for middle, 3 for right. */
        osgGA::GUIEventAdapter* mouseButtonPress(float x, float y, unsigned int button, double time);


        /** Method for adapting mouse button pressed events, placing this event on the back of the event queue.
          * Button numbering is 1 for left mouse button, 2 for middle, 3 for right. */
        osgGA::GUIEventAdapter* mouseDoubleButtonPress(float x, float y, unsigned int button) { return mouseDoubleButtonPress(x, y, button, getTime()); }

        /** Method for adapting mouse button pressed events, placing this event on the back of the event queue, with specified time.
          * Button numbering is 1 for left mouse button, 2 for middle, 3 for right. */
        osgGA::GUIEventAdapter* mouseDoubleButtonPress(float x, float y, unsigned int button, double time);


        /** Method for adapting mouse button release events, placing this event on the back of the event queue.
          * Button numbering is 1 for left mouse button, 2 for middle, 3 for right. */
        osgGA::GUIEventAdapter* mouseButtonRelease(float x, float y, unsigned int button) { return mouseButtonRelease(x, y, button, getTime()); }

        /** Method for adapting mouse button release events, placing this event on the back of the event queue, with specified time.
          * Button numbering is 1 for left mouse button, 2 for middle, 3 for right. */
        osgGA::GUIEventAdapter* mouseButtonRelease(float x, float y, unsigned int button, double time);


        /** Method for adapting keyboard press events. Note, special keys such as Ctrl/Function keys should be adapted to GUIEventAdapter::KeySymbol mappings.*/
        osgGA::GUIEventAdapter* keyPress(int key, int unmodifiedKey = 0) { return keyPress(key, getTime(), unmodifiedKey); }

        /** Method for adapting keyboard press events. Note, special keys such as Ctrl/Function keys should be adapted to GUIEventAdapter::KeySymbol mappings, with specified time.*/
        osgGA::GUIEventAdapter* keyPress(int key, double time, int unmodifiedKey = 0);


        /** Method for adapting keyboard press events. Note, special keys such as Ctrl/Function keys should be adapted to GUIEventAdapter::KeySymbol mappings.*/
        osgGA::GUIEventAdapter* keyRelease(int key, int unmodifiedKey = 0) { return keyRelease(key, getTime(), unmodifiedKey); }

        /** Method for adapting keyboard press events. Note, special keys such as Ctrl/Function keys should be adapted to GUIEventAdapter::KeySymbol mappings, with specified time.*/
        osgGA::GUIEventAdapter* keyRelease(int key, double time, int unmodifiedKey = 0);

        GUIEventAdapter* touchBegan(unsigned int id, GUIEventAdapter::TouchPhase phase, float x, float y, double time);
        GUIEventAdapter*  touchBegan(unsigned int id, GUIEventAdapter::TouchPhase phase,  float x, float y) {
            return touchBegan(id, phase, x, y, getTime());
        }

        GUIEventAdapter*  touchMoved(unsigned int id, GUIEventAdapter::TouchPhase phase,  float x, float y, double time);
        GUIEventAdapter*  touchMoved(unsigned int id, GUIEventAdapter::TouchPhase phase,  float x, float y) {
            return touchMoved(id, phase, x, y, getTime());
        }

        GUIEventAdapter*  touchEnded(unsigned int id, GUIEventAdapter::TouchPhase phase,  float x, float y, unsigned int tap_count, double time);
        GUIEventAdapter*  touchEnded(unsigned int id, GUIEventAdapter::TouchPhase phase,  float x, float y, unsigned int tap_count) {
            return touchEnded(id, phase, x, y, tap_count, getTime());
        }



        /** Method for adapting close window events.*/
        osgGA::GUIEventAdapter* closeWindow() { return closeWindow(getTime()); }

        /** Method for adapting close window event with specified event time.*/
        osgGA::GUIEventAdapter* closeWindow(double time);


        /** Method for adapting application quit events.*/
        osgGA::GUIEventAdapter* quitApplication() { return quitApplication(getTime()); }

        /** Method for adapting application quit events with specified event time.*/
        osgGA::GUIEventAdapter* quitApplication(double time);


        /** Method for adapting frame events.*/
        osgGA::GUIEventAdapter* frame(double time);


        void setStartTick(osg::Timer_t tick) { _startTick = tick; clear(); }
        osg::Timer_t getStartTick() const { return _startTick; }

        double getTime() const { return osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick()); }

        /** clear all events from queue. */
        void clear();

        /** convenience method for create an event ready to fill in.  Clones the getCurrentEventState() to produce a up to date event state. */
        GUIEventAdapter* createEvent();


        void setCurrentEventState(GUIEventAdapter* ea) { _accumulateEventState = ea; }
        GUIEventAdapter* getCurrentEventState() { return _accumulateEventState.get(); }
        const GUIEventAdapter* getCurrentEventState() const { return _accumulateEventState.get(); }

        /** Method for adapting user defined events */
        GUIEventAdapter* userEvent(osg::Referenced* userEventData) { return userEvent(userEventData, getTime()); }

        /** Method for adapting user defined events with specified event time */
        GUIEventAdapter* userEvent(osg::Referenced* userEventData, double time);

        void setFirstTouchEmulatesMouse(bool b) { _firstTouchEmulatesMouse = b; }
        bool getFirstTouchEmulatesMouse() const { return _firstTouchEmulatesMouse; }

    protected:

        virtual ~EventQueue();

        /** Prevent unwanted copy operator.*/
        EventQueue& operator = (const EventQueue&) { return *this; }

        osg::ref_ptr<GUIEventAdapter>   _accumulateEventState;

        bool                        _useFixedMouseInputRange;

        osg::Timer_t                _startTick;
        mutable OpenThreads::Mutex  _eventQueueMutex;
        Events                      _eventQueue;
        bool                        _firstTouchEmulatesMouse;

};

}

#endif
