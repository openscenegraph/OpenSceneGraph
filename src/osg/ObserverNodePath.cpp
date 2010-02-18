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
#include <osg/Notify>

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

void ObserverNodePath::setNodePathTo(osg::Node* node)
{
    if (node)
    {
        NodePathList nodePathList = node->getParentalNodePaths();
        if (nodePathList.empty())
        {
            NodePath nodePath;
            nodePath.push_back(node);
            setNodePath(nodePath);
        }
        else
        {
            if (nodePathList[0].empty())
            {
                nodePathList[0].push_back(node);
            }
            setNodePath(nodePathList[0]);
        }
    }
    else
    {
        clearNodePath();
    }
}

void ObserverNodePath::setNodePath(const osg::NodePath& nodePath)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*getObserverMutex());
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
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*getObserverMutex());
    _clearNodePath();
}

bool ObserverNodePath::getRefNodePath(RefNodePath& refNodePath) const
{
    refNodePath.clear();

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*getObserverMutex());
    if (!_valid) return false;

    for(osg::NodePath::const_iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        refNodePath.push_back(*itr);
    }

    return true;
}

bool ObserverNodePath::getNodePath(NodePath& nodePath) const
{
    nodePath.clear();

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*getObserverMutex());
    if (!_valid) return false;
    nodePath = _nodePath;
    return true;
}

void ObserverNodePath::_setNodePath(const osg::NodePath& nodePath)
{
    if (nodePath==_nodePath) return;

    _clearNodePath();

    //OSG_NOTICE<<"ObserverNodePath["<<this<<"]::_setNodePath() nodePath.size()="<<nodePath.size()<<std::endl;

    _nodePath = nodePath;

    for(osg::NodePath::iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        //OSG_NOTICE<<"   addObserver("<<*itr<<")"<<std::endl;
        (*itr)->addObserver(this);
    }

    _valid = true;
}

void ObserverNodePath::_clearNodePath()
{
    //OSG_NOTICE<<"ObserverNodePath["<<this<<"]::_clearNodePath() _nodePath.size()="<<_nodePath.size()<<std::endl;
    for(osg::NodePath::iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        //OSG_NOTICE<<"   removeObserver("<<*itr<<")"<<std::endl;
        (*itr)->removeObserver(this);
    }
    _nodePath.clear();
    _valid = false;
}

bool ObserverNodePath::objectUnreferenced(void* ptr)
{
    osg::Node* node = reinterpret_cast<osg::Node*>(ptr);

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*getObserverMutex());

    _valid = false;

    for(osg::NodePath::iterator itr = _nodePath.begin();
        itr != _nodePath.end();
        ++itr)
    {
        if (*itr == node)
        {
            _nodePath.erase(itr);

            //OSG_NOTICE<<"ObserverNodePath["<<this<<"]::objectUnreferenced("<<ptr<<") found pointer in node path."<<std::endl;

            // return true as we wish calling method to remove self from observer set.
            return true;
        }
    }

    //OSG_NOTICE<<"Error: ObserverNodePath["<<this<<"]::::objectUnreferenced("<<ptr<<") could not find pointer in node path."<<std::endl;

    // return true as we wish calling method to remove self from observer set.
    return true;
}
