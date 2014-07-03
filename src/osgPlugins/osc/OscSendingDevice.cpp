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


#include "OscSendingDevice.hpp"
#include "osc/OscHostEndianness.h"
#include <osg/UserDataContainer>
#include <osg/ValueObject>
#include <osg/Math>

static const unsigned long BUFFER_SIZE = 2048;

OscSendingDevice::OscSendingDevice(const std::string& address, int port, unsigned int num_messages_per_event, unsigned int delay_between_sends_in_millisecs)
    : osgGA::Device()
    , _transmitSocket(IpEndpointName(address.c_str(), port))
    , _buffer(new  char[BUFFER_SIZE])
    , _oscStream(_buffer, BUFFER_SIZE)
    , _numMessagesPerEvent(osg::maximum(1u,num_messages_per_event))
    , _delayBetweenSendsInMilliSecs( (_numMessagesPerEvent > 1) ? delay_between_sends_in_millisecs : 0)
{
    setCapabilities(SEND_EVENTS);

    OSG_NOTICE << "OscDevice :: sending events to " << address << ":" << port << " ";
    #ifdef OSC_HOST_LITTLE_ENDIAN
        OSG_NOTICE << "(little endian)";
    #elif OSC_HOST_BIG_ENDIAN
        OSG_NOTICE << "(big endian)";
    #endif
    OSG_NOTICE << " (" << _numMessagesPerEvent << "msgs/event, " << _delayBetweenSendsInMilliSecs << "ms delay between msgs)";
    OSG_NOTICE << std::endl;

}


OscSendingDevice::~OscSendingDevice()
{
    delete[] (_buffer);
}

void OscSendingDevice::sendEvent(const osgGA::GUIEventAdapter &ea)
{
    static osc::int64 msg_id(0);
    bool msg_sent(false);
    unsigned int num_messages = _numMessagesPerEvent;

    if((ea.getEventType() == osgGA::GUIEventAdapter::DRAG) || (ea.getEventType() == osgGA::GUIEventAdapter::MOVE))
        num_messages = 1;

    for(unsigned int i = 0; i < num_messages; ++i) {
        msg_sent = sendEventImpl(ea, msg_id);
        if ((_delayBetweenSendsInMilliSecs > 0) && (i < num_messages-1))
            OpenThreads::Thread::microSleep(1000 * _delayBetweenSendsInMilliSecs);
    }
    if (msg_sent)
        msg_id++;
}


bool OscSendingDevice::sendEventImpl(const osgGA::GUIEventAdapter &ea, MsgIdType msg_id)
{
    bool do_send(false);
    switch(ea.getEventType())
    {
        case osgGA::GUIEventAdapter::RESIZE:
            beginBundle(msg_id);
            _oscStream << osc::BeginMessage("/osgga/resize") << ea.getWindowX() << ea.getWindowY() << ea.getWindowWidth() << ea.getWindowHeight() << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::SCROLL:
            beginSendInputRange(ea, msg_id);
            _oscStream << osc::BeginMessage("/osgga/mouse/scroll") << ea.getScrollingMotion() << ea.getScrollingDeltaX() << ea.getScrollingDeltaY() << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::PEN_PRESSURE:
            beginBundle(msg_id);
            _oscStream
                << osc::BeginMessage("/osgga/pen/pressure")
                << ea.getPenPressure()
                << osc::EndMessage;
            _oscStream << osc::EndBundle;

            do_send = true;
            break;

        case osgGA::GUIEventAdapter::PEN_ORIENTATION:
            beginBundle(msg_id);
            _oscStream
                << osc::BeginMessage("/osgga/pen/orientation")
                << ea.getPenRotation()
                << ea.getPenTiltX()
                << ea.getPenTiltY()
                << osc::EndMessage;
            do_send = true;
            _oscStream << osc::EndBundle;
            break;

        case osgGA::GUIEventAdapter::PEN_PROXIMITY_ENTER:
            beginBundle(msg_id);
            _oscStream
                << osc::BeginMessage("/osgga/pen/proximity/enter")
                << ea.getTabletPointerType()
                << osc::EndMessage;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::PEN_PROXIMITY_LEAVE:
            beginBundle(msg_id);
            _oscStream
                << osc::BeginMessage("/osgga/pen/proximity/leave")
                << ea.getTabletPointerType()
                << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::PUSH:
            beginSendInputRange(ea, msg_id);
            _oscStream << osc::BeginMessage("/osgga/mouse/press") << ea.getX() << ea.getY() << getButtonNum(ea)  << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::RELEASE:
            beginSendInputRange(ea, msg_id);
            _oscStream << osc::BeginMessage("/osgga/mouse/release") << ea.getX() << ea.getY() << getButtonNum(ea)  << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::DOUBLECLICK:
            beginSendInputRange(ea, msg_id);
            _oscStream << osc::BeginMessage("/osgga/mouse/doublepress") << ea.getX() << ea.getY() << getButtonNum(ea) << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::MOVE:
        case osgGA::GUIEventAdapter::DRAG:
            beginSendInputRange(ea, msg_id);
            _oscStream << osc::BeginMessage("/osgga/mouse/motion") << ea.getX() << ea.getY() << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::KEYDOWN:
            beginBundle(msg_id);
            _oscStream << osc::BeginMessage("/osgga/key/press") << ea.getKey() << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::KEYUP:
            beginBundle(msg_id);
            _oscStream << osc::BeginMessage("/osgga/key/release") << ea.getKey() << osc::EndMessage;
            _oscStream << osc::EndBundle;
            do_send = true;
            break;

        case osgGA::GUIEventAdapter::USER:
            if (ea.getUserDataContainer())
            {
                std::string key = ea.getUserDataContainer()->getName();
                if (key.empty()) key = ea.getName();
                if (key.empty()) key = "user_data";

                sendUserDataContainer(transliterateKey(key), ea.getUserDataContainer(), true, msg_id);

                do_send = true;
            }

        default:
            break;

    }

    if (do_send)
    {
        OSG_INFO << "OscDevice :: sending event per OSC " << std::endl;

        _transmitSocket.Send( _oscStream.Data(), _oscStream.Size() );
        _oscStream.Clear();
    }

    return do_send;
}

int OscSendingDevice::getButtonNum(const osgGA::GUIEventAdapter& ea)
{
    switch(ea.getButton())
    {
        case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
            return 1;
            break;
        case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
            return 2;
            break;
        case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
            return 3;
            break;
        default:
            return -1;
    }
    return -1;
}

void OscSendingDevice::beginBundle(MsgIdType msg_id)
{
    _oscStream << osc::BeginBundle();
    _oscStream << osc::BeginMessage("/osc/msg_id") << msg_id << osc::EndMessage;
}

void OscSendingDevice::beginSendInputRange(const osgGA::GUIEventAdapter &ea, MsgIdType msg_id)
{
    beginBundle(msg_id);
    _oscStream << osc::BeginMessage("/osgga/mouse/set_input_range") << ea.getXmin() << ea.getYmin() << ea.getXmax() << ea.getYmax() << osc::EndMessage;
    _oscStream << osc::BeginMessage("/osgga/mouse/y_orientation_increasing_upwards") << (bool)(ea.getMouseYOrientation() == osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS)  << osc::EndMessage;
}


class OscSendingDeviceGetValueVisitor : public osg::ValueObject::GetValueVisitor {
public:
    OscSendingDeviceGetValueVisitor(osc::OutboundPacketStream& stream)
        : osg::ValueObject::GetValueVisitor()
        , _stream(stream)
    {
    }

    virtual void apply(bool value)                  { _stream << value; }
    virtual void apply(char value)                  { _stream << value; }
    virtual void apply(unsigned char value)         { _stream << value; }
    virtual void apply(short value)                 { _stream << value; }
    virtual void apply(unsigned short value)        { _stream << value; }
    virtual void apply(int value)                   { _stream << value; }
    virtual void apply(unsigned int value)          { _stream << static_cast<osc::int32>(value); }
    virtual void apply(float value)                 { _stream << value; }
    virtual void apply(double value)                { _stream << value; }
    virtual void apply(const std::string& value)    { _stream << value.c_str(); }
    virtual void apply(const osg::Vec2f& value)     { _stream << value[0] << value[1]; }
    virtual void apply(const osg::Vec3f& value)     { _stream << value[0] << value[1] << value[2]; }
    virtual void apply(const osg::Vec4f& value)     { _stream << value[0] << value[1] << value[2] << value[3]; }
    virtual void apply(const osg::Vec2d& value)     { _stream << value[0] << value[1]; }
    virtual void apply(const osg::Vec3d& value)     { _stream << value[0] << value[1] << value[2]; }
    virtual void apply(const osg::Vec4d& value)     { _stream << value[0] << value[1] << value[2] << value[3]; }
    virtual void apply(const osg::Quat& value)      { _stream << value[0] << value[1] << value[2] << value[3]; }
    virtual void apply(const osg::Plane& value)     { _stream << value[0] << value[1] << value[2] << value[3]; }
    virtual void apply(const osg::Matrixf& value)   { for(unsigned int i=0; i<16; ++i) _stream << (value.ptr())[i]; }
    virtual void apply(const osg::Matrixd& value)   { for(unsigned int i=0; i<16; ++i) _stream << (value.ptr())[i]; }

    virtual ~OscSendingDeviceGetValueVisitor() {}

private:
    osc::OutboundPacketStream& _stream;

};

std::string OscSendingDevice::transliterateKey(const std::string& key) const
{
    std::string result;
    result.reserve(key.size());
    for(std::string::const_iterator itr=key.begin();
        itr!=key.end();
        ++itr)
    {
        if ((*itr == ' ') || (*itr == 9))
            result += "-";
        else if ((*itr >= 'A') && (*itr <= 'Z'))
            result += tolower(*itr);
        else if (((*itr >= '0') && (*itr <= '9'))  || ((*itr >= 'a') && (*itr <= 'z')) || (*itr == '-') || (*itr == '/') || (*itr == '_'))
            result += *itr;
    }
    return result;
}

void OscSendingDevice::sendUserDataContainer(const std::string& key, const osg::UserDataContainer* udc, bool asBundle, MsgIdType msg_id)
{
    if (asBundle) {
        beginBundle(msg_id);
    }

    OscSendingDeviceGetValueVisitor gvv(_oscStream);

    unsigned int num_objects = udc->getNumUserObjects();
    for(unsigned int i = 0; i < num_objects; ++i)
    {
        const osg::Object* o = udc->getUserObject(i);
        const osg::UserDataContainer* child_udc = dynamic_cast<const osg::UserDataContainer*>(o);
        if (child_udc)
        {
            std::string new_key = key + "/" + (child_udc->getName().empty() ? "user_data" : child_udc->getName());
            sendUserDataContainer(transliterateKey(key), child_udc, false, msg_id);
        }
        else if (const osg::ValueObject* vo = dynamic_cast<const osg::ValueObject*>(o))
        {
            _oscStream << osc::BeginMessage(std::string("/" + key + "/" + transliterateKey(vo->getName())).c_str());
            vo->get(gvv);
            _oscStream << osc::EndMessage;
        }
    }

    if (asBundle)
        _oscStream << osc::EndBundle;

}
