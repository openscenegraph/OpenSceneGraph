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


#pragma once

/**
 * OscSendingDevice can be used to send osg-events via OSC to other hosts/ applications
 * It can even transmit user-events with attached user-data.
 * 
 * It uses the TUIO 1.1 Cursor2D-profile to send multitouch-events
 *
 * This device adds to every message-bundle a message-id, so you can check, if you miss events or similar.
 *
 * sending custom data via an event's user-data:
 *   osgGA::Event* event = new osgGA::Event();
 *   event->setName("/my_user_event");
 *   event->setUserValue("value_1", 23);
 *   event->setUserValue("value_2", 42);
 *   device->sendEvent(event);
 *
 * The receiver gets a message-bundle with 2 messages:
 *    /my_user_event/value_1 23
 *    /my_user_event/value_2 42
 *
 * The sending device knows how to handle Vec2, Vec3, Vec4, Quat, Plane and Matrix values
 *
 * TUIO-specific notes:
 * If you want to explicitly set the application-name when sending multi-touch-events, use
 * osc_device->setUserValue("tuio_application_name", "<your application name>");
 * You'll also have to add the address-part, e.g.
 * osc_device->setUserValue("tuio_application_name", "my_tuio_application@192.168.1.1");
 *
 * @TODO get local address for a correct TUIO-application-name
 * @TODO implement other TUIO-profiles
 * @TODO compute velocity + acceleration for TUIO-messages
 *
 */


#include <osgGA/Device>
#include <ip/UdpSocket.h>
#include <osc/OscOutboundPacketStream.h>

class OscSendingDevice : public osgGA::Device {
public:
    typedef osc::int64 MsgIdType;
    OscSendingDevice(const std::string& address, int port, unsigned int numMessagesPerEvent = 1, unsigned int delay_between_sends_in_millisecs = 0);
    ~OscSendingDevice();
    virtual void sendEvent(const osgGA::Event &ea);
    virtual const char* className() const { return "OSC sending device"; }
    
private:
    bool sendEventImpl(const osgGA::Event &ea,MsgIdType msg_id);
    bool sendUIEventImpl(const osgGA::GUIEventAdapter &ea,MsgIdType msg_id);
    void beginBundle(MsgIdType msg_id);
    void beginSendInputRange(const osgGA::GUIEventAdapter& ea, MsgIdType msg_id);
    void beginMultiTouchSequence();
    bool sendMultiTouchData(const osgGA::GUIEventAdapter& ea);
    int getButtonNum(const osgGA::GUIEventAdapter& ea);
    void sendUserDataContainer(const std::string& key, const osg::UserDataContainer* udc, bool asBundle, MsgIdType msg_id);
    std::string transliterateKey(const std::string& key) const;
    
    UdpTransmitSocket _transmitSocket;
    char* _buffer;
    osc::OutboundPacketStream _oscStream;
    unsigned int _numMessagesPerEvent, _delayBetweenSendsInMilliSecs;
    osc::int64  _msgId;
    osg::ref_ptr<osgGA::GUIEventAdapter> _lastEvent;
    bool                                 _finishMultiTouchSequence;
 
};

