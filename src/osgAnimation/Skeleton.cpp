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

#include <osgAnimation/Skeleton>
#include <osgAnimation/Bone>

using namespace osgAnimation;

struct computeBindMatrixVisitor : public osg::NodeVisitor
{
    osg::Matrix _skeleton;
    computeBindMatrixVisitor(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Node& node) { return ;}
    void apply(osg::Transform& node) 
    {
        Bone* bone = dynamic_cast<Bone*>(&node);
        if (!bone)
            return;
        bone->computeBindMatrix();
        traverse(node);
    }
};

struct updateMatrixVisitor : public osg::NodeVisitor
{
    osg::Matrix _skeleton;
    updateMatrixVisitor(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Node& node) { return ;}
    void apply(osg::Transform& node) 
    {
        // the idea is to traverse the skeleton or bone but to stop if other node is found
        Bone* bone = dynamic_cast<Bone*>(&node);
        if (!bone)
            return;

        Bone* parent = bone->getBoneParent();
        if (bone->needToComputeBindMatrix()) 
        {
            computeBindMatrixVisitor visitor;
            bone->accept(visitor);
        }

        if (parent)
            bone->setMatrixInSkeletonSpace(bone->getMatrixInBoneSpace() * bone->getBoneParent()->getMatrixInSkeletonSpace());
        else
            bone->setMatrixInSkeletonSpace(bone->getMatrixInBoneSpace());

        traverse(node);
    }
};

void Skeleton::UpdateSkeleton::operator()(osg::Node* node, osg::NodeVisitor* nv)
{ 
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR) 
    {
        Skeleton* b = dynamic_cast<Skeleton*>(node);
        if (b) 
        {
            // apply the updater only on the root bone, The udpateMatrixVisitor will
            // traverse only bone and will update only bone. Then we continu on the classic
            // process. It's important to update Bone before other things because the update
            // of RigGeometry need it
            updateMatrixVisitor visitor;
            b->accept(visitor);
        }
    }
    traverse(node,nv);
}


Skeleton::Skeleton()
{
}

void Skeleton::setDefaultUpdateCallback()
{
    setUpdateCallback(new Skeleton::UpdateSkeleton );
}
