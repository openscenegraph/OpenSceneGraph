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

ObserverNodePath::ObserverNodePath()
{
}

ObserverNodePath::ObserverNodePath(const ObserverNodePath& rhs)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock_rhs(_mutex);
    _nodePath = rhs._nodePath;
}

ObserverNodePath::ObserverNodePath(const osg::NodePath& nodePath)
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

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock_rhs(rhs._mutex);
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock_lhs(_mutex);
    _nodePath = rhs._nodePath;
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
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    refNodePath.resize(_nodePath.size());
    for(unsigned int i=0; i<_nodePath.size(); ++i)
    {
        if (!_nodePath[i].lock(refNodePath[i]))
        {
            OSG_INFO<<"ObserverNodePath::getRefNodePath() node has been invalidated"<<std::endl;
            refNodePath.clear();
            return false;
        }
    }
    return true;
}

bool ObserverNodePath::getNodePath(NodePath& nodePath) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    nodePath.resize(_nodePath.size());
    for(unsigned int i=0; i<_nodePath.size(); ++i)
    {
        if (_nodePath[i].valid())
        {
            nodePath[i] = _nodePath[i].get();
        }
        else
        {
            OSG_NOTICE<<"ObserverNodePath::getNodePath() node has been invalidated"<<std::endl;
            nodePath.clear();
            return false;
        }
    }
    return true;
}

void ObserverNodePath::_setNodePath(const osg::NodePath& nodePath)
{
    _clearNodePath();

    // OSG_NOTICE<<"ObserverNodePath["<<this<<"]::_setNodePath() nodePath.size()="<<nodePath.size()<<std::endl;

    _nodePath.resize(nodePath.size());
    for(unsigned int i=0; i<nodePath.size(); ++i)
    {
        _nodePath[i] = nodePath[i];
    }
}

void ObserverNodePath::_clearNodePath()
{
    // OSG_NOTICE<<"ObserverNodePath["<<this<<"]::_clearNodePath() _nodePath.size()="<<_nodePath.size()<<std::endl;
    _nodePath.clear();
}
