/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
 * applications, as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/StateAttribute>

#include <algorithm>

using namespace osg;

void StateAttribute::addParent(osg::StateSet* object)
{
    _parents.push_back(object);
}

void StateAttribute::removeParent(osg::StateSet* object)
{
    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),object);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}
