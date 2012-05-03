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
#include <osgGA/EventVisitor>

#include <algorithm>

using namespace osg;
using namespace osgGA;

EventVisitor::EventVisitor()
:    NodeVisitor(EVENT_VISITOR,TRAVERSE_ACTIVE_CHILDREN),
    _handled(false)
{
}


EventVisitor::~EventVisitor()
{
}

void EventVisitor::addEvent(GUIEventAdapter* event)
{
    _events.push_back(event);
}

void EventVisitor::removeEvent(GUIEventAdapter* event)
{
    EventList::iterator itr = std::find(_events.begin(),_events.end(),event);
    if (itr!=_events.end()) _events.erase(itr);
}


void EventVisitor::reset()
{
    _events.clear();
    _handled = false;
}
