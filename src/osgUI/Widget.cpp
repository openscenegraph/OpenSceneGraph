/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#include <osgUI/Widget>
#include <osgGA/EventVisitor>
#include <osgGA/GUIActionAdapter>

using namespace osgUI;

Widget::Widget():
    _focusBehaviour(FOCUS_FOLLOWS_POINTER),
    _hasEventFocus(false),
    _graphicsInitialized(false),
    _visible(true),
    _enabled(true)
{
    setNumChildrenRequiringEventTraversal(1);
}

Widget::Widget(const Widget& widget, const osg::CopyOp& copyop):
    osg::Group(),
    _focusBehaviour(widget._focusBehaviour),
    _hasEventFocus(false),
    _graphicsInitialized(false),
    _alignmentSettings(osg::clone(widget._alignmentSettings.get(), copyop)),
    _frameSettings(osg::clone(widget._frameSettings.get(), copyop)),
    _textSettings(osg::clone(widget._textSettings.get(), copyop)),
    _visible(widget._visible),
    _enabled(widget._enabled)
{
    setNumChildrenRequiringEventTraversal(1);
}

void Widget::setExtents(const osg::BoundingBoxf& bb)
{
    _extents = bb;
}

void Widget::updateFocus(osg::NodeVisitor& nv)
{
    osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(&nv);
    osgGA::GUIActionAdapter* aa = ev ? ev->getActionAdapter() : 0;
    if (ev && aa)
    {
        // OSG_NOTICE<<"updateFocus"<<std::endl;

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
    if (!runCallbacks("enter")) enterImplementation();
}

void Widget::enterImplementation()
{
    OSG_NOTICE<<"Widget::enter()"<<std::endl;
}

void Widget::leave()
{
    if (!runCallbacks("leave")) leaveImplementation();
}

void Widget::leaveImplementation()
{
    OSG_NOTICE<<"Widget::leave()"<<std::endl;
}

void Widget::traverse(osg::NodeVisitor& nv)
{
    if (nv.referenceCount()!=0)
    {
        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(&nv);
        if (runCallbacks("traverse",inputParameters, outputParameters)) return;
    }

    traverseImplementation(nv);

}

void Widget::traverseImplementation(osg::NodeVisitor& nv)
{
    if (!_graphicsInitialized && nv.getVisitorType()!=osg::NodeVisitor::CULL_VISITOR) createGraphics();

    osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(&nv);
    if (ev)
    {

        updateFocus(nv);

        // OSG_NOTICE<<"EventTraversal getHasEventFocus()="<<getHasEventFocus()<<std::endl;

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

        osg::Group::traverse(nv);
    }
    else
    {
        if (_graphicsSubgraph.valid()) _graphicsSubgraph->accept(nv);

        osg::Group::traverse(nv);
    }
}

bool Widget::handle(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    // currently lua scripting takes a ref count so messes up handling of NodeVisitor's created on stack,
    // so don't attempt to call the sctipt.
    if (ev->referenceCount()!=0)
    {
        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(ev);
        inputParameters.push_back(event);
        if (runCallbacks("handle",inputParameters, outputParameters))
        {
            if (outputParameters.size()>=1)
            {
                osg::BoolValueObject* bvo = dynamic_cast<osg::BoolValueObject*>(outputParameters[0].get());
                return bvo ? bvo->getValue() : false;
            }
        }
    }

    return handleImplementation(ev, event);
}

bool Widget::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
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

void Widget::dirty()
{
    _graphicsInitialized = false;
}

void Widget::createGraphics()
{
    if (!runCallbacks("createGraphics")) createGraphicsImplementation();
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

void Widget::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_graphicsSubgraph.valid()) _graphicsSubgraph->resizeGLObjectBuffers(maxSize);
    Group::resizeGLObjectBuffers(maxSize);
}


void Widget::releaseGLObjects(osg::State* state) const
{
    if (_graphicsSubgraph.valid()) _graphicsSubgraph->releaseGLObjects(state);
    Group::releaseGLObjects(state);
}
