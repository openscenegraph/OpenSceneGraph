/*  -*-c++-*- 
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/LinkVisitor>
#include <osgAnimation/UpdateCallback>
#include <osg/Notify>

using namespace osgAnimation;

LinkVisitor::LinkVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) 
{
    _nbLinkedTarget = 0;
}

void LinkVisitor::reset()
{
    _nbLinkedTarget = 0;
}

AnimationList& LinkVisitor::getAnimationList()
{
    return _animations;
}

void LinkVisitor::apply(osg::Node& node)
{
    osgAnimation::AnimationUpdateCallback* cb = dynamic_cast<osgAnimation::AnimationUpdateCallback*>(node.getUpdateCallback());
    if (cb) 
    {
        int result = 0;
        for (int i = 0; i < (int)_animations.size(); i++)
        {
            result += cb->link(_animations[i].get());
            _nbLinkedTarget += result;
        }
        osg::notify(osg::NOTICE) << "LinkVisitor links " << result << " for \"" << cb->getName() << '"' << std::endl;
    }
    traverse(node);
}
