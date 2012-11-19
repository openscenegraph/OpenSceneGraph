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

#include <osg/Referenced>
#include <OpenThreads/Thread>
#include <osgGA/Device>
#include <osc/OscPacketListener.h>
#include <ip/UdpSocket.h>


class OscDevice : public osgGA::Device, OpenThreads::Thread, osc::OscPacketListener {

public:
    class RequestHandler : public osg::Referenced {
    public:
        RequestHandler(const std::string& request_path)
            : osg::Referenced()
            , _requestPath(request_path)
            , _device(NULL)
        {
        }
        
        virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m) = 0;
        
        const std::string& getRequestPath() const { return _requestPath; }
        
        virtual void describeTo(std::ostream& out) const
        {
            out << getRequestPath() << ": no description available";
        }
        
    protected:
        void setDevice(OscDevice* device) { _device = device; }
        OscDevice* getDevice() const { return _device; }
            
        /// set the request-path, works only from the constructor
        void setRequestPath(const std::string& request_path) { _requestPath = request_path; }
    
        void handleException(const osc::Exception& e)
        {
            OSG_WARN << "OscDevice :: error while handling " << getRequestPath() << ": " << e.what() << std::endl;
        }
        
        double getLocalTime() const { return getDevice()->getEventQueue()->getTime(); }
    private:
        std::string _requestPath;
        OscDevice* _device;
    friend class OscDevice;
    };
    
    typedef std::multimap<std::string, osg::ref_ptr<RequestHandler> > RequestHandlerMap;
    
    OscDevice(const std::string& server_address, int listening_port);
    ~OscDevice();
    
    virtual void checkEvents() {}
    virtual void run();
    
    virtual void ProcessMessage( const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint );
    
    void addRequestHandler(RequestHandler* handler);
    
    void describeTo(std::ostream& out) const;
    
    friend std::ostream& operator<<(std::ostream& out, const OscDevice& device)
    {
        device.describeTo(out);
        return out;
    }
    
private:
    std::string _listeningAddress;
    unsigned int _listeningPort;
    UdpListeningReceiveSocket* _socket;
    RequestHandlerMap _map;

};


class SendKeystrokeRequestHandler : public OscDevice::RequestHandler {
public:
    SendKeystrokeRequestHandler(const std::string& request_path, int key) : OscDevice::RequestHandler(request_path), _key(key) {}
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& arguments)
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

