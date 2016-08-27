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
#include <osgViewer/View>

#include <algorithm>

using namespace osgUI;

Widget::Widget():
    _focusBehaviour(FOCUS_FOLLOWS_POINTER),
    _hasEventFocus(false),
    _graphicsInitialized(false),
    _autoFillBackground(false),
    _visible(true),
    _enabled(true)
{
    setNumChildrenRequiringEventTraversal(1);
}

Widget::Widget(const Widget& widget, const osg::CopyOp& copyop):
    osg::Group(widget, copyop),
    _focusBehaviour(widget._focusBehaviour),
    _hasEventFocus(false),
    _graphicsInitialized(false),
    _alignmentSettings(osg::clone(widget._alignmentSettings.get(), copyop)),
    _frameSettings(osg::clone(widget._frameSettings.get(), copyop)),
    _textSettings(osg::clone(widget._textSettings.get(), copyop)),
    _autoFillBackground(widget._autoFillBackground),
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
    osgGA::EventVisitor* ev = nv.asEventVisitor();
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
                int numButtonsPressed = 0;
                if (ea->getEventType()==osgGA::GUIEventAdapter::PUSH)
                {
                    if (ea->getButtonMask()&osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) ++numButtonsPressed;
                    if (ea->getButtonMask()&osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) ++numButtonsPressed;
                    if (ea->getButtonMask()&osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) ++numButtonsPressed;
                }

                bool previousFocus = _hasEventFocus;
                if (_focusBehaviour==CLICK_TO_FOCUS)
                {
                    if (ea->getEventType()==osgGA::GUIEventAdapter::PUSH)
                    {
                        if (numButtonsPressed==1)
                        {
                            osg::Vec3d intersection;
                            bool withinWidget = computeExtentsPositionInLocalCoordinates(ev, ea, intersection);

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
                        osg::Vec3d intersection;
                        bool withinWidget = computeExtentsPositionInLocalCoordinates(ev, ea, intersection);

                        _hasEventFocus = withinWidget;
                    }
                }

                if (_hasEventFocus && (ea->getEventType()==osgGA::GUIEventAdapter::PUSH || ea->getEventType()==osgGA::GUIEventAdapter::SCROLL) )
                {
                    osgViewer::View* view = dynamic_cast<osgViewer::View*>(aa);
                    if (view && view->getCameraManipulator())
                    {
                        view->getCameraManipulator()->finishAnimation();
                        view->requestContinuousUpdate( false );
                    }
                }


                if (previousFocus != _hasEventFocus)
                {
                    if (_hasEventFocus)
                    {
                        enter();

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

    osgGA::EventVisitor* ev = nv.asEventVisitor();
    if (ev)
    {
        if (_visible && _enabled)
        {

            updateFocus(nv);

            // OSG_NOTICE<<"EventTraversal getHasEventFocus()="<<getHasEventFocus()<<std::endl;

            // signify that event has been taken by widget with focus

            bool widgetsWithFocusSetHandled = getHasEventFocus();

            osgGA::EventQueue::Events& events = ev->getEvents();
            for(osgGA::EventQueue::Events::iterator itr = events.begin();
                itr != events.end();
                ++itr)
            {
                if (handle(ev, itr->get()) || widgetsWithFocusSetHandled)
                {
                    (*itr)->setHandled(true);
                    ev->setEventHandled(true);
                }
            }

            GraphicsSubgraphMap::iterator itr = _graphicsSubgraphMap.begin();
            while(itr!= _graphicsSubgraphMap.end() && itr->first<=0)
            {
                itr->second->accept(nv);
                ++itr;
            }

            osg::Group::traverse(nv);

            while(itr!= _graphicsSubgraphMap.end())
            {
                itr->second->accept(nv);
                ++itr;
            }

        }
    }
    else if (_visible ||
            (nv.getVisitorType()!=osg::NodeVisitor::UPDATE_VISITOR && nv.getVisitorType()!=osg::NodeVisitor::CULL_VISITOR && nv.getVisitorType()!=osg::NodeVisitor::INTERSECTION_VISITOR) )
    {
        osgUtil::CullVisitor* cv = (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR) ? nv.asCullVisitor() : 0;
        if (cv && _widgetStateSet.valid()) cv->pushStateSet(_widgetStateSet.get());

        GraphicsSubgraphMap::iterator itr = _graphicsSubgraphMap.begin();
        while(itr!= _graphicsSubgraphMap.end() && itr->first<=0)
        {
            itr->second->accept(nv);
            ++itr;
        }

        Group::traverse(nv);

        while(itr!= _graphicsSubgraphMap.end())
        {
            itr->second->accept(nv);
            ++itr;
        }

        if (cv && _widgetStateSet.valid()) cv->popStateSet();
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

bool Widget::handleImplementation(osgGA::EventVisitor* /*ev*/, osgGA::Event* /*event*/)
{
    return false;
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
    osg::BoundingSphere bs;
    if (_extents.valid()) bs.expandBy(_extents);
    bs.expandBy(Group::computeBound());
    return bs;
}

void Widget::resizeGLObjectBuffers(unsigned int maxSize)
{
    for(GraphicsSubgraphMap::iterator itr = _graphicsSubgraphMap.begin();
        itr !=  _graphicsSubgraphMap.end();
        ++itr)
    {
        itr->second->resizeGLObjectBuffers(maxSize);
    }

    Group::resizeGLObjectBuffers(maxSize);
}


void Widget::releaseGLObjects(osg::State* state) const
{
    for(GraphicsSubgraphMap::const_iterator itr = _graphicsSubgraphMap.begin();
        itr !=  _graphicsSubgraphMap.end();
        ++itr)
    {
        itr->second->releaseGLObjects(state);
    }

    Group::releaseGLObjects(state);


}

bool Widget::computePositionInLocalCoordinates(osgGA::EventVisitor* ev, osgGA::GUIEventAdapter* event, osg::Vec3d& localPosition) const
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

struct SortTraversalOrder
{
    bool operator() (const osgUtil::LineSegmentIntersector::Intersection* lhs, const osgUtil::LineSegmentIntersector::Intersection* rhs) const
    {
        double epsilon = 1e-6;
        if (lhs->ratio > (rhs->ratio+epsilon)) return true;
        if (lhs->ratio < (rhs->ratio-epsilon)) return false;

        const osg::NodePath& np_lhs = lhs->nodePath;
        const osg::NodePath& np_rhs = rhs->nodePath;

        osg::NodePath::const_iterator itr_lhs = np_lhs.begin();
        osg::NodePath::const_iterator end_lhs = np_lhs.end();
        osg::NodePath::const_iterator itr_rhs = np_rhs.begin();
        osg::NodePath::const_iterator end_rhs = np_rhs.end();
        const osg::Group* parent = 0;

        while(itr_lhs!=end_lhs && itr_rhs!=end_rhs)
        {
            if (*itr_lhs == *itr_rhs)
            {
                parent = (*itr_lhs)->asGroup();
                ++itr_lhs;
                ++itr_rhs;
            }
            else if (parent==0)
            {
                OSG_NOTICE<<"SortTraversalOrder::operator() NodePath has no parent, just have to use default less than operator for Intersection"<<std::endl;
                return (*lhs)<(*rhs);
            }
            else
            {
                const osgUI::Widget* widget = dynamic_cast<const osgUI::Widget*>(parent);

                unsigned int lhs_index = parent->getChildIndex(*itr_lhs);
                double lhs_sort_value = static_cast<double>(lhs_index)/static_cast<double>(parent->getNumChildren());

                unsigned int rhs_index = parent->getChildIndex(*itr_rhs);
                double rhs_sort_value = (static_cast<double>(rhs_index)+epsilon)/static_cast<double>(parent->getNumChildren());

                if (widget)
                {
                    const osgUI::Widget::GraphicsSubgraphMap& gsm = widget->getGraphicsSubgraphMap();
                    for(osgUI::Widget::GraphicsSubgraphMap::const_iterator itr=gsm.begin();
                        itr!=gsm.end();
                        ++itr)
                    {
                        if (itr->second==(*itr_lhs)) lhs_sort_value = itr->first;
                        if (itr->second==(*itr_rhs)) rhs_sort_value = itr->first;
                    }
                }

                if (lhs_sort_value>rhs_sort_value) return true;
                if (lhs_sort_value<rhs_sort_value) return false;

            }
        }

        return false;
    }
};

bool Widget::computeIntersections(osgGA::EventVisitor* ev, osgGA::GUIEventAdapter* event, Intersections& intersections, osg::Node::NodeMask traversalMask) const
{
    osgGA::GUIActionAdapter* aa = ev ? ev->getActionAdapter() : 0;
    osgUtil::LineSegmentIntersector::Intersections source_intersections;
    if (aa && aa->computeIntersections(*event, ev->getNodePath(), source_intersections, traversalMask))
    {
        typedef std::vector<const osgUtil::LineSegmentIntersector::Intersection*> IntersectionPointerList;
        IntersectionPointerList intersectionsToSort;

        // populate the temporay vector of poiners to the original intersection pointers.
        for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = source_intersections.begin();
            itr != source_intersections.end();
            ++itr)
        {
            if (itr->drawable->getName()!="DepthSetPanel")
            {
                intersectionsToSort.push_back(&(*itr));
            }
        }

        // sort the pointer list into order based on child traversal order, to be consistent with osgUI rendering order.
        std::sort(intersectionsToSort.begin(), intersectionsToSort.end(), SortTraversalOrder());

        // copy the pointers to final Intersection container
        for(IntersectionPointerList::iterator itr = intersectionsToSort.begin();
            itr != intersectionsToSort.end();
            ++itr)
        {
            intersections.push_back(*(*itr));
        }
        return true;
    }
    return false;
}


bool Widget::computeExtentsPositionInLocalCoordinates(osgGA::EventVisitor* ev, osgGA::GUIEventAdapter* event, osg::Vec3d& localPosition, bool withinExtents) const
{
    //OSG_NOTICE<<"Widget::computeExtentsPositionInLocalCoordinates(()"<<std::endl;
    const osg::Camera* camera = 0;
    double x=0.0, y=0.0;
    if (event->getNumPointerData()>=1)
    {
        const osgGA::PointerData* pd = event->getPointerData(event->getNumPointerData()-1);
        camera = pd->object->asCamera();
        if (camera)
        {
            x = pd->getXnormalized();
            y = pd->getYnormalized();
        }
    }
    //OSG_NOTICE<<"   camera = "<<camera<<", x = "<<x<<", y="<<y<<std::endl;
    if (!camera) return false;

    const osg::NodePath& nodePath = ev->getNodePath();

    osg::Matrixd matrix;
    if (nodePath.size()>1)
    {
        osg::NodePath prunedNodePath(nodePath.begin(),nodePath.end()-1);
        matrix = osg::computeLocalToWorld(prunedNodePath);
    }

    matrix.postMult(camera->getViewMatrix());
    matrix.postMult(camera->getProjectionMatrix());

    double zNear = -1.0;
    double zFar = 1.0;

    osg::Matrixd inverse;
    inverse.invert(matrix);

    osg::Vec3d startVertex = osg::Vec3d(x,y,zNear) * inverse;
    osg::Vec3d endVertex = osg::Vec3d(x,y,zFar) * inverse;

    //OSG_NOTICE<<"   startVertex("<<startVertex<<"(, endVertex("<<endVertex<<")"<<std::endl;


    osg::Plane plane(0.0, 0.0, 1.0, _extents.zMax());

    //OSG_NOTICE<<"   plane("<<plane<<")"<<std::endl;
    double ds = plane.distance(startVertex);
    double de = plane.distance(endVertex);
    if (ds*de>0.0) return false;

    double r = ds/(ds-de);
    //OSG_NOTICE<<"   r = "<<r<<std::endl;

    osg::Vec3d intersection = startVertex + (endVertex-startVertex)*r;
    //OSG_NOTICE<<"    intersection = "<<intersection<<std::endl;
    localPosition = intersection;

    return withinExtents ? _extents.contains(localPosition, 1e-6) : true;
}
