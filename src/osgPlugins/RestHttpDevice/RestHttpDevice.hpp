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
#include "server.hpp"

class RestHttpDevice : public osgGA::Device, OpenThreads::Thread {

public:
    
    class RequestHandler : public osg::Referenced {
    public:
        typedef std::map<std::string, std::string> Arguments;
        RequestHandler(const std::string& request_path)
            : osg::Referenced()
            , _requestPath(request_path)
            , _device(NULL)
        {
        }
        
        virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply) = 0;
        
        const std::string& getRequestPath() const { return _requestPath; }
        
        virtual void describeTo(std::ostream& out) const
        {
            out << getRequestPath() << ": no description available";
        }
        
    protected:
        void setDevice(RestHttpDevice* device) { _device = device; }
        RestHttpDevice* getDevice() { return _device; }
        
        void reportMissingArgument(const std::string& argument, http::server::reply& reply) const
        {
            OSG_WARN << "RequestHandler :: missing argument '" << argument << "' for " << getRequestPath() << std::endl;
            reply.content = "{ \"result\": 0, \"error\": \"missing argument '"+argument+"'\"}";
            reply.status = http::server::reply::ok;
        }
        
        bool sendOkReply(http::server::reply& reply)
        {
            if (reply.content.empty())
            {
                reply.status = http::server::reply::no_content;
            }
            return true;
        }
        
        bool getStringArgument(const Arguments& arguments, const std::string& argument, http::server::reply& reply, std::string& result) const
        {
            Arguments::const_iterator itr = arguments.find(argument);
            if (itr == arguments.end()) {
                reportMissingArgument(argument, reply);
                return false;
            }
            result = itr->second;
            return true;
        }
        
        bool getHexArgument(const Arguments& arguments, const std::string& argument, http::server::reply& reply, int& value) const
        {
            std::string hex_str;
            if (!getStringArgument(arguments, argument, reply, hex_str))
                return false;
            value = strtoul(hex_str.c_str(), NULL, 16);
            return true;
        }
        
        bool getIntArgument(const Arguments& arguments, const std::string& argument, http::server::reply& reply, int& value) const
        {
            std::string str;
            if (!getStringArgument(arguments, argument, reply, str))
                return false;
            value = strtol(str.c_str(), NULL, 10);
            return true;
        }
        
        bool getDoubleArgument(const Arguments& arguments, const std::string& argument, http::server::reply& reply, double& value) const
        {
            std::string str;
            if (!getStringArgument(arguments, argument, reply, str))
                return false;
            value = strtod(str.c_str(), NULL);
            return true;
        }
        
        /// set the request-path, works only from the constructor
        void setRequestPath(const std::string& request_path) { _requestPath = request_path; }
    protected:
        
        double getTimeStamp(const Arguments& arguments, http::server::reply& reply)
        {
            double time_stamp(0.0);
            getDoubleArgument(arguments, "time", reply, time_stamp);
            return time_stamp;
        }
        
        double getLocalTime(const Arguments& arguments, http::server::reply& reply)
        {
            return getLocalTime(getTimeStamp(arguments, reply));
        }
        
        double getLocalTime(double time_stamp)
        {
            return getDevice()->getLocalTime(time_stamp);
        }
        
    private:
        std::string _requestPath;
        RestHttpDevice* _device;
    friend class RestHttpDevice;
    };
    
    
    typedef std::multimap<std::string, osg::ref_ptr<RequestHandler> > RequestHandlerMap;
    
    RestHttpDevice(const std::string& listening_address, const std::string& listening_port, const std::string& doc_path);
    ~RestHttpDevice();
    
    void addRequestHandler(RequestHandler* handler);
    
    bool handleRequest(const std::string& request_path,  http::server::reply& reply);
    
    virtual void run();
    
    void describeTo(std::ostream& out) const;
    
    friend std::ostream& operator<<(std::ostream& out, const RestHttpDevice& device)
    {
        device.describeTo(out);
        return out;
    }
    
    double getLocalTime(double time_stamp)
    {
        if (_firstEventRemoteTimeStamp < 0)
        {
            _firstEventLocalTimeStamp = getEventQueue()->getTime();
            _firstEventRemoteTimeStamp = time_stamp;
        }
        double local_time = _firstEventLocalTimeStamp + (time_stamp - _firstEventRemoteTimeStamp);
        // std::cout << "ts: "<< time_stamp << " -> " << local_time << std::endl;
        return  local_time;
    }
    
    bool isNewer(double time_stamp)
    {
        bool is_newer(time_stamp > _lastEventRemoteTimeStamp);
        if (is_newer)
            _lastEventRemoteTimeStamp = time_stamp;
        return is_newer;
    }
    

    
    virtual bool checkEvents()
    {
        if (_targetMouseChanged && (fabs(_currentMouseX - _targetMouseY) > 0.1f) || (fabs(_currentMouseY - _targetMouseY) > 0.1))
        {
            static const float scalar = 0.2f;
            _currentMouseX = (1.0f - scalar) * _currentMouseX + scalar * _targetMouseX;
            _currentMouseY = (1.0f - scalar) * _currentMouseY + scalar * _targetMouseY;
            getEventQueue()->mouseMotion(_currentMouseX, _currentMouseY, getEventQueue()->getTime());
        }
        return !(getEventQueue()->empty());
    }
    
    void setTargetMousePosition(float x, float y, bool force = false)
    {
        _targetMouseChanged = true;
        _targetMouseX = x; _targetMouseY = y;
        if (force) {
            _currentMouseX = x; _currentMouseY = y;
        }
    }
    
    
    
private:
    void parseArguments(const std::string request_path, RequestHandler::Arguments& arguments);
    http::server::server _server;
    RequestHandlerMap _map;
    std::string _serverAddress, _serverPort, _documentRoot;
    double _firstEventLocalTimeStamp;
    double _firstEventRemoteTimeStamp;
    double _lastEventRemoteTimeStamp;
    float _currentMouseX, _currentMouseY, _targetMouseX, _targetMouseY;
    bool _targetMouseChanged;
    
};



class SendKeystrokeRequestHandler : public RestHttpDevice::RequestHandler {
public:
    SendKeystrokeRequestHandler(const std::string& request_path, int key) : RestHttpDevice::RequestHandler(request_path), _key(key) {}
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply)
    {
        double local_time = getLocalTime(arguments, reply);
        
        getDevice()->getEventQueue()->keyPress(_key, local_time);
        getDevice()->getEventQueue()->keyRelease(_key, local_time);
        
        return sendOkReply(reply);
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": send KEY_DOWN + KEY_UP, code: 0x" << std::hex << _key << std::dec;
    }
private:
    int _key;
};

