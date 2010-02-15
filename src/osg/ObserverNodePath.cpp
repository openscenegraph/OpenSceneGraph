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

#include <osg/ObserverNodePath>

using namespace osg;

ObserverNodePath::ObserverNodePath():
    _valid(false)
{
}

ObserverNodePath::ObserverNodePath(const ObserverNodePath& rhs):
    _valid(false)
{
    RefNodePath refNodePath;
    if (rhs.getRefNodePath(refNodePath))
    {
        setNodePath(refNodePath);
    }
}

ObserverNodePath::ObserverNodePath(const osg::NodePath& nodePath):
    _valid(false)
{
    setNodePath(nodePath);
}

ObserverNodePath::~ObserverNodePath()
{
    clearNodePath();
}

ObserverNodePath& ObserverNodePath::operator = (const ObserverNodePath& rhs)
{
    if (&rhs==this) return *this;

    RefNodePath refNodePath;
    if (rhs.getRefNodePath(refNodePath))
    {
        setNodePath(refNodePath);
    }
    return *this;
}

void ObserverNodePath::setNodePath(const osg::NodePath& nodePath)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _setNodePath(nodePath);
}

void ObserverNodePath::setNodePath(const osg::RefNodePath& refNodePath)
{
    osg::NodePath nodePath;
    for(RefNodePath::const_iterator itr = refNodePath.begin(); itr != refNodePath.end(); ++itr)
    {
        nodePath.push_back(itr->get());
    }
    setNodePath(nodePath);
}

void ObserverNodePath::clearNodePath()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _clearNodePath();
}

bool ObserverNodePath::getRefNodePath(RefNodePath& refNodePath) const
{
    refNodePath.clear();

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (!_valid) return false;

    for(osg::NodePath::const_iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        refNodePath.push_back(*itr);
    }

    return !refNodePath.empty();
}

bool ObserverNodePath::getNodePath(NodePath& nodePath) const
{
    nodePath.clear();

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (!_valid) return false;
    nodePath = _nodePath;
    return !nodePath.empty();
}

void ObserverNodePath::_setNodePath(const osg::NodePath& nodePath)
{
    if (nodePath==_nodePath) return;

    _clearNodePath();

    _nodePath = nodePath;

    for(osg::NodePath::iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        (*itr)->addObserver(this);
    }

    _valid = true;
}

void ObserverNodePath::_clearNodePath()
{
    for(osg::NodePath::iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        (*itr)->removeObserver(this);
    }
    _nodePath.clear();
    _valid = false;
}

bool ObserverNodePath::objectUnreferenced(void* ptr)
{
    osg::Node* node = reinterpret_cast<osg::Node*>(ptr);

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    _valid = false;

    for(osg::NodePath::iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        if (*itr == node)
        {
            _nodePath.erase(itr);

            // return true as we wish calling method to remove self from observert set.
            return true;
        }
    }
    // return true as we wish calling method to remove self from observert set.
    return true;
}
