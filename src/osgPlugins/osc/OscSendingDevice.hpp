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
    OscSendingDevice(const std::string& address, int port);
    ~OscSendingDevice();
    virtual void sendEvent(const osgGA::GUIEventAdapter &ea);
    
private:
    void sendInit(const osgGA::GUIEventAdapter& ea);
    int getButtonNum(const osgGA::GUIEventAdapter& ea);
    void sendUserDataContainer(const std::string& key, const osg::UserDataContainer* udc, bool asBundle);
    std::string transliterateKey(const std::string& key) const;
    UdpTransmitSocket _transmitSocket;
    char* _buffer;
    osc::OutboundPacketStream _oscStream;
    bool _firstRun;    
};

