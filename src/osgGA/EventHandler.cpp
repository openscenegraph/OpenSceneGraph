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

#include <osgGA/GUIEventHandler>
#include <osgGA/EventVisitor>

using namespace osgGA;

void EventHandler::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
    if (ev && ev->getActionAdapter() && !ev->getEvents().empty())
    {
        for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
            itr != ev->getEvents().end();
            ++itr)
        {
            handle(itr->get(), node, nv);
        }
    }
    if (node->getNumChildrenRequiringEventTraversal()>0 || _nestedCallback.valid()) traverse(node,nv);
}

void EventHandler::event(osg::NodeVisitor* nv, osg::Drawable* drawable)
{
    osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
    if (ev && ev->getActionAdapter() && !ev->getEvents().empty())
    {
        for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
            itr != ev->getEvents().end();
            ++itr)
        {
            handle(itr->get(), drawable, nv);
        }
    }
}

bool EventHandler::handle(osgGA::Event* event, osg::Object* object, osg::NodeVisitor* nv)
{
    OSG_NOTICE<<"Handle event "<<event<<std::endl;
    return false;
}
