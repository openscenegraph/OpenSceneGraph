#include <osgGA/GUIEventHandler>

using osgGA::CompositeGUIEventHandler;

bool CompositeGUIEventHandler::handle(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    bool result=false;

    for (ChildList::iterator itr=_children.begin();
         itr!=_children.end();
         ++itr)
    {
        result |= (*itr)->handle(ea,aa);
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
