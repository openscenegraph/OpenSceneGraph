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
#include <osg/UserDataContainer>

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
            _userDataContainer = obj.getUserDataContainer()->clone(copyop);
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

osg::Object* Object::getOrCreateUserDataContainer()
{
    if (!_userDataContainer) _userDataContainer = new UserDataContainer();
    return _userDataContainer.get();
}

void Object::setUserData(Referenced* obj)
{
    getOrCreateUserDataContainer()->setUserData(obj);
}

Referenced* Object::getUserData()
{
    return _userDataContainer.valid() ? _userDataContainer->getUserData() : 0;
}

const Referenced* Object::getUserData() const
{
    return _userDataContainer.valid() ? _userDataContainer->getUserData() : 0;
}

unsigned int Object::addUserObject(Object* obj)
{
    // make sure the UserDataContainer exists
    return getOrCreateUserDataContainer()->addUserObject(obj);
}

void Object::removeUserObject(unsigned int i)
{
    if (_userDataContainer.valid()) _userDataContainer->removeUserObject(i);
}

void Object::setUserObject(unsigned int i, Object* obj)
{
    // make sure the UserDataContainer exists
    getOrCreateUserDataContainer()->setUserObject(i,obj);
}

Object* Object::getUserObject(unsigned int i)
{
    return _userDataContainer.valid() ? _userDataContainer->getUserObject(i) : 0;
}

const Object* Object::getUserObject(unsigned int i) const
{
    return _userDataContainer.valid() ? _userDataContainer->getUserObject(i) : 0;
}

unsigned int Object::getNumUserObjects() const
{
    return _userDataContainer.valid() ? _userDataContainer->getNumUserObjects() : 0;
}

unsigned int Object::getUserObjectIndex(const osg::Object* obj, unsigned int startPos) const
{
    return _userDataContainer.valid() ? _userDataContainer->getUserObjectIndex(obj, startPos) : 0;
}

unsigned int Object::getUserObjectIndex(const std::string& name, unsigned int startPos) const
{
    return _userDataContainer.valid() ? _userDataContainer->getUserObjectIndex(name, startPos) : 0;
}

Object* Object::getUserObject(const std::string& name, unsigned int startPos)
{
     return getUserObject(getUserObjectIndex(name, startPos));
}

const Object* Object::getUserObject(const std::string& name, unsigned int startPos) const
{
     return getUserObject(getUserObjectIndex(name, startPos));
}

void Object::setDescriptions(const DescriptionList& descriptions)
{
    getOrCreateUserDataContainer()->setDescriptions(descriptions);
}

Object::DescriptionList& Object::getDescriptions()
{
    return getOrCreateUserDataContainer()->getDescriptions();
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
    if (_userDataContainer.valid()) return _userDataContainer->getDescriptions();
    else return getStaticDescriptionList();
}

std::string& Object::getDescription(unsigned int i)
{
    return getOrCreateUserDataContainer()->getDescriptions()[i];
}

const std::string& Object::getDescription(unsigned int i) const
{
    if (_userDataContainer.valid()) return _userDataContainer->getDescriptions()[i];
    else return getStaticDescriptionList()[i];
}

unsigned int Object::getNumDescriptions() const
{
    return _userDataContainer.valid() ? _userDataContainer->getDescriptions().size() : 0;
}

void Object::addDescription(const std::string& desc)
{
    getOrCreateUserDataContainer()->getDescriptions().push_back(desc);
}

} // end of namespace osg
