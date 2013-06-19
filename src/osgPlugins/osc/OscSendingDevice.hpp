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



#include <osgGA/Device>
#include <ip/UdpSocket.h>
#include <osc/OscOutboundPacketStream.h>

class OscSendingDevice : public osgGA::Device {
public:
    typedef osc::int64 MsgIdType;
    OscSendingDevice(const std::string& address, int port, unsigned int numMessagesPerEvent = 1, unsigned int delay_between_sends_in_millisecs = 0);
    ~OscSendingDevice();
    virtual void sendEvent(const osgGA::GUIEventAdapter &ea);
    virtual const char* className() const { return "OSC sending device"; }
    
private:
    bool sendEventImpl(const osgGA::GUIEventAdapter &ea,MsgIdType msg_id);
    void beginBundle(MsgIdType msg_id);
    void beginSendInputRange(const osgGA::GUIEventAdapter& ea, MsgIdType msg_id);
    int getButtonNum(const osgGA::GUIEventAdapter& ea);
    void sendUserDataContainer(const std::string& key, const osg::UserDataContainer* udc, bool asBundle, MsgIdType msg_id);
    std::string transliterateKey(const std::string& key) const;
    UdpTransmitSocket _transmitSocket;
    char* _buffer;
    osc::OutboundPacketStream _oscStream;
    unsigned int _numMessagesPerEvent, _delayBetweenSendsInMilliSecs;
};

