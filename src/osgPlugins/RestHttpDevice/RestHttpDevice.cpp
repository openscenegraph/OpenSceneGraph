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

#include "RestHttpDevice.hpp"
#include <OpenThreads/Thread>
#include <osg/ValueObject>
#include <osgDB/FileUtils>
#include "request_handler.hpp"

namespace RestHttp {


class UserEventRequestHandler : public RestHttpDevice::RequestHandler {
public:
    UserEventRequestHandler() : RestHttpDevice::RequestHandler("/user-event") {}
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply)
    {
        OSG_INFO << "RestHttpDevice :: handling request " << full_request_path << " as user-event" << std::endl;
        
        osg::ref_ptr<osgGA::Event> event = new osgGA::Event();
        event->setName(full_request_path);
        event->setTime(getDevice()->getEventQueue()->getTime());
        
        for(Arguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
        {
            event->setUserValue(i->first,i->second);
        }
        getDevice()->getEventQueue()->addEvent(event.get());
        
        return sendOkReply(reply);
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": fall-through request-handler, catches all requests w/o registered handler and report them to the console" << std::dec;
    }
};

class HomeRequestHandler : public RestHttpDevice::RequestHandler {
public:
    HomeRequestHandler()
        : RestHttpDevice::RequestHandler("/home")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply)
    {
        double time = getLocalTime(arguments, reply);
        getDevice()->getEventQueue()->keyPress(' ', time);
        getDevice()->getEventQueue()->keyRelease(' ', time);
        
        return sendOkReply(reply);
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": sets the mouse-input-range arguments: 'x_min','y_min', 'x_max' and 'y_max'" << std::dec;
    }
};

class SetMouseInputRangeRequestHandler : public RestHttpDevice::RequestHandler {
public:
    SetMouseInputRangeRequestHandler()
        : RestHttpDevice::RequestHandler("/mouse/set_input_range")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply)
    {
        int x_min(0), y_min(0), x_max(0), y_max(0);
        
        if (   getIntArgument(arguments, "x_min", reply, x_min)
            && getIntArgument(arguments, "y_min", reply, y_min)
            && getIntArgument(arguments, "x_max", reply, x_max)
            && getIntArgument(arguments, "y_max", reply, y_max))
        {
            getDevice()->getEventQueue()->setMouseInputRange(x_min, y_min, x_max, y_max);
        }
        
        return sendOkReply(reply);
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": sets the mouse-input-range arguments: 'x_min','y_min', 'x_max' and 'y_max'" << std::dec;
    }
};





class KeyCodeRequestHandler : public RestHttpDevice::RequestHandler {
public:
    KeyCodeRequestHandler(bool handle_key_press)
        : RestHttpDevice::RequestHandler(std::string("/key/") + ((handle_key_press) ? "press" : "release"))
        , _handleKeyPress(handle_key_press)
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply)
    {
        int keycode(0);
        
        if (getHexArgument(arguments, "code", reply, keycode))
        {
            if (_handleKeyPress)
                getDevice()->getEventQueue()->keyPress(keycode, getLocalTime(arguments, reply));
            else
                getDevice()->getEventQueue()->keyRelease(keycode, getLocalTime(arguments, reply));
        }
        
        return sendOkReply(reply);
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": send KEY_" << (_handleKeyPress ? "DOWN" : "UP") <<", using hex-argument 'code' as keycode" << std::dec;
    }
private:
    bool _handleKeyPress;
};


class MouseMotionRequestHandler : public RestHttpDevice::RequestHandler {
public:
    MouseMotionRequestHandler()
        : RestHttpDevice::RequestHandler("/mouse/motion")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply)
    {
        int x(0),y(0);
        if (getIntArgument(arguments, "x", reply, x) && getIntArgument(arguments, "y", reply, y))
        {
            double time_stamp = getTimeStamp(arguments, reply);
            
            if (getDevice()->isNewer(time_stamp))
            {
                //getDevice()->getEventQueue()->mouseMotion(x,y, getLocalTime(time_stamp));
                getDevice()->setTargetMousePosition(x,y);
            }
        }
        
        return sendOkReply(reply);
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": send mouse motion using arguments 'x' and 'y' as coordinates" << std::dec;
    }
private:

};


class MouseButtonRequestHandler : public RestHttpDevice::RequestHandler {
public:
    enum Mode { PRESS, RELEASE, DOUBLE_PRESS};
    
    MouseButtonRequestHandler(Mode mode)
        : RestHttpDevice::RequestHandler("")
        , _mode(mode)
    {
        switch(mode) {
            case PRESS:
                setRequestPath("/mouse/press");
                break;
            case RELEASE:
                setRequestPath("/mouse/release");
                break;
            case DOUBLE_PRESS:
                setRequestPath("/mouse/doublepress");
                break;
        }
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const Arguments& arguments, http::server::reply& reply)
    {
        int x(0),y(0), button(0);

        if (getIntArgument(arguments, "x", reply, x)
            && getIntArgument(arguments, "y", reply, y)
            && getIntArgument(arguments, "button", reply, button))
        {
            getDevice()->setTargetMousePosition(x,y, true);
            switch (_mode) {
                case PRESS:
                    getDevice()->getEventQueue()->mouseButtonPress(x,y, button, getLocalTime(arguments, reply));
                    break;
                case RELEASE:
                    getDevice()->getEventQueue()->mouseButtonRelease(x,y, button, getLocalTime(arguments, reply));
                    break;
                case DOUBLE_PRESS:
                    getDevice()->getEventQueue()->mouseDoubleButtonPress(x,y, button, getLocalTime(arguments, reply));
                    break;
            }
        }
        
        return sendOkReply(reply);
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": send mouse ";
        switch (_mode) {
            case PRESS:
                out << "press"; break;
            case RELEASE:
                out << "release"; break;
            case DOUBLE_PRESS:
                out << "double press"; break;
        }
        out << " using arguments 'x', 'y' and 'button' as coordinates" << std::dec;
    }
private:
    Mode _mode;
};






class RequestHandlerDispatcherCallback: public http::server::request_handler::Callback {
public:
    RequestHandlerDispatcherCallback(RestHttpDevice* parent)
        : http::server::request_handler::Callback()
        , _parent(parent)
    {
    }
    
    virtual bool operator()(const std::string& request_path, http::server::reply& rep);
    
    virtual std::string applyTemplateVars(const std::string& txt) { return txt; }
private:
    RestHttpDevice* _parent;
};


bool RequestHandlerDispatcherCallback::operator()(const std::string& request_path, http::server::reply& reply)
{
    return _parent->handleRequest(request_path, reply);
}

}

RestHttpDevice::RestHttpDevice(const std::string& listening_address, const std::string& listening_port, const std::string& doc_root)
    : osgGA::Device()
    , OpenThreads::Thread()
    , _server(listening_address, listening_port, osgDB::findDataFile(doc_root), std::max(OpenThreads::GetNumberOfProcessors() - 1, 1))
    , _serverAddress(listening_address)
    , _serverPort(listening_port)
    , _documentRoot(doc_root)
    , _firstEventLocalTimeStamp()
    , _firstEventRemoteTimeStamp(-1)
    , _lastEventRemoteTimeStamp(0)
    , _currentMouseX(0.0f)
    , _currentMouseY(0.0f)
    , _targetMouseX(0.0f)
    , _targetMouseY(0.0f)
    , _targetMouseChanged(false)
{
    setCapabilities(RECEIVE_EVENTS);
    
    OSG_NOTICE << "RestHttpDevice :: listening on " << listening_address << ":" << listening_port << ", document root: " << doc_root << std::endl;
    
    if (osgDB::findDataFile(doc_root).empty())
    {
        OSG_WARN << "RestHttpDevice :: warning, can't locate document-root '" << doc_root << "'for the http-server, starting anyway" << std::endl;
    }
    _server.setCallback(new RestHttp::RequestHandlerDispatcherCallback(this));
    
    addRequestHandler(new RestHttp::KeyCodeRequestHandler(false));
    addRequestHandler(new RestHttp::KeyCodeRequestHandler(true));
    
    addRequestHandler(new RestHttp::SetMouseInputRangeRequestHandler());
    addRequestHandler(new RestHttp::MouseMotionRequestHandler());
    addRequestHandler(new RestHttp::MouseButtonRequestHandler(RestHttp::MouseButtonRequestHandler::PRESS));
    addRequestHandler(new RestHttp::MouseButtonRequestHandler(RestHttp::MouseButtonRequestHandler::RELEASE));
    addRequestHandler(new RestHttp::MouseButtonRequestHandler(RestHttp::MouseButtonRequestHandler::DOUBLE_PRESS));
    
    addRequestHandler(new RestHttp::HomeRequestHandler());
    
    addRequestHandler(new RestHttp::UserEventRequestHandler());
    
    // start the thread
    start();
}


void RestHttpDevice::run()
{
    _server.run();
}


RestHttpDevice::~RestHttpDevice()
{
    _server.stop();
    join();
}


void RestHttpDevice::addRequestHandler(RequestHandler* handler)
{
    if (handler)
    {
        _map.insert(std::make_pair(handler->getRequestPath(), handler));
        handler->setDevice(this);
    }
}


void RestHttpDevice::parseArguments(const std::string request_path, RequestHandler::Arguments& arguments)
{
    std::size_t pos = request_path.find('?');
    if (pos == std::string::npos)
        return;
    
    std::vector<std::string> list;
    osgDB::split(request_path.substr(pos+1, std::string::npos), list, '&');
    for(std::vector<std::string>::iterator i = list.begin(); i != list.end(); ++i)
    {
        std::vector<std::string> sub_list;
        osgDB::split(*i, sub_list, '=');
        if (sub_list.size() == 2)
            arguments[sub_list[0]] = sub_list[1];
        else if (sub_list.size() == 1)
            arguments[sub_list[0]] = "";
    }
}


bool RestHttpDevice::handleRequest(const std::string& in_request_path,  http::server::reply& reply)
{
    std::string request_path = in_request_path.substr(0, in_request_path.find('?'));
    request_path += "/";
    RequestHandler::Arguments arguments;
    bool arguments_parsed(false);
    
    std::size_t pos(std::string::npos);
    bool handled(false);
    do {
        pos = request_path.find_last_of('/', pos-1);
        if (pos != std::string::npos)
        {
            std::string mangled_path = request_path.substr(0, pos);
            
            std::pair<RequestHandlerMap::iterator,RequestHandlerMap::iterator> range = _map.equal_range(mangled_path);
            if (!arguments_parsed && (range.first != range.second))
            {
                // parse arguments
                parseArguments(in_request_path, arguments);
                arguments_parsed = true;
            }
            for(RequestHandlerMap::iterator i = range.first; i != range.second; ++i)
            {
                if (i->second->operator()(mangled_path, in_request_path, arguments, reply) && !handled)
                    handled = true;
            }
            
        }
    } while ((pos != std::string::npos) && (pos > 0) && !handled);
    
    return handled;
}


void RestHttpDevice::describeTo(std::ostream& out) const
{
    out << "RestHttpDevice :: Server:        " << _serverAddress << std::endl;
    out << "RestHttpDevice :: Port:          " << _serverPort << std::endl;
    out << "RestHttpDevice :: Document-Root: " << _documentRoot << std::endl;
    out << std::endl;
    
    for(RequestHandlerMap::const_iterator i = _map.begin(); i != _map.end(); ++i)
    {
        const RequestHandler* handler(i->second.get());
        out << "RestHttpDevice :: ";
        handler->describeTo(out);
        out << std::endl;
    }

}

