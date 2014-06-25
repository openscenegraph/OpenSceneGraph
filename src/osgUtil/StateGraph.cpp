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
#include <osgUtil/StateGraph>

#include <osg/Notify>

using namespace osg;
using namespace osgUtil;

void StateGraph::reset()
{
    _parent = NULL;
    _stateset = NULL;

    _depth = 0;

    _children.clear();
    _leaves.clear();
}

/** recursively clean the StateGraph of all its drawables, lights and depths.
  * Leaves children intact, and ready to be populated again.*/
void StateGraph::clean()
{

    // clean local drawables etc.
    _leaves.clear();

    // call clean on all children.
    for(ChildList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        itr->second->clean();
    }

}

/** recursively prune the StateGraph of empty children.*/
void StateGraph::prune()
{
    // call prune on all children.
    ChildList::iterator citr=_children.begin();
    while(citr!=_children.end())
    {
        citr->second->prune();

        if (citr->second->empty())
        {
            ChildList::iterator ditr= citr++;
            _children.erase(ditr);
        }
        else ++citr;
    }
}
