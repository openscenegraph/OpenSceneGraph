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
#include <osg/UserDataContainer>

namespace osg
{

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// UserDataContainer
//
UserDataContainer::UserDataContainer():
    Object(true)
{
}

UserDataContainer::UserDataContainer(const UserDataContainer& udc, const osg::CopyOp& copyop):
    Object(udc, copyop)
{
    _userData = udc._userData;
    _descriptionList = udc._descriptionList;
}

void UserDataContainer::setThreadSafeRefUnref(bool threadSafe)
{
    Object::setThreadSafeRefUnref(threadSafe);

    if (_userData.valid()) _userData->setThreadSafeRefUnref(threadSafe);

    for(ObjectList::iterator itr = _objectList.begin();
        itr != _objectList.end();
        ++itr)
    {
        (*itr)->setThreadSafeRefUnref(threadSafe);
    }
}

void UserDataContainer::setUserData(Referenced* obj)
{
    _userData = obj;
}

Referenced* UserDataContainer::getUserData()
{
    return _userData.get();
}

const Referenced* UserDataContainer::getUserData() const
{
    return _userData.get();
}

unsigned int UserDataContainer::addUserObject(Object* obj)
{
    // make sure that the object isn't already in the container
    unsigned int i = getUserObjectIndex(obj);
    if (i<_objectList.size())
    {
        // object already in container so just return.
        return i;
    }

    unsigned int pos = _objectList.size();

    // object not already on user data container so add it in.
    _objectList.push_back(obj);

    return pos;
}

void UserDataContainer::removeUserObject(unsigned int i)
{
    if (i<_objectList.size())
    {
        _objectList.erase(_objectList.begin()+i);
    }
}

void UserDataContainer::setUserObject(unsigned int i, Object* obj)
{
    if (i<_objectList.size())
    {
        _objectList[i] = obj;
    }
}

Object* UserDataContainer::getUserObject(unsigned int i)
{
    if (i<_objectList.size())
    {
        return _objectList[i].get();
    }
    return 0;
}

const Object* UserDataContainer::getUserObject(unsigned int i) const
{
    if (i<_objectList.size())
    {
        return _objectList[i].get();
    }
    return 0;
}

unsigned int UserDataContainer::getNumUserObjects() const
{
    return _objectList.size();
}

unsigned int UserDataContainer::getUserObjectIndex(const osg::Object* obj, unsigned int startPos) const
{
    for(unsigned int i = startPos; i < _objectList.size(); ++i)
    {
        if (_objectList[i]==obj) return i;
    }
    return _objectList.size();
}

unsigned int UserDataContainer::getUserObjectIndex(const std::string& name, unsigned int startPos) const
{
    for(unsigned int i = startPos; i < _objectList.size(); ++i)
    {
        Object* obj = _objectList[i].get();
        if (obj && obj->getName()==name) return i;
    }
    return _objectList.size();
}

void UserDataContainer::setDescriptions(const DescriptionList& descriptions)
{
    _descriptionList = descriptions;
}

Object::DescriptionList& UserDataContainer::getDescriptions()
{
    return _descriptionList;
}

const Object::DescriptionList& UserDataContainer::getDescriptions() const
{
    return _descriptionList;
}



} // end of namespace osg
