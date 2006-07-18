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

#include <osgGA/GUIEventHandler>
#include <osgGA/EventVisitor>

using namespace osgGA;


void GUIEventHandler::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
    if (ev && ev->getActionAdapter() && !ev->getEvents().empty())
    {
        for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
            itr != ev->getEvents().end();
            ++itr)
        {
            if (handle(*(*itr), *(ev->getActionAdapter()), node, nv))
            {
                return;
            }
        }
    }
    traverse(node,nv);
}

void GUIEventHandler::event(osg::NodeVisitor* nv, osg::Drawable* drawable)
{
    osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
    if (ev && ev->getActionAdapter() && !ev->getEvents().empty())
    {
        for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
            itr != ev->getEvents().end();
            ++itr)
        {
            handle(*(*itr), *(ev->getActionAdapter()), drawable, nv);
        }
    }
}

void CompositeGUIEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    for (ChildList::const_iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->getUsage(usage);
    }
}

bool CompositeGUIEventHandler::handle(const GUIEventAdapter& ea,GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv)
{
    bool result=false;

    for (ChildList::iterator itr=_children.begin();
         itr!=_children.end();
         ++itr)
    {
        result |= (*itr)->handle(ea, aa, object, nv);
    }
    return result;
}


bool CompositeGUIEventHandler::addChild(GUIEventHandler *child)
{
    if (child && !containsNode(child))
    {
        // note ref_ptr<> automatically handles incrementing child's reference count.
        _children.push_back(child);
        return true;
    }
    else return false;

}

bool CompositeGUIEventHandler::removeChild(GUIEventHandler *child)
{
    ChildList::iterator itr = findChild(child);
    if (itr!=_children.end())
    {
        // note ref_ptr<> automatically handles decrementing child's reference count.
        _children.erase(itr);

        return true;
    }
    else return false;

}
