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
 * OscReceivingDevice can be used to receive osg-events via OSC from other hosts/ applications
 * It can even translate custom osc-message to user-events with attached user-data.
 * 
 * It uses the TUIO 1.1 Cursor2D-profile to receive multitouch-events
 *
 * This device adds to every message-bundle a message-id, so you can check, if you miss events or similar.
 *
 * receiving custom osc-data:
 *    /my_user_event/value_1 23
 *    /my_user_event/value_2 42
 *
 * this will result in one osgGA:Event (when both messages are bundled) witht the name "/my_user_event", 
 * and two user-values (value_1 and value_2)
 * To get value_1 you'll do something like event->getUserValue("value_1", my_int);
 * 
 * Currently osg's user-data can not cast to different types, they have to match, so 
 * event->getUserValue("value_1", my_string) will fail.
 *
 * The receiving device will try to combine multiple osc arguments intelligently, multiple osc-arguments are
 * bundled into Vec2, Vec3, Vec4 or Matrix, it depends on the number of arguments.
 
 * TUIO-specific notes:
 * If multiple TUIO-applications are transmitting their touch-points to one oscReceivingDevice, all
 * touchpoints get multiplexed, so you'll get one event with x touchpoints. 
 * You can differentiate the specific applications by the touch_ids, the upper 16bits 
 * are specific to an application, the lower 16bits contain the touch-id for that application.
 * If you need "better" separation, use multiple oscReceivingDevices listening on different ports.
 *
 * @TODO implement other TUIO-profiles
 *
 */


#include <osg/Referenced>
#include <OpenThreads/Thread>
#include <osgGA/Device>
#include <osc/OscPacketListener.h>
#include <ip/UdpSocket.h>
#include "OscSendingDevice.hpp"



class OscReceivingDevice : public osgGA::Device, OpenThreads::Thread, osc::OscPacketListener {

public:
    typedef OscSendingDevice::MsgIdType MsgIdType;
    
    class RequestHandler : public osg::Referenced {
    public:
        RequestHandler(const std::string& request_path)
            : osg::Referenced()
            , _requestPath(request_path)
            , _device(NULL)
        {
        }
        
        virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m, const IpEndpointName& remoteEndPoint) = 0;
        virtual void operator()(osgGA::EventQueue* queue) {}
        
        const std::string& getRequestPath() const { return _requestPath; }
        
        virtual void describeTo(std::ostream& out) const
        {
            out << getRequestPath() << ": no description available";
        }
        
    protected:
        virtual void setDevice(OscReceivingDevice* device) { _device = device; }
        OscReceivingDevice* getDevice() const { return _device; }
            
        /// set the request-path, works only from the constructor
        void setRequestPath(const std::string& request_path) { _requestPath = request_path; }
    
        void handleException(const osc::Exception& e)
        {
            OSG_WARN << "OscDevice :: error while handling " << getRequestPath() << ": " << e.what() << std::endl;
        }
        
        double getLocalTime() const { return getDevice()->getEventQueue()->getTime(); }
    private:
        std::string _requestPath;
        OscReceivingDevice* _device;
    friend class OscReceivingDevice;
    };
    
    typedef std::multimap<std::string, osg::ref_ptr<RequestHandler> > RequestHandlerMap;
    
    OscReceivingDevice(const std::string& server_address, int listening_port);
    ~OscReceivingDevice();
    
        
    virtual void run();
    
    
    virtual void ProcessMessage( const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint );
    virtual void ProcessPacket( const char *data, int size, const IpEndpointName& remoteEndpoint );
    virtual void ProcessBundle( const osc::ReceivedBundle& b, const IpEndpointName& remoteEndpoint );
    void addRequestHandler(RequestHandler* handler);
    
    void describeTo(std::ostream& out) const;
    
    friend std::ostream& operator<<(std::ostream& out, const OscReceivingDevice& device)
    {
        device.describeTo(out);
        return out;
    }
    
    osgGA::Event* getOrCreateUserDataEvent()
    {
        if (!_userDataEvent.valid())
        {
            _userDataEvent = new osgGA::Event();
        }
        return _userDataEvent.get();
    }
    
    virtual const char* className() const { return "OSC receiving device"; }
    
    void addHandleOnCheckEvents(RequestHandler* handler) { _handleOnCheckEvents.push_back(handler); }
    
    virtual bool checkEvents() {
        osgGA::EventQueue* queue = getEventQueue();
        
        for(std::vector<RequestHandler*>::iterator i = _handleOnCheckEvents.begin(); i != _handleOnCheckEvents.end(); ++i) {
            (*i)->operator()(queue);
        }
        return osgGA::Device::checkEvents();
    }
    
private:
    std::string _listeningAddress;
    unsigned int _listeningPort;
    UdpListeningReceiveSocket* _socket;
    RequestHandlerMap _map;
    osg::ref_ptr<osgGA::Event> _userDataEvent;
    MsgIdType _lastMsgId;
    osg::Timer_t _lastMsgTimeStamp;
    std::vector<RequestHandler*> _handleOnCheckEvents;

};


class SendKeystrokeRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    SendKeystrokeRequestHandler(const std::string& request_path, int key) : OscReceivingDevice::RequestHandler(request_path), _key(key) {}
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& arguments, const IpEndpointName& remoteEndPoint)
    {
        getDevice()->getEventQueue()->keyPress(_key);
        getDevice()->getEventQueue()->keyRelease(_key);
        
        return true;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": send KEY_DOWN + KEY_UP, code: 0x" << std::hex << _key << std::dec;
    }
private:
    int _key;
};

