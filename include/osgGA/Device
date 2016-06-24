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

#ifndef OSGGA_EVENTSOURCE
#define OSGGA_EVENTSOURCE 1

#include <osgGA/EventQueue>

namespace osgGA {

/**
 * Device base class from abstracting away from devices/windows that can generate events.
 */
class OSGGA_EXPORT Device : public osg::Object
{
    public:
        enum Capabilities
        {
            UNKNOWN         = 0,
            RECEIVE_EVENTS  = 1,
            SEND_EVENTS     = 2
        };

        Device();
        Device(const Device& es, const osg::CopyOp& copyop);

        META_Object(osgGA,Device);

        int getCapabilities() const { return _capabilities; }

        virtual bool checkEvents() { return _eventQueue.valid() ? !(getEventQueue()->empty()) : false; }
        virtual void sendEvent(const Event& ea);
        virtual void sendEvents(const EventQueue::Events& events);

        void setEventQueue(osgGA::EventQueue* eventQueue) { _eventQueue = eventQueue; }
        osgGA::EventQueue* getEventQueue() { return _eventQueue.get(); }
        const osgGA::EventQueue* getEventQueue() const { return _eventQueue.get(); }

    protected:
        void setCapabilities(int capabilities) { _capabilities = capabilities; }

        virtual ~Device();

        /** Prevent unwanted copy operator.*/
        Device& operator = (const Device&) { return *this; }

        osg::ref_ptr<osgGA::EventQueue> _eventQueue;

    private:
        int _capabilities;

};

}

#endif
