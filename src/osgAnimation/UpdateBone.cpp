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

#include <osg/NodeVisitor>
#include <osgAnimation/Bone>
#include <osgAnimation/UpdateBone>

using namespace osgAnimation;


UpdateBone::UpdateBone(const std::string& name) : UpdateMatrixTransform(name)
{
}

UpdateBone::UpdateBone(const UpdateBone& apc,const osg::CopyOp& copyop) : osg::Object(apc,copyop), UpdateMatrixTransform(apc, copyop)
{
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void UpdateBone::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        Bone* b = dynamic_cast<Bone*>(node);
        if (!b)
        {
            osg::notify(osg::WARN) << "Warning: UpdateBone set on non-Bone object." << std::endl;
            return;
        }

        // here we would prefer to have a flag inside transform stack in order to avoid update and a dirty state in matrixTransform if it's not require.
        _transforms.update();
        const osg::Matrix& matrix = _transforms.getMatrix();
        b->setMatrix(matrix);

        Bone* parent = b->getBoneParent();
        if (parent)
            b->setMatrixInSkeletonSpace(b->getMatrixInBoneSpace() * parent->getMatrixInSkeletonSpace());
        else
            b->setMatrixInSkeletonSpace(b->getMatrixInBoneSpace());
    }
    traverse(node,nv);
}
