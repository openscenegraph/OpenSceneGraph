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

#ifndef OSGGA_GUIEVENTHANDLER
#define OSGGA_GUIEVENTHANDLER 1

#include <vector>

#include <osg/Drawable>
#include <osg/ApplicationUsage>

#include <osgGA/EventHandler>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>


// #define COMPILE_COMPOSITE_EVENTHANDLER

namespace osgGA{

/**

GUIEventHandler provides a basic interface for any class which wants to handle
a GUI Events.

The GUIEvent is supplied by a GUIEventAdapter. Feedback resulting from the
handle method is supplied by a GUIActionAdapter, which allows the GUIEventHandler
to ask the GUI to take some action in response to an incoming event.

For example, consider a Trackball Viewer class which takes mouse events and
manipulates a scene camera in response. The Trackball Viewer is a GUIEventHandler,
and receives the events via the handle method. If the user 'throws' the model,
the Trackball Viewer class can detect this via the incoming events, and
request that the GUI set up a timer callback to continually redraw the view.
This request is made via the GUIActionAdapter class.

*/

class OSGGA_EXPORT GUIEventHandler : public EventHandler
{
public:

        GUIEventHandler() {}
        GUIEventHandler(const GUIEventHandler& eh,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            osg::Object(eh, copyop),
            osg::Callback(eh, copyop),
            EventHandler(eh, copyop) {}

        META_Object(osgGA,GUIEventHandler);

        /** Handle event. Override the handle(..) method in your event handlers to respond to events. */
        virtual bool handle(osgGA::Event* event, osg::Object* object, osg::NodeVisitor* nv);

        /** Handle events, return true if handled, false otherwise. */
        virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*) { return handle(ea,aa); }

        /** Deprecated, Handle events, return true if handled, false otherwise. */
        virtual bool handle(const GUIEventAdapter&,GUIActionAdapter&) { return false; }

protected:
        virtual ~GUIEventHandler();

};

}

#endif
