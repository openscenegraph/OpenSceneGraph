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
 * 
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */

#include <osgAnimation/BoneMapVisitor>

osgAnimation::BoneMapVisitor::BoneMapVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

void osgAnimation::BoneMapVisitor::apply(osg::Node&) { return; }
void osgAnimation::BoneMapVisitor::apply(osg::Transform& node)
{
    Bone* bone = dynamic_cast<Bone*>(&node);
    if (bone) 
    {
        _map[bone->getName()] = bone;
        traverse(node);
    }
}
const osgAnimation::Bone::BoneMap& osgAnimation::BoneMapVisitor::getBoneMap() const
{
    return _map;
}
