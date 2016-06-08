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


#include <osg/Geode>
#include <osg/ScriptEngine>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/ValueObject>
#include <osg/io_utils>

#include <osgGA/Widget>
#include <osgGA/EventVisitor>
#include <osgGA/GUIActionAdapter>

using namespace osgGA;

Widget::Widget():
    _focusBehaviour(FOCUS_FOLLOWS_POINTER),
    _hasEventFocus(false),
    _graphicsInitialized(false)

{
    setNumChildrenRequiringEventTraversal(1);
}

Widget::Widget(const Widget& widget, const osg::CopyOp& copyop):
    osg::Group(widget, copyop),
    _focusBehaviour(widget._focusBehaviour),
    _hasEventFocus(false),
    _graphicsInitialized(false)
{
    setNumChildrenRequiringEventTraversal(1);
}

void Widget::setExtents(const osg::BoundingBoxf& bb)
{
    _extents = bb;
}

void Widget::updateFocus(osg::NodeVisitor& nv)
{
    osgGA::EventVisitor* ev = nv.asEventVisitor();
    osgGA::GUIActionAdapter* aa = ev ? ev->getActionAdapter() : 0;
    if (ev && aa)
    {
        osgGA::EventQueue::Events& events = ev->getEvents();
        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* ea = (*itr)->asGUIEventAdapter();
            if (ea)
            {
                bool previousFocus = _hasEventFocus;
                if (_focusBehaviour==CLICK_TO_FOCUS)
                {
                    if (ea->getEventType()==osgGA::GUIEventAdapter::PUSH)
                    {
                        int numButtonsPressed = 0;
                        if (ea->getButtonMask()&osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) ++numButtonsPressed;
                        if (ea->getButtonMask()&osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) ++numButtonsPressed;
                        if (ea->getButtonMask()&osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) ++numButtonsPressed;

                        if (numButtonsPressed==1)
                        {
                            osgUtil::LineSegmentIntersector::Intersections intersections;
                            bool withinWidget = aa->computeIntersections(*ea, nv.getNodePath(), intersections);
                            if (withinWidget) _hasEventFocus = true;
                            else _hasEventFocus = false;
                        }
                    }
                }
                else if (_focusBehaviour==FOCUS_FOLLOWS_POINTER)
                {
                    bool checkWithinWidget = false;
                    if (!_hasEventFocus)
                    {
                        checkWithinWidget = (ea->getEventType()!=osgGA::GUIEventAdapter::FRAME) && ea->getButtonMask()==0;
                    }
                    else
                    {
                        // if mouse move or mouse release check to see if mouse still within widget to retain focus
                        if (ea->getEventType()==osgGA::GUIEventAdapter::MOVE)
                        {
                            checkWithinWidget = true;
                        }
                        else if (ea->getEventType()==osgGA::GUIEventAdapter::RELEASE)
                        {
                            // if no buttons pressed then check
                            if (ea->getButtonMask()==0) checkWithinWidget = true;
                        }
                    }

                    if (checkWithinWidget)
                    {
                        osgUtil::LineSegmentIntersector::Intersections intersections;
                        bool withinWidget = aa->computeIntersections(*ea, nv.getNodePath(), intersections);

                        _hasEventFocus = withinWidget;
                    }
                }

                if (previousFocus != _hasEventFocus)
                {
                    if (_hasEventFocus)
                    {
                        enter();
#if 0
                        if (view->getCameraManipulator())
                        {
                            view->getCameraManipulator()->finishAnimation();
                            view->requestContinuousUpdate( false );
                        }
#endif
                    }
                    else
                    {
                        leave();
                    }
                }

            }
        }
    }
}

void Widget::setHasEventFocus(bool focus)
{
    if (_hasEventFocus == focus) return;

    _hasEventFocus = focus;

    if (_hasEventFocus) enter();
    else leave();
}

bool Widget::getHasEventFocus() const
{
    return _hasEventFocus;
}

void Widget::enter()
{
    osg::CallbackObject* co = osg::getCallbackObject(this, "enter");
    if (co)
    {
        co->run(this);
    }
    else
    {
        enterImplementation();
    }
}

void Widget::enterImplementation()
{
    OSG_NOTICE<<"enter()"<<std::endl;
}

void Widget::leave()
{
    osg::CallbackObject* co = osg::getCallbackObject(this, "leave");
    if (co)
    {
        co->run(this);
    }
    else
    {
        leaveImplementation();
    }
}

void Widget::leaveImplementation()
{
    OSG_NOTICE<<"leave()"<<std::endl;
}

void Widget::traverse(osg::NodeVisitor& nv)
{
    osg::CallbackObject* co = osg::getCallbackObject(this, "traverse");
    if (co)
    {
        // currently lua scripting takes a ref count so messes up handling of NodeVisitor's created on stack,
        // so don't attempt to call the sctipt.
        if (nv.referenceCount()==0)
        {
            traverseImplementation(nv);
            return;
        }

        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(&nv);
        co->run(this, inputParameters, outputParameters);
    }
    else
    {
        traverseImplementation(nv);
    }
}

void Widget::traverseImplementation(osg::NodeVisitor& nv)
{
    if (!_graphicsInitialized && nv.getVisitorType()!=osg::NodeVisitor::CULL_VISITOR) createGraphics();


    osgGA::EventVisitor* ev = nv.asEventVisitor();
    if (ev)
    {
        updateFocus(nv);

        if (getHasEventFocus())
        {
            // signify that event has been taken by widget with focus
            ev->setEventHandled(true);

            osgGA::EventQueue::Events& events = ev->getEvents();
            for(osgGA::EventQueue::Events::iterator itr = events.begin();
                itr != events.end();
                ++itr)
            {
                if (handle(ev, itr->get()))
                {
                    (*itr)->setHandled(true);
                }
            }
        }
    }
    else
    {
        osg::Group::traverse(nv);
    }
}

bool Widget::handle(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    osg::CallbackObject* co = osg::getCallbackObject(this, "handle");
    if (co)
    {
        // currently lua scripting takes a ref count so messes up handling of NodeVisitor's created on stack,
        // so don't attempt to call the sctipt.
        if (ev->referenceCount()==0)
        {
            return handleImplementation(ev, event);
        }

        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(ev);
        inputParameters.push_back(event);
        if (co->run(this, inputParameters, outputParameters))
        {
            if (outputParameters.size()>=1)
            {
                osg::BoolValueObject* bvo = dynamic_cast<osg::BoolValueObject*>(outputParameters[0].get());
                if (bvo)
                {
                    return bvo->getValue();
                }
                return false;
            }
        }
        return false;
    }
    else
    {
        return handleImplementation(ev, event);
    }
}

bool Widget::handleImplementation(osgGA::EventVisitor* /*ev*/, osgGA::Event* /*event*/)
{
    return false;
}

bool Widget::computePositionInLocalCoordinates(osgGA::EventVisitor* ev, osgGA::GUIEventAdapter* event, osg::Vec3& localPosition) const
{
    osgGA::GUIActionAdapter* aa = ev ? ev->getActionAdapter() : 0;
    osgUtil::LineSegmentIntersector::Intersections intersections;
    if (aa && aa->computeIntersections(*event, ev->getNodePath(), intersections))
    {
        localPosition = intersections.begin()->getLocalIntersectPoint();

        return (_extents.contains(localPosition, 1e-6));
    }
    else
    {
        return false;
    }
}


void Widget::createGraphics()
{
    osg::CallbackObject* co = osg::getCallbackObject(this, "createGraphics");
    if (co)
    {
        co->run(this);
    }
    else
    {
        createGraphicsImplementation();
    }

}

void Widget::createGraphicsImplementation()
{
    _graphicsInitialized = true;
}

osg::BoundingSphere Widget::computeBound() const
{
    if (_extents.valid()) return osg::BoundingSphere(_extents);
    else return osg::Group::computeBound();
}
