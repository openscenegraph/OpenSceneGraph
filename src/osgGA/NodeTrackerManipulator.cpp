/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
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

#include <osgGA/NodeTrackerManipulator>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/Transform>

using namespace osg;
using namespace osgGA;


NodeTrackerManipulator::NodeTrackerManipulator( int flags )
    : inherited( flags ),
      _trackerMode(NODE_CENTER_AND_ROTATION)
{
    setVerticalAxisFixed(false);
}


NodeTrackerManipulator::NodeTrackerManipulator( const NodeTrackerManipulator& m, const CopyOp& copyOp )
    : inherited( m, copyOp ),
      _trackNodePath( m._trackNodePath ),
      _trackerMode( m._trackerMode )
{
}


void NodeTrackerManipulator::setTrackNodePath(const osg::NodePath& nodePath)
{
    _trackNodePath.clear();
    _trackNodePath.reserve(nodePath.size());
    std::copy(nodePath.begin(), nodePath.end(), std::back_inserter(_trackNodePath));
}


osg::NodePath NodeTrackerManipulator::getNodePath() const
{
    osg::NodePath nodePath;
    for(ObserverNodePath::const_iterator itr = _trackNodePath.begin();
        itr != _trackNodePath.end();
        ++itr)
    {
        nodePath.push_back(const_cast<osg::Node*>(itr->get()));
    }
    return nodePath;
}

bool NodeTrackerManipulator::validateNodePath() const
{
    for(ObserverNodePath::const_iterator itr = _trackNodePath.begin();
        itr != _trackNodePath.begin();
        ++itr)
    {
        if (*itr==0)
        {
            OSG_NOTICE<<"Warning: tracked node path has been invalidated by changes in the scene graph."<<std::endl;
            const_cast<ObserverNodePath&>(_trackNodePath).clear();
            return false;
        }
    }
    return true;
}

void NodeTrackerManipulator::setTrackerMode(TrackerMode mode)
{
    _trackerMode = mode;
}

/// Sets rotation mode. \sa setVerticalAxisFixed
void NodeTrackerManipulator::setRotationMode(RotationMode mode)
{
    setVerticalAxisFixed(mode!=TRACKBALL);

    if (getAutoComputeHomePosition())
        computeHomePosition();
}

/// Gets rotation mode. \sa getVerticalAxisFixed
NodeTrackerManipulator::RotationMode NodeTrackerManipulator::getRotationMode() const
{
    return getVerticalAxisFixed() ? ELEVATION_AZIM : TRACKBALL;
}

void NodeTrackerManipulator::setNode(Node* node)
{
    inherited::setNode( node );

    // update model size
    if (_flags & UPDATE_MODEL_SIZE)
    {
        if (_node.valid())
        {
            setMinimumDistance(clampBetween(_modelSize*0.001, 0.00001, 1.0));
            OSG_INFO << "NodeTrackerManipulator: setting minimum distance to "
                         << _minimumDistance << std::endl;
        }
    }
}

void NodeTrackerManipulator::setTrackNode(osg::Node* node)
{
    if (!node)
    {
        OSG_NOTICE<<"NodeTrackerManipulator::setTrackNode(Node*):  Unable to set tracked node due to null Node*"<<std::endl;
        return;
    }

    osg::NodePathList nodePaths = node->getParentalNodePaths();
    if (!nodePaths.empty())
    {
        if (nodePaths.size()>1)
        {
            OSG_NOTICE<<"osgGA::NodeTrackerManipualtor::setTrackNode(..) taking first parent path, ignoring others."<<std::endl;
        }

        OSG_INFO<<"NodeTrackerManipulator::setTrackNode(Node*"<<node<<" "<<node->getName()<<"): Path set"<<std::endl;
        _trackNodePath.clear();
        setTrackNodePath( nodePaths.front() );
    }
    else
    {
        OSG_NOTICE<<"NodeTrackerManipulator::setTrackNode(Node*): Unable to set tracked node due to empty parental path."<<std::endl;
    }

    OSG_INFO<<"setTrackNode("<<node->getName()<<")"<<std::endl;
    for(unsigned int i=0; i<_trackNodePath.size(); ++i)
    {
        OSG_INFO<<"  "<<_trackNodePath[i]->className()<<" '"<<_trackNodePath[i]->getName()<<"'"<<std::endl;
    }

}


void NodeTrackerManipulator::computeHomePosition()
{
    osg::Node* node = _trackNodePath.empty() ? getNode() : _trackNodePath.back().get();

    if(node)
    {
        const osg::BoundingSphere& boundingSphere=node->getBound();

        setHomePosition(boundingSphere._center+osg::Vec3d( 0.0,-3.5f * boundingSphere._radius,0.0f),
                        boundingSphere._center,
                        osg::Vec3d(0.0f,0.0f,1.0f),
                        _autoComputeHomePosition);
    }
}


void NodeTrackerManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    osg::Vec3d eye,center,up;
    matrix.getLookAt(eye,center,up,_distance);
    computePosition(eye,center,up);
}

void NodeTrackerManipulator::computeNodeWorldToLocal(osg::Matrixd& worldToLocal) const
{
    if (validateNodePath())
    {
        worldToLocal = osg::computeWorldToLocal(getNodePath());
    }
}

void NodeTrackerManipulator::computeNodeLocalToWorld(osg::Matrixd& localToWorld) const
{
    if (validateNodePath())
    {
        localToWorld = osg::computeLocalToWorld(getNodePath());
    }

}

void NodeTrackerManipulator::computeNodeCenterAndRotation(osg::Vec3d& nodeCenter, osg::Quat& nodeRotation) const
{
    osg::Matrixd localToWorld, worldToLocal;
    computeNodeLocalToWorld(localToWorld);
    computeNodeWorldToLocal(worldToLocal);

    if (validateNodePath())
        nodeCenter = osg::Vec3d(_trackNodePath.back()->getBound().center())*localToWorld;
    else
        nodeCenter = osg::Vec3d(0.0f,0.0f,0.0f)*localToWorld;


    switch(_trackerMode)
    {
        case(NODE_CENTER_AND_AZIM):
        {
            CoordinateFrame coordinateFrame = getCoordinateFrame(nodeCenter);
            osg::Matrixd localToFrame(localToWorld*osg::Matrixd::inverse(coordinateFrame));

            double azim = atan2(-localToFrame(0,1),localToFrame(0,0));
            osg::Quat nodeRotationRelToFrame, rotationOfFrame;
            nodeRotationRelToFrame.makeRotate(-azim,0.0,0.0,1.0);
            rotationOfFrame = coordinateFrame.getRotate();
            nodeRotation = nodeRotationRelToFrame*rotationOfFrame;
            break;
        }
        case(NODE_CENTER_AND_ROTATION):
        {
            // scale the matrix to get rid of any scales before we extract the rotation.
            double sx = 1.0/sqrt(localToWorld(0,0)*localToWorld(0,0) + localToWorld(1,0)*localToWorld(1,0) + localToWorld(2,0)*localToWorld(2,0));
            double sy = 1.0/sqrt(localToWorld(0,1)*localToWorld(0,1) + localToWorld(1,1)*localToWorld(1,1) + localToWorld(2,1)*localToWorld(2,1));
            double sz = 1.0/sqrt(localToWorld(0,2)*localToWorld(0,2) + localToWorld(1,2)*localToWorld(1,2) + localToWorld(2,2)*localToWorld(2,2));
            localToWorld = localToWorld*osg::Matrixd::scale(sx,sy,sz);

            nodeRotation = localToWorld.getRotate();
            break;
        }
        case(NODE_CENTER):
        default:
        {
            CoordinateFrame coordinateFrame = getCoordinateFrame(nodeCenter);
            nodeRotation = coordinateFrame.getRotate();
            break;
        }
    }

}


osg::Matrixd NodeTrackerManipulator::getMatrix() const
{
    osg::Vec3d nodeCenter;
    osg::Quat nodeRotation;
    computeNodeCenterAndRotation(nodeCenter,nodeRotation);
    return osg::Matrixd::translate(0.0,0.0,_distance)*osg::Matrixd::rotate(_rotation)*osg::Matrixd::rotate(nodeRotation)*osg::Matrix::translate(nodeCenter);
}

osg::Matrixd NodeTrackerManipulator::getInverseMatrix() const
{
    osg::Vec3d nodeCenter;
    osg::Quat nodeRotation;
    computeNodeCenterAndRotation(nodeCenter,nodeRotation);
    return osg::Matrixd::translate(-nodeCenter)*osg::Matrixd::rotate(nodeRotation.inverse())*osg::Matrixd::rotate(_rotation.inverse())*osg::Matrixd::translate(0.0,0.0,-_distance);
}

void NodeTrackerManipulator::computePosition(const osg::Vec3d& eye,const osg::Vec3d& center,const osg::Vec3d& up)
{
    if (!_node) return;

    // compute rotation matrix
    osg::Vec3d lv(center-eye);
    _distance = lv.length();

    osg::Matrixd lookat;
    lookat.makeLookAt(eye,center,up);

    _rotation = lookat.getRotate().inverse();
}


// doc in parent
bool NodeTrackerManipulator::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
    osg::Vec3d nodeCenter;
    osg::Quat nodeRotation;
    computeNodeCenterAndRotation(nodeCenter, nodeRotation);

    // rotate camera
    if( getVerticalAxisFixed() ) {

         osg::Matrix rotation_matrix;
         rotation_matrix.makeRotate(_rotation);

         osg::Vec3d lookVector = -getUpVector(rotation_matrix);
         osg::Vec3d sideVector = getSideVector(rotation_matrix);
         osg::Vec3d upVector = getFrontVector(rotation_matrix);

         osg::Vec3d localUp(0.0f,0.0f,1.0f);

         osg::Vec3d forwardVector = localUp^sideVector;
         sideVector = forwardVector^localUp;

         forwardVector.normalize();
         sideVector.normalize();

         osg::Quat rotate_elevation;
         rotate_elevation.makeRotate(dy,sideVector);

         osg::Quat rotate_azim;
         rotate_azim.makeRotate(-dx,localUp);

         _rotation = _rotation * rotate_elevation * rotate_azim;

    } else
        rotateTrackball( _ga_t0->getXnormalized(), _ga_t0->getYnormalized(),
                         _ga_t1->getXnormalized(), _ga_t1->getYnormalized(),
                         _thrown ? float( _delta_frame_time / eventTimeDelta ) : 1.f );

    return true;
}


// doc in parent
bool NodeTrackerManipulator::performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
    osg::Vec3d nodeCenter;
    osg::Quat nodeRotation;
    computeNodeCenterAndRotation(nodeCenter, nodeRotation);

    return true;
}


// doc in parent
bool NodeTrackerManipulator::performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
    osg::Vec3d nodeCenter;
    osg::Quat nodeRotation;
    computeNodeCenterAndRotation(nodeCenter, nodeRotation);

    return inherited::performMovementRightMouseButton(eventTimeDelta, dx, dy);
}
