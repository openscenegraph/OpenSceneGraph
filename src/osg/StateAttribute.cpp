/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/StateAttribute>
#include <osg/StateSet>
#include <osg/State>
#include <osg/NodeVisitor>
#include <osg/Notify>

#include <algorithm>

using namespace osg;

StateAttribute::StateAttribute()
    :Object(true)
{
}


void StateAttribute::addParent(osg::StateSet* object)
{
    OSG_DEBUG_FP<<"Adding parent"<<getRefMutex()<<std::endl;
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    _parents.push_back(object);
}

void StateAttribute::removeParent(osg::StateSet* object)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),object);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}


void StateAttribute::setUpdateCallback(StateAttributeCallback* uc)
{
    OSG_DEBUG<<"StateAttribute::Setting Update callbacks"<<std::endl;

    if (_updateCallback==uc) return;

    int delta = 0;
    if (_updateCallback.valid()) --delta;
    if (uc) ++delta;

    _updateCallback = uc;

    if (delta!=0)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringUpdateTraversal((*itr)->getNumChildrenRequiringUpdateTraversal()+delta);
        }
    }
}

void StateAttribute::setEventCallback(StateAttributeCallback* ec)
{
    OSG_DEBUG<<"StateAttribute::Setting Event callbacks"<<std::endl;

    if (_eventCallback==ec) return;

    int delta = 0;
    if (_eventCallback.valid()) --delta;
    if (ec) ++delta;

    _eventCallback = ec;

    if (delta!=0)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringEventTraversal((*itr)->getNumChildrenRequiringEventTraversal()+delta);
        }
    }
}
