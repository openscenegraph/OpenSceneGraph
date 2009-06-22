/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
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

#include <osgAnimation/Skinning>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>

osgAnimation::Bone::UpdateBone::UpdateBone(const osgAnimation::Bone::UpdateBone& apc,const osg::CopyOp& copyop) :
    osg::Object(apc, copyop),
    osgAnimation::AnimationUpdateCallback(apc, copyop)
{
    _quaternion = new osgAnimation::QuatTarget(apc._quaternion->getValue());
    _position = new osgAnimation::Vec3Target(apc._position->getValue());
    _scale = new osgAnimation::Vec3Target(apc._scale->getValue());
}


osgAnimation::Bone::Bone(const Bone& b, const osg::CopyOp& copyop) :
    osg::Transform(b,copyop),
    _position(b._position),
    _rotation(b._rotation),
    _scale(b._scale),
    _needToRecomputeBindMatrix(true),
    _bindInBoneSpace(b._bindInBoneSpace),
    _invBindInSkeletonSpace(b._invBindInSkeletonSpace),
    _boneInSkeletonSpace(b._boneInSkeletonSpace)
{
    osg::ref_ptr<osg::NodeCallback> updatecallback = getUpdateCallback();
    setUpdateCallback(0);
    while (updatecallback.valid()) {
        osg::NodeCallback* ucb = dynamic_cast<osg::NodeCallback*>(updatecallback->clone(copyop));
        ucb->setNestedCallback(0);
        addUpdateCallback(ucb);
        updatecallback = updatecallback->getNestedCallback();
    }
}

osgAnimation::Bone::Bone(const std::string& name)
{
    if (!name.empty())
        setName(name);
    _needToRecomputeBindMatrix = false;
}


void osgAnimation::Bone::setDefaultUpdateCallback(const std::string& name)
{
    std::string cbName = name;
    if (cbName.empty())
        cbName = getName();
    setUpdateCallback(new UpdateBone(cbName));
}

void osgAnimation::Bone::computeBindMatrix()
{
    _invBindInSkeletonSpace = osg::Matrix::inverse(_bindInBoneSpace);
    const Bone* parent = getBoneParent();
    _needToRecomputeBindMatrix = false;
    if (!parent)
    {
        osg::notify(osg::WARN) << "Warning " << className() <<"::computeBindMatrix you should not have this message, it means you miss to attach this bone(" << getName() <<") to a Skeleton node" << std::endl;
        return;
    }
    _invBindInSkeletonSpace = parent->getInvBindMatrixInSkeletonSpace() * _invBindInSkeletonSpace;
}

osgAnimation::Bone* osgAnimation::Bone::getBoneParent() 
{
    if (getParents().empty())
        return 0;
    osg::Node::ParentList parents = getParents();
    for (osg::Node::ParentList::iterator it = parents.begin(); it != parents.end(); it++) 
    {
        Bone* pb = dynamic_cast<Bone*>(*it);
        if (pb)
            return pb;
    }
    return 0;
}
const osgAnimation::Bone* osgAnimation::Bone::getBoneParent() const
{
    if (getParents().empty())
        return 0;
    const osg::Node::ParentList& parents = getParents();
    for (osg::Node::ParentList::const_iterator it = parents.begin(); it != parents.end(); it++) 
    {
        const Bone* pb = dynamic_cast<const Bone*>(*it);
        if (pb)
            return pb;
    }
    return 0;
}


/** Add Node to Group.
 * If node is not NULL and is not contained in Group then increment its
 * reference count, add it to the child list and dirty the bounding
 * sphere to force it to recompute on next getBound() and return true for success.
 * Otherwise return false. Scene nodes can't be added as child nodes.
 */
bool osgAnimation::Bone::addChild( Node *child ) 
{
    Bone* bone = dynamic_cast<Bone*>(child);
    if (bone)
        bone->setNeedToComputeBindMatrix(true);
    return osg::Group::addChild(child);
}

osgAnimation::Bone::BoneMap osgAnimation::Bone::getBoneMap()
{
    BoneMapVisitor mapVisitor;
    this->accept(mapVisitor);
    return mapVisitor._map;
}
