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

#include <osgUI/Callbacks>
#include <osgUI/Widget>
#include <osgUI/Dialog>

#include <osg/ValueObject>
#include <osg/MatrixTransform>
#include <osg/io_utils>

using namespace osgUI;

CloseCallback::CloseCallback(const std::string& callbackName, osgUI::Widget* closeWidget):
    _closeWidget(closeWidget)
{
    setName(callbackName);
}

CloseCallback::CloseCallback(const CloseCallback& hc, const osg::CopyOp& copyop)
{
}

bool CloseCallback::run(osg::Object* object, osg::Parameters&, osg::Parameters&) const
{
    if (_closeWidget.valid())
    {
        _closeWidget->setVisible(false);
    }

    osg::Node* node = dynamic_cast<osg::Node*>(object);
    if (node)
    {
        osg::NodePathList nodePathList = node->getParentalNodePaths();
        for(osg::NodePathList::iterator itr = nodePathList.begin();
            itr != nodePathList.end();
            ++itr)
        {
            osg::NodePath& nodePath = *itr;
            for(osg::NodePath::reverse_iterator ritr = nodePath.rbegin();
                ritr !=  nodePath.rend();
                ++ritr)
            {
                osgUI::Dialog* dialog = dynamic_cast<osgUI::Dialog*>(*ritr);
                if (dialog)
                {
                    dialog->setVisible(false);
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

HandleCallback::HandleCallback()
{
    setName("handle");
}

HandleCallback::HandleCallback(const HandleCallback& hc, const osg::CopyOp& copyop):
    osg::CallbackObject(hc, copyop)
{
}

bool HandleCallback::run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
{
    if (inputParameters.size()>=2)
    {
        osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(inputParameters[0].get());
        osgGA::Event* event = dynamic_cast<osgGA::Event*>(inputParameters[1].get());
        if (ev && event)
        {
            outputParameters.push_back(new osg::BoolValueObject("return",handle(ev,event)));
            return true;
        }
    }
    return false;
}

bool HandleCallback::handle(osgGA::EventVisitor* ev, osgGA::Event* event) const
{
    return false;
}


DragCallback::DragCallback():
    _dragging(false)
{
}

DragCallback::DragCallback(const DragCallback& hc, const osg::CopyOp& copyop):
    HandleCallback(hc, copyop)
{
}

osg::Transform* findNearestTransform(const osg::NodePath& nodePath)
{
    osg::Transform* transform = 0;
    for(osg::NodePath::const_reverse_iterator itr = nodePath.rbegin();
        itr != nodePath.rend();
        ++itr)
    {
        if ((*itr)->asTransform())
        {
            transform = (*itr)->asTransform();
            break;
        }
    }
    return transform;
}

bool DragCallback::handle(osgGA::EventVisitor* ev, osgGA::Event* event) const
{
    osgGA::GUIEventAdapter* ea = event ? event->asGUIEventAdapter() : 0;
    if (!ev || !ea) return false;

    osgUI::Widget* widget = dynamic_cast<osgUI::Widget*>(ev->getNodePath().empty() ? 0 : ev->getNodePath().back());
    if (widget && widget->getHasEventFocus())
    {
        DragCallback* dc = const_cast<DragCallback*>(this);
        switch(ea->getEventType())
        {
            case(osgGA::GUIEventAdapter::SCROLL):
            {
                osg::Vec3d localPosition;
                if (!widget->computeExtentsPositionInLocalCoordinates(ev, ea, localPosition)) break;

                OSG_NOTICE<<"Scroll motion: "<<ea->getScrollingMotion()<<", "<<localPosition<<std::endl;
                double scale = 1.0;

                switch(ea->getScrollingMotion())
                {
                    case(osgGA::GUIEventAdapter::SCROLL_UP): scale = 0.9; break;
                    case(osgGA::GUIEventAdapter::SCROLL_DOWN): scale = 1.0/0.9; break;
                    default: break;
                }

                if (scale!=1.0)
                {
                    osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(findNearestTransform(ev->getNodePath()));
                    if (transform)
                    {
                        transform->setMatrix(
                            osg::Matrixd::translate(-localPosition)*
                            osg::Matrixd::scale(osg::Vec3d(scale, scale, scale))*
                            osg::Matrixd::translate(localPosition)*
                            transform->getMatrix());
                    }
                }

                break;
            }
            case(osgGA::GUIEventAdapter::PUSH):
            {
                dc->_dragging = (ea->getButtonMask()==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
                if (dc->_dragging)
                {
                    osg::Vec3d localPosition;
                    if (widget->computeExtentsPositionInLocalCoordinates(ev, ea, localPosition))

                    {
                        dc->_previousPosition = localPosition;
                    }
                }
                break;
            }
            case(osgGA::GUIEventAdapter::DRAG):
            {
                if (dc->_dragging)
                {
                    osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(findNearestTransform(ev->getNodePath()));
                    if (transform)
                    {
                        osg::Vec3d position;
                        if (widget->computeExtentsPositionInLocalCoordinates(ev, ea, position, false))
                        {
                            osg::Vec3d delta = position-_previousPosition;
                            osg::MatrixTransform* mt = transform->asMatrixTransform();
                            mt->setMatrix(osg::Matrixd::translate(delta)*mt->getMatrix());
                            // OSG_NOTICE<<"Move to local "<<position<<", "<<position-_previousPosition<<std::endl;
                        }
                    }
                    else
                    {
                        OSG_NOTICE<<"Failed to drag, No Transform to move"<<std::endl;
                    }
                }
                break;
            }
            case(osgGA::GUIEventAdapter::RELEASE):
                dc->_dragging = false;
            default:
                break;
        }
    }
    return false;
}
