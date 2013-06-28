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

#include <osgGA/Device>

using namespace osgGA;

Device::Device()
    : osg::Object()
    , _capabilities(UNKNOWN)
{
    setEventQueue(new EventQueue);
}

Device::Device(const Device& es, const osg::CopyOp& copyop):
    osg::Object(es,copyop)
{
    setEventQueue(new EventQueue);
}

void Device::sendEvent(const GUIEventAdapter& /*event*/)
{
    OSG_WARN << "Device::sendEvent not implemented!" << std::endl;
}


void Device::sendEvents(const EventQueue::Events& events)
{
    for(EventQueue::Events::const_iterator i = events.begin(); i != events.end(); i++)
    {
        sendEvent(**i);
    }
}


Device::~Device()
{
}
