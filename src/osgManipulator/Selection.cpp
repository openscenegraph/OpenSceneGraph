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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Selection>
#include <osgManipulator/Command>

#include <algorithm>

using namespace osgManipulator;

class FindNodePathToRootVisitor : public osg::NodeVisitor
{
    public:
        
        osg::NodePath& _nodePathToRoot;

        FindNodePathToRootVisitor(osg::NodePath& np) 
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS), 
              _nodePathToRoot(np)
        {}

        virtual void apply(osg::Node& node)
        {
            _nodePathToRoot.push_back(&node);
            traverse(node);
        }
};

void osgManipulator::computeNodePathToRoot(osg::Node& node, osg::NodePath& np)
{
    np.clear();
    osg::ref_ptr<FindNodePathToRootVisitor> visitor = new FindNodePathToRootVisitor(np);
    node.accept(*visitor);
    np.pop_back();
    std::reverse(np.begin(), np.end());
}

Selection::Selection()
{
}

Selection::~Selection()
{
}

bool Selection::receive(const MotionCommand& command)
{
    switch (command.getStage())
    {
        case MotionCommand::START:
            {
                // Save the current matrix
                _startMotionMatrix = getMatrix();

                // Get the LocalToWorld and WorldToLocal matrix for this node.
                osg::NodePath nodePathToRoot;
                computeNodePathToRoot(*this,nodePathToRoot);
                _localToWorld = osg::computeLocalToWorld(nodePathToRoot);
                _worldToLocal = osg::Matrix::inverse(_localToWorld);

                return true;
            }
        case MotionCommand::MOVE:
            {
                // Transform the command's motion matrix into local motion matrix.
                osg::Matrix localMotionMatrix = _localToWorld * command.getWorldToLocal()
                                                * command.getMotionMatrix()
                                                * command.getLocalToWorld() * _worldToLocal;

                // Transform by the localMotionMatrix
                setMatrix(localMotionMatrix * _startMotionMatrix);

                return true;
            }
        case MotionCommand::FINISH:
            {
                return true; 
            }
        case MotionCommand::NONE:
        default:
            return false;
    }
}

