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

#include "OscDevice.hpp"
#include <OpenThreads/Thread>
#include <osgDB/FileUtils>
#include "osc/OscPrintReceivedElements.h"


class StandardRequestHandler : public OscDevice::RequestHandler {
public:
    StandardRequestHandler() : OscDevice::RequestHandler("") {}
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        OSG_NOTICE << "OscDevice :: unhandled request: " << full_request_path << std::endl;
        
        for(osc::ReceivedMessageArgumentIterator itr = m.ArgumentsBegin(); itr != m.ArgumentsEnd(); ++itr)
        {
            OSG_NOTICE << "      " << (*itr) << std::endl;
            
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": fall-through request-handler, catches all requests w/o registered handler and report them to the console";
    }
};



class SetMouseInputRangeRequestHandler : public OscDevice::RequestHandler {
public:
    SetMouseInputRangeRequestHandler()
        : OscDevice::RequestHandler("/osgga/mouse/set_input_range")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        try {
            float x_min(-1.0f), y_min(-1.0f), x_max(1.0f), y_max(1.0f);
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> x_min >> y_min >> x_max >> y_max >> osc::EndMessage;
            
            getDevice()->getEventQueue()->setMouseInputRange(x_min, y_min, x_max, y_max);
                
            return true;
        }
        catch(osc::Exception e) {
            handleException(e);
        }
        
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(float x_min, float y_min, float x_max, float y_max): sets the mouse-input-range" << std::dec;
    }
};


class SetMouseOrientationRequestHandler : public OscDevice::RequestHandler {
public:
    SetMouseOrientationRequestHandler()
        : OscDevice::RequestHandler("/osgga/mouse/y_orientation_increasing_upwards")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        try {
            bool increasing_upwards(false);
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >>increasing_upwards >> osc::EndMessage;
            
            getDevice()->getEventQueue()->getCurrentEventState()->setMouseYOrientation(
                increasing_upwards ? osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS : osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS);
                
            return true;
        }
        catch(osc::Exception e) {
            handleException(e);
        }
        
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(float x_min, float y_min, float x_max, float y_max): sets the mouse-input-range" << std::dec;
    }
};


class KeyCodeRequestHandler : public OscDevice::RequestHandler {
public:
    KeyCodeRequestHandler(bool handle_key_press)
        : OscDevice::RequestHandler(std::string("/osgga/key/") + ((handle_key_press) ? "press" : "release"))
        , _handleKeyPress(handle_key_press)
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        try {
            osc::int32 keycode(0);
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> keycode >> osc::EndMessage;
            
            if (_handleKeyPress)
                getDevice()->getEventQueue()->keyPress(keycode, getLocalTime());
            else
                getDevice()->getEventQueue()->keyRelease(keycode, getLocalTime());
            
            return true;
        }
        catch(osc::Exception e) {
            handleException(e);
        }
        
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(int keycode): send KEY_" << (_handleKeyPress ? "DOWN" : "UP");
    }
private:
    bool _handleKeyPress;
};


class KeyPressAndReleaseRequestHandler : public OscDevice::RequestHandler {
public:
    KeyPressAndReleaseRequestHandler()
        : OscDevice::RequestHandler("/osgga/key/press_and_release")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        try {
            osc::int32 keycode(0);
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> keycode >> osc::EndMessage;
            
            getDevice()->getEventQueue()->keyPress(keycode, getLocalTime());
            getDevice()->getEventQueue()->keyRelease(keycode, getLocalTime());
            
            return true;
        }
        catch(osc::Exception e) {
            handleException(e);
        }
        
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(int keycode): send KEY_DOWN and KEY_UP";
    }
private:
    bool _handleKeyPress;
};


class MouseMotionRequestHandler : public OscDevice::RequestHandler {
public:
    MouseMotionRequestHandler()
        : OscDevice::RequestHandler("/osgga/mouse/motion")
        , _lastX(0.0f)
        , _lastY(0.0f)
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        
        try {
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> _lastX >> _lastY >> osc::EndMessage;
        
            getDevice()->getEventQueue()->mouseMotion(_lastX, _lastY, getLocalTime());
            
            return true;
        }
        catch (osc::Exception e) {
            handleException(e);
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(float x, float y): send mouse motion";
    }
    float getLastX() const { return _lastX; }
    float getLastY() const { return _lastY; }
private:
    float _lastX, _lastY;
};

class MouseButtonToggleRequestHandler : public OscDevice::RequestHandler {
public:
    MouseButtonToggleRequestHandler(const std::string& btn_name, MouseMotionRequestHandler* mm_handler)
        : OscDevice::RequestHandler("/osgga/mouse/toggle/"+btn_name)
        , _mmHandler(mm_handler)
        , _btnNum(atoi(btn_name.c_str()))
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        float down(0.0f);
        
        try {
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> down >> osc::EndMessage;
            
            if (down > 0)
                getDevice()->getEventQueue()->mouseButtonPress(_mmHandler->getLastX(), _mmHandler->getLastY(), _btnNum, getLocalTime());
            else
                getDevice()->getEventQueue()->mouseButtonRelease(_mmHandler->getLastX(), _mmHandler->getLastY(), _btnNum, getLocalTime());
            
            return true;
        }
        catch (osc::Exception e) {
            handleException(e);
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(float down): toggle mouse button";
    }
private:
    osg::observer_ptr<MouseMotionRequestHandler> _mmHandler;
    int _btnNum;
};


class MouseButtonRequestHandler : public OscDevice::RequestHandler {
public:
    enum Mode { PRESS, RELEASE, DOUBLE_PRESS};
    
    MouseButtonRequestHandler(Mode mode)
        : OscDevice::RequestHandler("")
        , _mode(mode)
    {
        switch(mode) {
            case PRESS:
                setRequestPath("/osgga/mouse/press");
                break;
            case RELEASE:
                setRequestPath("/osgga/mouse/release");
                break;
            case DOUBLE_PRESS:
                setRequestPath("/osgga/mouse/doublepress");
                break;
        }
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        float  x(0.0f), y(0.0f);
        osc::int32 btn(0);
        
        try {
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> x >> y >> btn >> osc::EndMessage;
            switch (_mode) {
                case PRESS:
                    getDevice()->getEventQueue()->mouseButtonPress(x,y, btn, getLocalTime());
                    break;
                case RELEASE:
                    getDevice()->getEventQueue()->mouseButtonRelease(x,y, btn, getLocalTime());
                    break;
                case DOUBLE_PRESS:
                    getDevice()->getEventQueue()->mouseDoubleButtonPress(x,y, btn, getLocalTime());
                    break;
            }
            
            return true;
        }
        catch (osc::Exception e) {
            handleException(e);
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(float x, float y, int btn): send mouse ";
        switch (_mode) {
            case PRESS:
                out << "press"; break;
            case RELEASE:
                out << "release"; break;
            case DOUBLE_PRESS:
                out << "double press"; break;
        }
    }
    
private:
    Mode _mode;
};



OscDevice::OscDevice(const std::string& server_address, int listening_port)
    : osgGA::Device()
    , OpenThreads::Thread()
    , osc::OscPacketListener()
    , _listeningAddress(server_address)
    , _listeningPort(listening_port)
    , _socket(NULL)
    , _map()
{
    _socket = new UdpListeningReceiveSocket(IpEndpointName( server_address.c_str(), listening_port ), this);
    
    addRequestHandler(new KeyCodeRequestHandler(false));
    addRequestHandler(new KeyCodeRequestHandler(true));
    addRequestHandler(new KeyPressAndReleaseRequestHandler());
    
    addRequestHandler(new SetMouseInputRangeRequestHandler());
    addRequestHandler(new SetMouseOrientationRequestHandler());
    
    MouseMotionRequestHandler* mm_handler = new MouseMotionRequestHandler();
    addRequestHandler(mm_handler);
    addRequestHandler(new MouseButtonRequestHandler(MouseButtonRequestHandler::PRESS));
    addRequestHandler(new MouseButtonRequestHandler(MouseButtonRequestHandler::RELEASE));
    addRequestHandler(new MouseButtonRequestHandler(MouseButtonRequestHandler::DOUBLE_PRESS));
    
    addRequestHandler(new MouseButtonToggleRequestHandler("1", mm_handler));
    addRequestHandler(new MouseButtonToggleRequestHandler("2", mm_handler));
    addRequestHandler(new MouseButtonToggleRequestHandler("3", mm_handler));
    
    addRequestHandler(new StandardRequestHandler());
    
    start();
}

OscDevice::~OscDevice()
{
    _socket->AsynchronousBreak();
    join();
    delete _socket;
}

void OscDevice::run()
{
    _socket->Run();
    
}


void OscDevice::ProcessMessage( const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint )
{
    std::string in_request_path(m.AddressPattern());
    std::string request_path = in_request_path + "/";

    std::size_t pos(std::string::npos);
    bool handled(false);
    do {
        pos = request_path.find_last_of('/', pos-1);
        if (pos != std::string::npos)
        {
            std::string mangled_path = request_path.substr(0, pos);
            
            std::pair<RequestHandlerMap::iterator,RequestHandlerMap::iterator> range = _map.equal_range(mangled_path);
            
            for(RequestHandlerMap::iterator i = range.first; i != range.second; ++i)
            {
                OSG_INFO << "OscDevice :: handling " << mangled_path << " with " << i->second << std::endl;
                
                if (i->second->operator()(mangled_path, in_request_path, m) && !handled)
                    handled = true;
            }
            
        }
    } while ((pos != std::string::npos) && (pos > 0) && !handled);

}

void OscDevice::addRequestHandler(RequestHandler* handler)
{
    if (handler)
    {
        _map.insert(std::make_pair(handler->getRequestPath(), handler));
        handler->setDevice(this);
    }
}

void OscDevice::describeTo(std::ostream& out) const
{
    out << "OscDevice :: listening on " << _listeningAddress << ":" << _listeningPort << std::endl;
    out << std::endl;
    
    for(RequestHandlerMap::const_iterator i = _map.begin(); i != _map.end(); ++i)
    {
        const RequestHandler* handler(i->second.get());
        out << "OscDevice :: ";
        handler->describeTo(out);
        out << std::endl;
    }

}
