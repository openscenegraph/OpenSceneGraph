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

#ifndef OSGGA_EVENTHANDLER
#define OSGGA_EVENTHANDLER 1

#include <vector>

#include <osg/Drawable>
#include <osg/ApplicationUsage>

#include <osgGA/Export>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>


namespace osgGA{

/**
EventHandler is base class for adding handling of events, either as node event callback, drawable event callback or an event handler attached directly to the view(er)
*/

class OSGGA_EXPORT EventHandler : public osg::NodeCallback, public osg::DrawableEventCallback
{
public:

        EventHandler() {}
        EventHandler(const EventHandler& eh,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            osg::Object(eh, copyop),
            osg::Callback(eh, copyop),
            osg::NodeCallback(eh, copyop),
            osg::DrawableEventCallback(eh, copyop) {}

        META_Object(osgGA, EventHandler);

        virtual NodeCallback* asNodeCallback() { return osg::NodeCallback::asNodeCallback(); }
        virtual const NodeCallback* asNodeCallback() const { return osg::NodeCallback::asNodeCallback(); }

        virtual DrawableEventCallback* asDrawableEventCallback() { return osg::DrawableEventCallback::asDrawableEventCallback(); }
        virtual const DrawableEventCallback* asDrawableEventCallback() const { return osg::DrawableEventCallback::asDrawableEventCallback(); }

        virtual EventHandler* asEventHandler() { return this; }
        virtual const EventHandler* asEventHandler() const { return this; }

        virtual bool run(osg::Object* object, osg::Object* data)
        {
            osg::Node* node = object->asNode();
            osg::NodeVisitor* nv = data->asNodeVisitor();
            operator()(node, nv);
            return true;
        }

        /** Event traversal node callback method. There is no need to override this method in subclasses of EventHandler as this implementation calls handle(..) for you. */
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

        /** Event traversal drawable callback method. There is no need to override this method in subclasses of EventHandler as this implementation calls handle(..) for you. */
        virtual void event(osg::NodeVisitor* nv, osg::Drawable* drawable);

        /** Handle event. Override the handle(..) method in your event handlers to respond to events. */
        virtual bool handle(osgGA::Event* event, osg::Object* object, osg::NodeVisitor* nv);

        /** Get the user interface usage of this event handler, i.e. keyboard and mouse bindings.*/
        virtual void getUsage(osg::ApplicationUsage&) const {}

protected:

};

}

#endif
