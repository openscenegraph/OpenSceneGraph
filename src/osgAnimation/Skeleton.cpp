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

#include <osgAnimation/Skeleton>
#include <osgAnimation/Bone>
#include <osg/Notify>

using namespace osgAnimation;

Skeleton::Skeleton() {}

Skeleton::Skeleton(const Skeleton& b, const osg::CopyOp& copyop) : osg::MatrixTransform(b,copyop) {}

Skeleton::UpdateSkeleton::UpdateSkeleton() : _needValidate(true) {}

Skeleton::UpdateSkeleton::UpdateSkeleton(const UpdateSkeleton& us, const osg::CopyOp& copyop) :
    osg::Object(us, copyop),
    osg::NodeCallback(us, copyop)
{
    _needValidate = true;
}

bool Skeleton::UpdateSkeleton::needToValidate() const
{
    return _needValidate;
}


class ValidateSkeletonVisitor : public osg::NodeVisitor
{
public:
    ValidateSkeletonVisitor(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Node& /*node*/) { return; }
    void apply(osg::Transform& node)
    {
        // the idea is to traverse the skeleton or bone but to stop if other node is found
        Bone* bone = dynamic_cast<Bone*>(&node);
        if (!bone)
            return;

        bool foundNonBone = false;

        for (unsigned int i = 0; i < bone->getNumChildren(); ++i)
        {
            if (dynamic_cast<Bone*>(bone->getChild(i)))
            {
                if (foundNonBone)
                {
                    OSG_WARN <<
                        "Warning: a Bone was found after a non-Bone child "
                        "within a Skeleton. Children of a Bone must be ordered "
                        "with all child Bones first for correct update order." << std::endl;
                    setTraversalMode(TRAVERSE_NONE);
                    return;
                }
            }
            else
            {
                foundNonBone = true;
            }
        }
        traverse(node);
    }
};

void Skeleton::UpdateSkeleton::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        Skeleton* skeleton = dynamic_cast<Skeleton*>(node);
        if (_needValidate && skeleton)
        {
            ValidateSkeletonVisitor visitor;
            for (unsigned int i = 0; i < skeleton->getNumChildren(); ++i)
            {
                osg::Node* child = skeleton->getChild(i);
                child->accept(visitor);
            }
            _needValidate = false;
        }
    }
    traverse(node,nv);
}

void Skeleton::setDefaultUpdateCallback()
{
    setUpdateCallback(new Skeleton::UpdateSkeleton );
}
