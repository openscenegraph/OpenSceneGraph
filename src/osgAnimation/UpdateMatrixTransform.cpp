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

#include <osgAnimation/UpdateMatrixTransform>
#include <osg/NodeVisitor>
#include <osg/MatrixTransform>

using namespace osgAnimation;

UpdateMatrixTransform::UpdateMatrixTransform( const UpdateMatrixTransform& apc,const osg::CopyOp& copyop) : osg::Object(apc,copyop), AnimationUpdateCallback<osg::NodeCallback>(apc, copyop)
{
    _transforms = StackedTransform(apc.getStackedTransforms(), copyop);
}

UpdateMatrixTransform::UpdateMatrixTransform(const std::string& name) : AnimationUpdateCallback<osg::NodeCallback>(name)
{
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void UpdateMatrixTransform::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        osg::MatrixTransform* matrixTransform = dynamic_cast<osg::MatrixTransform*>(node);
        if (matrixTransform)
        {
            // here we would prefer to have a flag inside transform stack in order to avoid update and a dirty state in matrixTransform if it's not require.
            _transforms.update();
            const osg::Matrix& matrix = _transforms.getMatrix();
            matrixTransform->setMatrix(matrix);
        }
    }
    traverse(node,nv);
}


bool UpdateMatrixTransform::link(osgAnimation::Channel* channel)
{
    const std::string& channelName = channel->getName();

    // check if we can link a StackedTransformElement to the current Channel
    for (StackedTransform::iterator it = _transforms.begin(); it != _transforms.end(); ++it)
    {
        StackedTransformElement* element = it->get();
        if (element && !element->getName().empty() && channelName == element->getName())
        {
            Target* target = element->getOrCreateTarget();
            if (target && channel->setTarget(target))
                return true;
        }
    }

    OSG_INFO << "UpdateMatrixTransform::link Channel " << channel->getName() << " does not contain a symbolic name that can be linked to a StackedTransformElement." << std::endl;

    return false;
}
