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
    _dataVariance(obj._dataVariance),
    _userDataContainer(0)
{
    if (obj._userDataContainer)
    {
        if (copyop.getCopyFlags()&osg::CopyOp::DEEP_COPY_USERDATA)
        {
            setUserDataContainer(osg::clone(obj._userDataContainer,copyop));
        }
        else
        {
            setUserDataContainer(obj._userDataContainer);
        }
    }
}

Object::~Object()
{
    if (_userDataContainer) _userDataContainer->unref();
}


void Object::setThreadSafeRefUnref(bool threadSafe)
{
    Referenced::setThreadSafeRefUnref(threadSafe);
    if (_userDataContainer) _userDataContainer->setThreadSafeRefUnref(threadSafe);
}

void Object::setUserDataContainer(osg::UserDataContainer* udc)
{
    if (_userDataContainer == udc) return;

    if (_userDataContainer) _userDataContainer->unref();

    _userDataContainer = udc;

    if (_userDataContainer) _userDataContainer->ref();
}

osg::UserDataContainer* Object::getOrCreateUserDataContainer()
{
    if (!_userDataContainer) setUserDataContainer(new DefaultUserDataContainer());
    return _userDataContainer;
}

void Object::setUserData(Referenced* obj)
{
    if (getUserData()==obj) return;

    getOrCreateUserDataContainer()->setUserData(obj);
}

Referenced* Object::getUserData()
{
    return _userDataContainer ? _userDataContainer->getUserData() : 0;
}

const Referenced* Object::getUserData() const
{
    return _userDataContainer ? _userDataContainer->getUserData() : 0;
}

} // end of namespace osg
