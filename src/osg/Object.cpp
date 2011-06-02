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
#include <osg/Object>

namespace osg
{

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Object
//
Object::Object(const Object& obj,const CopyOp& copyop): 
    Referenced(),
    _name(obj._name),
    _dataVariance(obj._dataVariance)
{
    if (obj._userDataContainer.valid())
    {
        if (copyop.getCopyFlags()&osg::CopyOp::DEEP_COPY_USERDATA)
        {
            _userDataContainer = new UserDataContainer(*obj._userDataContainer, copyop);
        }
        else
        {
            _userDataContainer = obj._userDataContainer;
        }
    }
}

void Object::setThreadSafeRefUnref(bool threadSafe)
{
    Referenced::setThreadSafeRefUnref(threadSafe);
    if (_userDataContainer.valid()) _userDataContainer->setThreadSafeRefUnref(threadSafe);
}

void Object::setUserData(Referenced* obj)
{
    getOrCreateUserDataContainer()->_userData = obj;
}

Referenced* Object::getUserData()
{
    return _userDataContainer.valid() ? _userDataContainer->_userData.get() : 0;
}

const Referenced* Object::getUserData() const
{
    return _userDataContainer.valid() ? _userDataContainer->_userData.get() : 0;
}

void Object::addUserObject(Object* obj)
{
    // make sure the UserDataContainer exists
    getOrCreateUserDataContainer();

    // make sure that the object isn't already in the container
    unsigned int i = getUserObjectIndex(obj);
    if (i<_userDataContainer->_objectList.size())
    {
        // object already in container so just return.
        return;
    }

    // object not already on user data container so add it in.
    _userDataContainer->_objectList.push_back(obj);
}

void Object::removeUserObject(unsigned int i)
{
     if (_userDataContainer.valid() && i<_userDataContainer->_objectList.size())
     {
         _userDataContainer->_objectList.erase(_userDataContainer->_objectList.begin()+i);
     }
}

void Object::setUserObject(unsigned int i, Object* obj)
{
     if (_userDataContainer.valid() && i<_userDataContainer->_objectList.size())
     {
         _userDataContainer->_objectList[i] = obj;
     }
}

Object* Object::getUserObject(unsigned int i)
{
     if (_userDataContainer.valid() && i<_userDataContainer->_objectList.size())
     {
         return _userDataContainer->_objectList[i].get();
     }
     return 0;
}

const Object* Object::getUserObject(unsigned int i) const
{
     if (_userDataContainer.valid() && i<_userDataContainer->_objectList.size())
     {
         return _userDataContainer->_objectList[i].get();
     }
     return 0;
}

unsigned int Object::getUserObjectIndex(const osg::Object* obj) const
{
     if (_userDataContainer.valid())
     {
        for(unsigned int i = 0; i < _userDataContainer->_objectList.size(); ++i)
        {
            if (_userDataContainer->_objectList[i]==obj) return i;
        }
        return _userDataContainer->_objectList.size();
     }
     return 0;
}

unsigned int Object::getUserObjectIndex(const std::string& name) const
{
     if (_userDataContainer.valid())
     {
        for(unsigned int i = 0; i < _userDataContainer->_objectList.size(); ++i)
        {
            Object* obj = _userDataContainer->_objectList[i].get();
            if (obj && obj->getName()==name) return i;
        }
        return _userDataContainer->_objectList.size();
     }
     return 0;
}

Object* Object::getUserObject(const std::string& name)
{
     if (_userDataContainer.valid())
     {
         unsigned int i = getUserObjectIndex(name);
         return (i<_userDataContainer->_objectList.size()) ? _userDataContainer->_objectList[i].get() : 0;
     }
     else
     {
         return 0;
     }
}

const Object* Object::getUserObject(const std::string& name) const
{
     if (_userDataContainer.valid())
     {
         unsigned int i = getUserObjectIndex(name);
         return (i<_userDataContainer->_objectList.size()) ? _userDataContainer->_objectList[i].get() : 0;
     }
     else
     {
         return 0;
     }
}

unsigned int Object::getNumUserObjects() const
{
    return _userDataContainer.valid() ? _userDataContainer->_objectList.size() : 0;
}


void Object::setDescriptions(const DescriptionList& descriptions)
{
    getOrCreateUserDataContainer()->_descriptionList = descriptions;
}

Object::DescriptionList& Object::getDescriptions()
{
    return getOrCreateUserDataContainer()->_descriptionList;
}

static OpenThreads::Mutex s_mutex_StaticDescriptionList;
static const Object::DescriptionList& getStaticDescriptionList()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_StaticDescriptionList);
    static Object::DescriptionList s_descriptionList;
    return s_descriptionList;
}

const Object::DescriptionList& Object::getDescriptions() const
{
    if (_userDataContainer.valid()) return _userDataContainer->_descriptionList;
    else return getStaticDescriptionList();
}

std::string& Object::getDescription(unsigned int i)
{
    return getOrCreateUserDataContainer()->_descriptionList[i];
}

const std::string& Object::getDescription(unsigned int i) const
{
    if (_userDataContainer.valid()) return _userDataContainer->_descriptionList[i];
    else return getStaticDescriptionList()[i];
}

unsigned int Object::getNumDescriptions() const
{
    return _userDataContainer.valid() ? _userDataContainer->_descriptionList.size() : 0;
}

void Object::addDescription(const std::string& desc)
{
    getOrCreateUserDataContainer()->_descriptionList.push_back(desc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// UserDataContainer
//
Object::UserDataContainer::UserDataContainer():
    Referenced(true)
{
}

Object::UserDataContainer::UserDataContainer(const UserDataContainer& udc, const osg::CopyOp& copyop):
    Referenced(true)
{
    _userData = udc._userData;
    _descriptionList = udc._descriptionList;
}

void Object::UserDataContainer::setThreadSafeRefUnref(bool threadSafe)
{
    Referenced::setThreadSafeRefUnref(threadSafe);

    if (_userData.valid()) _userData->setThreadSafeRefUnref(threadSafe);

    for(ObjectList::iterator itr = _objectList.begin();
        itr != _objectList.end();
        ++itr)
    {
        (*itr)->setThreadSafeRefUnref(threadSafe);
    }
}



} // end of namespace osg
