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

#ifndef OSGGA_EVENTVISITOR
#define OSGGA_EVENTVISITOR 1

#include <osg/NodeVisitor>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/LOD>
#include <osg/Switch>
#include <osg/LightSource>
#include <osg/Transform>
#include <osg/Projection>
#include <osg/OccluderNode>
#include <osg/ScriptEngine>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIEventHandler>
#include <osgGA/GUIActionAdapter>
#include <osgGA/EventQueue>

namespace osgGA {

/**
 * Basic EventVisitor implementation for animating a scene.
 * This visitor traverses the scene graph, calling each nodes appCallback if
 * it exists.
 */
class OSGGA_EXPORT EventVisitor : public osg::NodeVisitor
{
    public:

        EventVisitor();
        virtual ~EventVisitor();

        META_NodeVisitor(osgGA, EventVisitor)

        /** Convert 'this' into a osgGA::EventVisitor pointer if Object is a osgGA::EventVisitor, otherwise return 0.
          * Equivalent to dynamic_cast<osgGA::EventVisitor*>(this).*/
        virtual osgGA::EventVisitor* asEventVisitor() { return this; }

        /** convert 'const this' into a const osgGA::EventVisitor pointer if Object is a osgGA::EventVisitor, otherwise return 0.
          * Equivalent to dynamic_cast<const osgGA::EventVisitor*>(this).*/
        virtual const osgGA::EventVisitor* asEventVisitor() const { return this; }

        void setActionAdapter(osgGA::GUIActionAdapter* actionAdapter) { _actionAdapter=actionAdapter; }

        osgGA::GUIActionAdapter* getActionAdapter() { return _actionAdapter; }

        const osgGA::GUIActionAdapter* getActionAdapter() const { return _actionAdapter; }

        void addEvent(Event* event);
        void removeEvent(Event* event);

        void setEventHandled(bool handled) { _handled = handled; }
        bool getEventHandled() const { return _handled; }

        void setEvents(const EventQueue::Events& events) { _events = events; }
        EventQueue::Events& getEvents() { return _events; }
        const EventQueue::Events& getEvents() const { return _events; }

    public:

        virtual void reset();

        /** During traversal each type of node calls its callbacks and its children traversed. */
        virtual void apply(osg::Node& node)         { handle_callbacks_and_traverse(node); }


        virtual void apply(osg::Drawable& drawable)
        {
            osg::Callback* callback = drawable.getEventCallback();
            if (callback)
            {
                osgGA::EventHandler* eh = callback->asEventHandler();
                if (eh)
                {
                    callback->run(&drawable,this);
                }
                else
                {
                    osg::DrawableEventCallback* drawable_callback = callback->asDrawableEventCallback();
                    osg::NodeCallback* node_callback = callback->asNodeCallback();
                    osg::CallbackObject* callback_object = callback->asCallbackObject();

                    if (drawable_callback) drawable_callback->event(this,&drawable);
                    if (node_callback) (*node_callback)(&drawable, this);
                    if (callback_object)  callback_object->run(&drawable, this);

                    if (!drawable_callback && !node_callback && !callback_object) callback->run(&drawable, this);
                }
            }

            handle_callbacks(drawable.getStateSet());
        }

        // The following overrides are technically redundant as the default implementation would eventually trickle down to
        // apply(osg::Node&); - however defining these explicitly should save a couple of virtual function calls
        virtual void apply(osg::Geode& node)        { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::Billboard& node)    { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::LightSource& node)  { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::Group& node)        { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::Transform& node)    { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::Projection& node)   { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::Switch& node)       { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::LOD& node)          { handle_callbacks_and_traverse(node); }
        virtual void apply(osg::OccluderNode& node) { handle_callbacks_and_traverse(node); }


    protected:

        /** Prevent unwanted copy operator.*/
        EventVisitor& operator = (const EventVisitor&) { return *this; }

        inline void handle_callbacks(osg::StateSet* stateset)
        {
            if (stateset && stateset->requiresEventTraversal())
            {
                stateset->runEventCallbacks(this);
            }
        }

        inline void handle_callbacks_and_traverse(osg::Node& node)
        {
            handle_callbacks(node.getStateSet());

            osg::Callback* callback = node.getEventCallback();
            if (callback) callback->run(&node,this);
            else if (node.getNumChildrenRequiringEventTraversal()>0) traverse(node);
        }


        osgGA::GUIActionAdapter*        _actionAdapter;

        osg::ref_ptr<GUIEventAdapter>   _accumulateEventState;

        bool                            _handled;
        EventQueue::Events              _events;
};

}

#endif

