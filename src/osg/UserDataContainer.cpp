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
}

Object* UserDataContainer::getUserObject(const std::string& name, unsigned int startPos)
{
     return getUserObject(getUserObjectIndex(name, startPos));
}

const Object* UserDataContainer::getUserObject(const std::string& name, unsigned int startPos) const
{
     return getUserObject(getUserObjectIndex(name, startPos));
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// DefaultUserDataContainer
//
DefaultUserDataContainer::DefaultUserDataContainer()
{
}

DefaultUserDataContainer::DefaultUserDataContainer(const DefaultUserDataContainer& udc, const osg::CopyOp& copyop):
    UserDataContainer(udc, copyop)
{
    _userData = udc._userData;
    _descriptionList = udc._descriptionList;
    for(ObjectList::const_iterator itr = udc._objectList.begin();
        itr != udc._objectList.end();
        ++itr)
    {
        _objectList.push_back(copyop(itr->get()));
    }
}

void DefaultUserDataContainer::setThreadSafeRefUnref(bool threadSafe)
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

void DefaultUserDataContainer::setUserData(Referenced* obj)
{
    _userData = obj;
}

Referenced* DefaultUserDataContainer::getUserData()
{
    return _userData.get();
}

const Referenced* DefaultUserDataContainer::getUserData() const
{
    return _userData.get();
}

unsigned int DefaultUserDataContainer::addUserObject(Object* obj)
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

void DefaultUserDataContainer::removeUserObject(unsigned int i)
{
    if (i<_objectList.size())
    {
        _objectList.erase(_objectList.begin()+i);
    }
}

void DefaultUserDataContainer::setUserObject(unsigned int i, Object* obj)
{
    if (i<_objectList.size())
    {
        _objectList[i] = obj;
    }
}

Object* DefaultUserDataContainer::getUserObject(unsigned int i)
{
    if (i<_objectList.size())
    {
        return _objectList[i].get();
    }
    return 0;
}

const Object* DefaultUserDataContainer::getUserObject(unsigned int i) const
{
    if (i<_objectList.size())
    {
        return _objectList[i].get();
    }
    return 0;
}

unsigned int DefaultUserDataContainer::getNumUserObjects() const
{
    return _objectList.size();
}

unsigned int DefaultUserDataContainer::getUserObjectIndex(const osg::Object* obj, unsigned int startPos) const
{
    for(unsigned int i = startPos; i < _objectList.size(); ++i)
    {
        if (_objectList[i]==obj) return i;
    }
    return _objectList.size();
}

unsigned int DefaultUserDataContainer::getUserObjectIndex(const std::string& name, unsigned int startPos) const
{
    for(unsigned int i = startPos; i < _objectList.size(); ++i)
    {
        Object* obj = _objectList[i].get();
        if (obj && obj->getName()==name) return i;
    }
    return _objectList.size();
}

void DefaultUserDataContainer::setDescriptions(const DescriptionList& descriptions)
{
    _descriptionList = descriptions;
}

UserDataContainer::DescriptionList& DefaultUserDataContainer::getDescriptions()
{
    return _descriptionList;
}

const UserDataContainer::DescriptionList& DefaultUserDataContainer::getDescriptions() const
{
    return _descriptionList;
}

unsigned int DefaultUserDataContainer::getNumDescriptions() const
{
    return _descriptionList.size();
}

void DefaultUserDataContainer::addDescription(const std::string& desc)
{
    _descriptionList.push_back(desc);
}

} // end of namespace osg
