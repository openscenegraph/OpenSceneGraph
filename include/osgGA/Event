/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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

#ifndef OSGGA_EVENT
#define OSGGA_EVENT 1

#include <osgGA/Export>
#include <osg/Object>

namespace osgGA {

// forward declare
class GUIEventAdapter;

/** Base Event class.*/
class OSGGA_EXPORT Event : public osg::Object
{
public:
    Event();

    Event(const Event& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Object(osgGA, Event);

    virtual GUIEventAdapter* asGUIEventAdapter() { return 0; }
    virtual const GUIEventAdapter* asGUIEventAdapter() const { return 0; }


    /** Set whether this event has been handled by an event handler or not.*/
    void setHandled(bool handled) const { _handled = handled; }

    /** Get whether this event has been handled by an event handler or not.*/
    bool getHandled() const { return _handled; }


    /** set time in seconds of event. */
    void setTime(double time) { _time = time; }

    /** get time in seconds of event. */
    double getTime() const { return _time; }

protected:
    virtual ~Event() {}

    mutable bool        _handled;
    double              _time;
};

}

#endif
