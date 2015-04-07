/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/UpdateBone>

using namespace osgAnimation;

Bone::Bone(const Bone& b, const osg::CopyOp& copyop) : osg::MatrixTransform(b,copyop), _invBindInSkeletonSpace(b._invBindInSkeletonSpace), _boneInSkeletonSpace(b._boneInSkeletonSpace)
{
}

Bone::Bone(const std::string& name)
{
    if (!name.empty())
        setName(name);
}


void Bone::setDefaultUpdateCallback(const std::string& name)
{
    std::string cbName = name;
    if (cbName.empty())
        cbName = getName();
    setUpdateCallback(new UpdateBone(cbName));
}

Bone* Bone::getBoneParent()
{
    if (getParents().empty())
        return 0;
    osg::Node::ParentList parents = getParents();
    for (osg::Node::ParentList::iterator it = parents.begin(); it != parents.end(); ++it)
    {
        Bone* pb = dynamic_cast<Bone*>(*it);
        if (pb)
            return pb;
    }
    return 0;
}
const Bone* Bone::getBoneParent() const
{
    if (getParents().empty())
        return 0;
    const osg::Node::ParentList& parents = getParents();
    for (osg::Node::ParentList::const_iterator it = parents.begin(); it != parents.end(); ++it)
    {
        const Bone* pb = dynamic_cast<const Bone*>(*it);
        if (pb)
            return pb;
    }
    return 0;
}
