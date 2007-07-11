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

#include <osg/NodeTrackerCallback>
#include <osg/NodeVisitor>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/CameraView>
#include <osg/Camera>
#include <osg/Notify>

using namespace osg;

class ApplyMatrixVisitor : public NodeVisitor
{
    public:

        ApplyMatrixVisitor(const osg::Matrix& matrix):
            _matrix(matrix) {}

        virtual void apply(Camera& camera)
        {
            camera.setViewMatrix(_matrix);
        }

        virtual void apply(CameraView& cv)
        {
            cv.setPosition(_matrix.getTrans());
            cv.setAttitude(_matrix.getRotate());
        }

        virtual void apply(MatrixTransform& mt)
        {
            mt.setMatrix(_matrix);
        }
        
        virtual void apply(PositionAttitudeTransform& pat)
        {
            pat.setPosition(_matrix.getTrans());
            pat.setAttitude(_matrix.getRotate());
        }
        
        osg::Matrix _matrix;
};


void NodeTrackerCallback::setTrackNode(osg::Node* node)
{
    if (!node)
    {
        osg::notify(osg::NOTICE)<<"NodeTrackerCallback::setTrackNode(Node*):  Unable to set tracked node due to null Node*"<<std::endl;
        return;
    }

    NodePathList parentNodePaths = node->getParentalNodePaths();

    if (!parentNodePaths.empty())
    {
        osg::notify(osg::INFO)<<"NodeTrackerCallback::setTrackNode(Node*): Path set"<<std::endl;
        setTrackNodePath(parentNodePaths[0]);
    }
    else
    {
        osg::notify(osg::NOTICE)<<"NodeTrackerCallback::setTrackNode(Node*): Unable to set tracked node due to empty parental path."<<std::endl;
    }
}

void NodeTrackerCallback::operator()(Node* node, NodeVisitor* nv)
{
    if (nv->getVisitorType()==NodeVisitor::UPDATE_VISITOR)
    {
        update(*node);
    }

    traverse(node,nv);
}

bool NodeTrackerCallback::validateNodePath() const
{
    for(ObserverNodePath::const_iterator itr = _trackNodePath.begin();
        itr != _trackNodePath.begin();
        ++itr)
    {
        if (*itr==0) 
        {
            osg::notify(osg::NOTICE)<<"Warning: tracked node path has been invalidated by changes in the scene graph."<<std::endl;
            const_cast<ObserverNodePath&>(_trackNodePath).clear();
            return false;
        }
    }
    return true;
}


void NodeTrackerCallback::update(osg::Node& node)
{
    if (!validateNodePath()) return;
    
    osg::NodePath nodePath;
    for(ObserverNodePath::iterator itr = _trackNodePath.begin();
        itr != _trackNodePath.end();
        ++itr)
    {
        nodePath.push_back(itr->get());
    }

    ApplyMatrixVisitor applyMatrix(computeWorldToLocal(nodePath));
    node.accept(applyMatrix);
}
