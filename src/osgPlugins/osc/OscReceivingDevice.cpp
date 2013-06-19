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

#include "OscReceivingDevice.hpp"
#include <OpenThreads/Thread>
#include <osg/UserDataContainer>
#include <osg/ValueObject>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgPresentation/PropertyManager>
#include "osc/OscPrintReceivedElements.h"
#include "osc/OscHostEndianness.h"

namespace OscDevice {

template <class T, int SIZE>
struct NativeTypeTraits {
    typedef T type;
    static T create(const std::vector<T>& t) { return type(t); }
};

template<>
struct NativeTypeTraits<float,2> {
    typedef osg::Vec2f type;
    static type create(const std::vector<float>& t) { return type(t[0], t[1]); }
};

template<>
struct NativeTypeTraits<float,3> {
    typedef osg::Vec3f type;
    static type create(const std::vector<float>& t) { return type(t[0], t[1], t[2]); }

};

template<>
struct NativeTypeTraits<float,4> {
    typedef osg::Vec4f type;
    static type create(const std::vector<float>& t) { return type(t[0], t[1], t[2], t[3]); }

};

template<>
struct NativeTypeTraits<float,16> {
    typedef osg::Matrixf type;
    static type create(const std::vector<float>& t) { return type(&t.front()); }

};


template<>
struct NativeTypeTraits<double,2> {
    typedef osg::Vec2d type;
    static type create(const std::vector<double>& t) { return type(t[0], t[1]); }
};

template<>
struct NativeTypeTraits<double,3> {
    typedef osg::Vec3d type;
    static type create(const std::vector<double>& t) { return type(t[0], t[1], t[2]); }
};

template<>
struct NativeTypeTraits<double,4> {
    typedef osg::Vec4d type;
    static type create(const std::vector<double>& t) { return type(t[0], t[1], t[2], t[3]); }
};

template<>
struct NativeTypeTraits<double,16> {
    typedef osg::Matrixd type;
    static type create(const std::vector<double>& t) { return type(&t.front()); }

};



class StandardRequestHandler : public OscReceivingDevice::RequestHandler {

public:
    StandardRequestHandler(const std::string& request_handler, bool treat_first_argument_as_value_name)
        : OscReceivingDevice::RequestHandler(request_handler)
        , _treatFirstArgumentAsValueName(treat_first_argument_as_value_name)
    {
    }
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m);
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << ": add all transmitted arguments as ValueObjects to an USER-event";
        if (_treatFirstArgumentAsValueName)
            out << ", the first argument is used as the name of the value, if it's a string";
    }
private:
    void addArgumentToUdc(osg::UserDataContainer* udc, const std::string& key, const osc::ReceivedMessageArgumentIterator& itr);
    
    template <class T>
    bool addNativeTypeFromVector(osg::UserDataContainer* udc, const std::string& key, const std::vector<T>& arr)
    {
        switch (arr.size()) {
            case 2:
                udc->setUserValue(key, NativeTypeTraits<T,2>::create(arr));
                return true;
                break;
            case 3:
                udc->setUserValue(key, NativeTypeTraits<T,3>::create(arr));
                return true;
                break;
            case 4:
                udc->setUserValue(key, NativeTypeTraits<T,4>::create(arr));
                return true;
                break;
            case 16:
                udc->setUserValue(key, NativeTypeTraits<T,16>::create(arr));
                return true;
                break;
            default:
                return false;
        }
        return false;
    }
    
    bool _treatFirstArgumentAsValueName;
    
};



bool StandardRequestHandler::operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
{
    try
    {
        std::string path = osgDB::getFilePath(full_request_path);
        std::string last_elem = osgDB::getSimpleFileName(full_request_path);
        
        osg::ref_ptr<osgGA::GUIEventAdapter> ea = getDevice()->getOrCreateUserDataEvent();
        osg::UserDataContainer* udc = ea->getOrCreateUserDataContainer();
        
        
        ea->setName(_treatFirstArgumentAsValueName ? full_request_path : path);
        udc->setName(ea->getName());
        
        if (m.ArgumentCount() == 0) {
            return true;
        }
        
        // if we have only one argument, get it and save it to the udc
        else if (m.ArgumentCount() == 1)
        {
            addArgumentToUdc(udc, last_elem, m.ArgumentsBegin());
            return true;
        }
        else
        {
            unsigned int i(0);
            osc::ReceivedMessageArgumentIterator start = m.ArgumentsBegin();
            if ((_treatFirstArgumentAsValueName) && (start->TypeTag() == osc::STRING_TYPE_TAG))
            {
                last_elem = start->AsString();
                ++start;
                // if we hav only 2 arguments, then save the value and return
                if (m.ArgumentCount() == 2)
                {
                    addArgumentToUdc(udc, last_elem, start);
                    return true;
                }
            }
            std::vector<float> float_vec;
            std::vector<double> double_vec;
            bool mixed_arguments(false);
            for(osc::ReceivedMessageArgumentIterator itr = start; itr != m.ArgumentsEnd(); ++itr, ++i)
            {
                if(itr->TypeTag() == osc::FLOAT_TYPE_TAG)
                {
                    float_vec.push_back(itr->AsFloat());
                }
                else if(itr->TypeTag() == osc::DOUBLE_TYPE_TAG)
                {
                    double_vec.push_back(itr->AsDouble());
                }
                else if(itr->TypeTag() == osc::INT32_TYPE_TAG)
                {
                    float_vec.push_back(itr->AsInt32());
                }
                else {
                    mixed_arguments = true;
                    break;
                }
            }
            if (!mixed_arguments)
            {
                unsigned int sum = float_vec.size() + double_vec.size();
                if (sum == float_vec.size())
                {
                    if (addNativeTypeFromVector(udc, last_elem, float_vec))
                        return true;
                }
                else if (sum == double_vec.size())
                {
                    if (addNativeTypeFromVector(udc, last_elem, double_vec))
                        return true;
                }
            }
            
            for(osc::ReceivedMessageArgumentIterator itr = start; itr != m.ArgumentsEnd(); ++itr, ++i)
            {
                std::ostringstream ss;
                ss << last_elem << "_" << i;
                addArgumentToUdc(udc, ss.str(), itr);
            }
        }
        return true;
        
    }
    catch(osc::Exception& e)
    {
        handleException(e);
        return false;
    }
    return false;
}


void StandardRequestHandler::addArgumentToUdc(osg::UserDataContainer* udc, const std::string& key, const osc::ReceivedMessageArgumentIterator& itr)
{
    switch((*itr).TypeTag())
    {
        case osc::TRUE_TYPE_TAG:
            udc->setUserValue(key, true);
            break;
        
        case osc::FALSE_TYPE_TAG:
            udc->setUserValue(key, false);
            break;
        
        case osc::INT32_TYPE_TAG:
            udc->setUserValue(key, (int)((*itr).AsInt32Unchecked()));
            break;
        
        case osc::FLOAT_TYPE_TAG:
            udc->setUserValue(key, (*itr).AsFloatUnchecked());
            break;
        
        case osc::CHAR_TYPE_TAG:
            udc->setUserValue(key, (*itr).AsCharUnchecked());
            break;
        
        case osc::RGBA_COLOR_TYPE_TAG:
            // TODO: should we convert the color to an osg::Vec4?
            udc->setUserValue(key, static_cast<unsigned int>((*itr).AsRgbaColorUnchecked()));
            break;
        
        case osc::INT64_TYPE_TAG:
            // TODO 64bit ints not supported by ValueObject
            udc->setUserValue(key, static_cast<double>((*itr).AsInt64Unchecked()));
            break;
        
        case osc::TIME_TAG_TYPE_TAG:
            // TODO 64bit ints not supported by ValueObject
            udc->setUserValue(key, static_cast<double>((*itr).AsTimeTagUnchecked()));
            break;
        
        case osc::DOUBLE_TYPE_TAG:
            udc->setUserValue(key, (*itr).AsDoubleUnchecked());
            break;
        
        case osc::STRING_TYPE_TAG:
            udc->setUserValue(key, std::string((*itr).AsStringUnchecked()));
            break;
        
        case osc::SYMBOL_TYPE_TAG:
            udc->setUserValue(key, std::string((*itr).AsSymbol()));
            break;

        default:
        break;

    }

}




class SetMouseInputRangeRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    SetMouseInputRangeRequestHandler()
        : OscReceivingDevice::RequestHandler("/osgga/mouse/set_input_range")
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


class SetMouseOrientationRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    SetMouseOrientationRequestHandler()
        : OscReceivingDevice::RequestHandler("/osgga/mouse/y_orientation_increasing_upwards")
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


class KeyCodeRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    KeyCodeRequestHandler(bool handle_key_press)
        : OscReceivingDevice::RequestHandler(std::string("/osgga/key/") + ((handle_key_press) ? "press" : "release"))
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


class KeyPressAndReleaseRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    KeyPressAndReleaseRequestHandler()
        : OscReceivingDevice::RequestHandler("/osgga/key/press_and_release")
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




class MouseMotionRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    MouseMotionRequestHandler()
        : OscReceivingDevice::RequestHandler("/osgga/mouse/motion")
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


class MouseScrollRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    MouseScrollRequestHandler()
        : OscReceivingDevice::RequestHandler("/osgga/mouse/scroll")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        
        try {
            osc::int32 sm(osgGA::GUIEventAdapter::SCROLL_NONE);
            float delta_x(0.0f), delta_y(0.0f);
            
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> sm >> delta_x >> delta_y >> osc::EndMessage;
        
            if (sm != osgGA::GUIEventAdapter::SCROLL_NONE)
                getDevice()->getEventQueue()->mouseScroll((osgGA::GUIEventAdapter::ScrollingMotion)sm, getLocalTime());
            
            if ((delta_x != 0.0f) || (delta_y != 0.0f))
                getDevice()->getEventQueue()->mouseScroll2D(delta_x, delta_y, getLocalTime());
            
            return true;
        }
        catch (osc::Exception e) {
            handleException(e);
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(int scroll_motion, float x, float y): send mouse scroll-motion";
    }
};



class MouseButtonToggleRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    MouseButtonToggleRequestHandler(const std::string& btn_name, MouseMotionRequestHandler* mm_handler)
        : OscReceivingDevice::RequestHandler("/osgga/mouse/toggle/"+btn_name)
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


class MouseButtonRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    enum Mode { PRESS, RELEASE, DOUBLE_PRESS};
    
    MouseButtonRequestHandler(Mode mode)
        : OscReceivingDevice::RequestHandler("")
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


class PenPressureRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    PenPressureRequestHandler()
        : OscReceivingDevice::RequestHandler("/osgga/pen/pressure")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        try {
            float pressure(0.0f);
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> pressure >> osc::EndMessage;
        
            getDevice()->getEventQueue()->penPressure(pressure, getLocalTime());
            
            return true;
        }
        catch (osc::Exception e) {
            handleException(e);
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(float pressure): send pen pressure";
    }
};

class PenProximityRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    PenProximityRequestHandler(bool handle_enter)
        : OscReceivingDevice::RequestHandler(std::string("/osgga/pen/proximity/") + ((handle_enter) ? std::string("enter") : std::string("leave")))
        , _handleEnter(handle_enter)
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        try {
            osc::int32 pt(osgGA::GUIEventAdapter::UNKNOWN);
            
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> pt >> osc::EndMessage;
        
            getDevice()->getEventQueue()->penProximity((osgGA::GUIEventAdapter::TabletPointerType)pt, _handleEnter, getLocalTime());
            
            return true;
        }
        catch (osc::Exception e) {
            handleException(e);
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(int table_pointer_type): send pen proximity " << (_handleEnter ? "enter":"leave");
    }
private:
    bool _handleEnter;
};


class PenOrientationRequestHandler : public OscReceivingDevice::RequestHandler {
public:
    PenOrientationRequestHandler()
        : OscReceivingDevice::RequestHandler("/osgga/pen/orientation")
    {
    }
    
    virtual bool operator()(const std::string& request_path, const std::string& full_request_path, const osc::ReceivedMessage& m)
    {
        try {
            float rotation(0.0f), tilt_x(0.0f), tilt_y(0.0f);
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> rotation >> tilt_x >> tilt_y >> osc::EndMessage;
        
            getDevice()->getEventQueue()->penOrientation(tilt_x, tilt_y, rotation, getLocalTime());
            
            return true;
        }
        catch (osc::Exception e) {
            handleException(e);
        }
        return false;
    }
    
    virtual void describeTo(std::ostream& out) const
    {
        out << getRequestPath() << "(float rotation, float tilt_x, float tilt_y): send pen orientation";
    }
};


} // end of namespace



OscReceivingDevice::OscReceivingDevice(const std::string& server_address, int listening_port)
    : osgGA::Device()
    , OpenThreads::Thread()
    , osc::OscPacketListener()
    , _listeningAddress(server_address)
    , _listeningPort(listening_port)
    , _socket(NULL)
    , _map()
    , _lastMsgId(0)
{
    setCapabilities(RECEIVE_EVENTS);
    OSG_NOTICE << "OscDevice :: listening on " << server_address << ":" << listening_port << " ";
    #ifdef OSC_HOST_LITTLE_ENDIAN
        OSG_NOTICE << "(little endian)";
    #elif OSC_HOST_BIG_ENDIAN
        OSG_NOTICE << "(big endian)";
    #endif
    OSG_NOTICE << std::endl;
    
    _socket = new UdpListeningReceiveSocket(IpEndpointName( server_address.c_str(), listening_port ), this);
    
    addRequestHandler(new OscDevice::KeyCodeRequestHandler(false));
    addRequestHandler(new OscDevice::KeyCodeRequestHandler(true));
    addRequestHandler(new OscDevice::KeyPressAndReleaseRequestHandler());
    
    addRequestHandler(new OscDevice::SetMouseInputRangeRequestHandler());
    addRequestHandler(new OscDevice::SetMouseOrientationRequestHandler());
    
    OscDevice::MouseMotionRequestHandler* mm_handler = new OscDevice::MouseMotionRequestHandler();
    addRequestHandler(mm_handler);
    addRequestHandler(new OscDevice::MouseButtonRequestHandler(OscDevice::MouseButtonRequestHandler::PRESS));
    addRequestHandler(new OscDevice::MouseButtonRequestHandler(OscDevice::MouseButtonRequestHandler::RELEASE));
    addRequestHandler(new OscDevice::MouseButtonRequestHandler(OscDevice::MouseButtonRequestHandler::DOUBLE_PRESS));
    addRequestHandler(new OscDevice::MouseScrollRequestHandler());
    
    addRequestHandler(new OscDevice::MouseButtonToggleRequestHandler("1", mm_handler));
    addRequestHandler(new OscDevice::MouseButtonToggleRequestHandler("2", mm_handler));
    addRequestHandler(new OscDevice::MouseButtonToggleRequestHandler("3", mm_handler));
    
    addRequestHandler(new OscDevice::PenPressureRequestHandler());
    addRequestHandler(new OscDevice::PenOrientationRequestHandler());
    addRequestHandler(new OscDevice::PenProximityRequestHandler(true));
    addRequestHandler(new OscDevice::PenProximityRequestHandler(false));
    
    addRequestHandler(new OscDevice::StandardRequestHandler("/osg/set_user_value", true));
    
    addRequestHandler(new OscDevice::StandardRequestHandler("", false));
    setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_LOW);
    start();
}


OscReceivingDevice::~OscReceivingDevice()
{
    _socket->AsynchronousBreak();
    join();
    delete _socket;
}


void OscReceivingDevice::run()
{
    _socket->Run();
}


void OscReceivingDevice::ProcessMessage( const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint )
{
    std::string in_request_path(m.AddressPattern());
    
    if (in_request_path == "/osc/msg_id")
        return;
    
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
                // OSG_INFO << "OscDevice :: handling " << mangled_path << " with " << i->second << std::endl;
                
                if (i->second->operator()(mangled_path, in_request_path, m) && !handled)
                    handled = true;
            }
            
        }
    } while ((pos != std::string::npos) && (pos > 0) && !handled);

}

void OscReceivingDevice::ProcessBundle( const osc::ReceivedBundle& b,
                           const IpEndpointName& remoteEndpoint )
{
    // find msg-id
    MsgIdType msg_id(0);
    
    for( osc::ReceivedBundle::const_iterator i = b.ElementsBegin(); i != b.ElementsEnd(); ++i ){
        const osc::ReceivedMessage& m = osc::ReceivedMessage(*i);
        std::string address_pattern(m.AddressPattern());
        if(address_pattern == "/osc/msg_id")
        {
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> msg_id;
            break;
        }
    }
    if (msg_id)
    {
        osg::Timer_t now(osg::Timer::instance()->tick());
        if (osg::Timer::instance()->delta_s(_lastMsgTimeStamp, now) > 0.5) {
            OSG_INFO << "OscReceiver :: resetting msg_id to 0 " << std::endl;
            _lastMsgId = 0;
        }
        _lastMsgTimeStamp = now;
        
        if (msg_id <= _lastMsgId) {
            // already handled
            // OSG_WARN << "OscReceiver :: message with lower id received: " << msg_id << std::endl;
            return;
        }
        else {
            if ((msg_id > _lastMsgId+1) && (_lastMsgId > 0)) {
                OSG_WARN << "OscReceiver :: missed " << (msg_id - _lastMsgId) << " messages, (" << msg_id << "/" << _lastMsgId << ")" << std::endl;
            }
            _lastMsgId = msg_id;
        }
    }
        
    
    for( osc::ReceivedBundle::const_iterator i = b.ElementsBegin(); i != b.ElementsEnd(); ++i ){
        if( i->IsBundle() )
            ProcessBundle( osc::ReceivedBundle(*i), remoteEndpoint );
        else
        {
            ProcessMessage( osc::ReceivedMessage(*i), remoteEndpoint );
        }
    }
}


void OscReceivingDevice::ProcessPacket( const char *data, int size, const IpEndpointName& remoteEndpoint )
{
    try {
        osc::OscPacketListener::ProcessPacket(data, size, remoteEndpoint);
    }
    catch(const osc::Exception& e) {
        OSG_WARN << "OscDevice :: could not process UDP-packet: " << e.what() << std::endl;
    }
    catch(...) {
        OSG_WARN << "OscDevice :: could not process UDP-packet because of an exception!" << std::endl;
    }
    
    if (_userDataEvent.valid())
    {
        char address[IpEndpointName::ADDRESS_AND_PORT_STRING_LENGTH];

        
        remoteEndpoint.AddressAndPortAsString(address);
        
        _userDataEvent->setUserValue("osc/remote_end_point", std::string(address));
        
        getEventQueue()->addEvent(_userDataEvent.get());
        _userDataEvent = NULL;
    }
}

void OscReceivingDevice::addRequestHandler(RequestHandler* handler)
{
    if (handler)
    {
        _map.insert(std::make_pair(handler->getRequestPath(), handler));
        handler->setDevice(this);
    }
}

void OscReceivingDevice::describeTo(std::ostream& out) const
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
