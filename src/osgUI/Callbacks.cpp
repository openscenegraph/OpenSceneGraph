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
#include <osg/ValueObject>
#include <osg/MatrixTransform>
#include <osg/io_utils>

using namespace osgUI;

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
            case(osgGA::GUIEventAdapter::PUSH):
            {
                dc->_dragging = (ea->getButtonMask()==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
                if (dc->_dragging)
                {
                    osg::Vec3d localPosition;
#if 0
                    if (widget->computePositionInLocalCoordinates(ev, ea, localPosition))
#else
                    if (widget->computeExtentsPositionInLocalCoordinates(ev, ea, localPosition))
#endif
                    {
                        dc->_previousPosition = localPosition;
                        OSG_NOTICE<<"* Move to local"<<_previousPosition<<std::endl;
                    }
                }
                break;
            }
            case(osgGA::GUIEventAdapter::DRAG):
            {
                if (dc->_dragging)
                {
                    osg::Transform* transform = 0;
                    for(osg::NodePath::reverse_iterator itr = ev->getNodePath().rbegin();
                        itr != ev->getNodePath().rend();
                        ++itr)
                    {
                        if ((*itr)->asTransform())
                        {
                            transform = (*itr)->asTransform();
                            break;
                        }
                    }
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
