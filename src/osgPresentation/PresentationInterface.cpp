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

#include <osgPresentation/PresentationInterface>

using namespace osgPresentation;

osgViewer::ViewerBase* PresentationInterface::getViewer()
{
    SlideEventHandler* seh = SlideEventHandler::instance();
    return (seh!=0) ? seh->getViewer() : 0;
}

osg::Node* PresentationInterface::getPresentation()
{
    SlideEventHandler* seh = SlideEventHandler::instance();
    return (seh!=0) ? seh->getPresentationSwitch() : 0;
}

osgPresentation::SlideEventHandler* PresentationInterface::getSlideEventHandler()
{
    return SlideEventHandler::instance();
}

void PresentationInterface::jump(const osgPresentation::JumpData* jumpdata)
{
    SlideEventHandler* seh = SlideEventHandler::instance();
    if ((seh!=0) && (jumpdata!=0)) jumpdata->jump(seh);
}

void PresentationInterface::sendEventToViewer(osgGA::Event* event)
{
    SlideEventHandler* seh = SlideEventHandler::instance();
    if ((seh!=0) && (event!=0)) seh->dispatchEvent(event);
}

void PresentationInterface::sendEventToViewer(const osgPresentation::KeyPosition* kp)
{
    SlideEventHandler* seh = SlideEventHandler::instance();
    if ((seh!=0) && (kp!=0)) seh->dispatchEvent(*kp);
}

void PresentationInterface::sendEventToDevices(osgGA::Event* event)
{
    SlideEventHandler* seh = SlideEventHandler::instance();
    if ((seh!=0) && (event!=0)) seh->forwardEventToDevices(event);
}
