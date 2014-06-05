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

#include <osgGA/TerrainManipulator>
#include <osgUtil/LineSegmentIntersector>
#include <osg/io_utils>

using namespace osg;
using namespace osgGA;



/// Constructor.
TerrainManipulator::TerrainManipulator( int flags )
    : inherited( flags )
{
}


/// Constructor.
TerrainManipulator::TerrainManipulator( const TerrainManipulator& tm, const CopyOp& copyOp )
    : osg::Callback(tm, copyOp),
      inherited( tm, copyOp ),
      _previousUp( tm._previousUp )
{
}


/** Sets the manipulator rotation mode. RotationMode is now deprecated by
    osgGA::StandardManipulator::setVerticalAxisFixed() functionality,
    that is used across StandardManipulator derived classes.*/
void TerrainManipulator::setRotationMode( TerrainManipulator::RotationMode mode )
{
    setVerticalAxisFixed( mode == ELEVATION_AZIM );
}


/** Returns the manipulator rotation mode.*/
TerrainManipulator::RotationMode TerrainManipulator::getRotationMode() const
{
    return getVerticalAxisFixed() ? ELEVATION_AZIM : ELEVATION_AZIM_ROLL;
}


void TerrainManipulator::setNode( Node* node )
{
    inherited::setNode( node );

    // update model size
    if( _flags & UPDATE_MODEL_SIZE )
    {
        if( _node.valid() )
        {
            setMinimumDistance( clampBetween( _modelSize * 0.001, 0.00001, 1.0 ) );
            OSG_INFO << "TerrainManipulator: setting _minimumDistance to "
                     << _minimumDistance << std::endl;
        }
    }
}


void TerrainManipulator::setByMatrix(const Matrixd& matrix)
{

    Vec3d lookVector(- matrix(2,0),-matrix(2,1),-matrix(2,2));
    Vec3d eye(matrix(3,0),matrix(3,1),matrix(3,2));

    OSG_INFO<<"eye point "<<eye<<std::endl;
    OSG_INFO<<"lookVector "<<lookVector<<std::endl;

    if (!_node)
    {
        _center = eye+ lookVector;
        _distance = lookVector.length();
        _rotation = matrix.getRotate();
        return;
    }


    // need to reintersect with the terrain
    const BoundingSphere& bs = _node->getBound();
    float distance = (eye-bs.center()).length() + _node->getBound().radius();
    Vec3d start_segment = eye;
    Vec3d end_segment = eye + lookVector*distance;

    Vec3d ip;
    bool hitFound = false;
    if (intersect(start_segment, end_segment, ip))
    {
        OSG_INFO << "Hit terrain ok A"<< std::endl;
        _center = ip;

        _distance = (eye-ip).length();

        Matrixd rotation_matrix = Matrixd::translate(0.0,0.0,-_distance)*
                                  matrix*
                                  Matrixd::translate(-_center);

        _rotation = rotation_matrix.getRotate();

        hitFound = true;
    }

    if (!hitFound)
    {
        CoordinateFrame eyePointCoordFrame = getCoordinateFrame( eye );

        if (intersect(eye+getUpVector(eyePointCoordFrame)*distance,
                      eye-getUpVector(eyePointCoordFrame)*distance,
                      ip))
        {
            _center = ip;

            _distance = (eye-ip).length();

            _rotation.set(0,0,0,1);

            hitFound = true;
        }
    }


    CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
    _previousUp = getUpVector(coordinateFrame);

    clampOrientation();
}


void TerrainManipulator::setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up )
{
    if (!_node) return;

    // compute rotation matrix
    Vec3d lv(center-eye);
    _distance = lv.length();
    _center = center;

    OSG_INFO << "In compute"<< std::endl;

    if (_node.valid())
    {
        bool hitFound = false;

        double distance = lv.length();
        double maxDistance = distance+2*(eye-_node->getBound().center()).length();
        Vec3d farPosition = eye+lv*(maxDistance/distance);
        Vec3d endPoint = center;
        for(int i=0;
            !hitFound && i<2;
            ++i, endPoint = farPosition)
        {
            // compute the intersection with the scene.

            Vec3d ip;
            if (intersect(eye, endPoint, ip))
            {
                _center = ip;
                _distance = (ip-eye).length();

                hitFound = true;
            }
        }
    }

    // note LookAt = inv(CF)*inv(RM)*inv(T) which is equivalent to:
    // inv(R) = CF*LookAt.

    Matrixd rotation_matrix = Matrixd::lookAt(eye,center,up);

    _rotation = rotation_matrix.getRotate().inverse();

    CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
    _previousUp = getUpVector(coordinateFrame);

    clampOrientation();
}


bool TerrainManipulator::intersect( const Vec3d& start, const Vec3d& end, Vec3d& intersection ) const
{
    ref_ptr<osgUtil::LineSegmentIntersector> lsi = new osgUtil::LineSegmentIntersector(start,end);

    osgUtil::IntersectionVisitor iv(lsi.get());
    iv.setTraversalMask(_intersectTraversalMask);

    _node->accept(iv);

    if (lsi->containsIntersections())
    {
        intersection = lsi->getIntersections().begin()->getWorldIntersectPoint();
        return true;
    }
    return false;
}


bool TerrainManipulator::performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
    // pan model.
    double scale = -0.3f * _distance * getThrowScale( eventTimeDelta );

    Matrixd rotation_matrix;
    rotation_matrix.makeRotate(_rotation);


    // compute look vector.
    Vec3d sideVector = getSideVector(rotation_matrix);

    // CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
    // Vec3d localUp = getUpVector(coordinateFrame);
    Vec3d localUp = _previousUp;

    Vec3d forwardVector =localUp^sideVector;
    sideVector = forwardVector^localUp;

    forwardVector.normalize();
    sideVector.normalize();

    Vec3d dv = forwardVector * (dy*scale) + sideVector * (dx*scale);

    _center += dv;

    // need to recompute the intersection point along the look vector.

    bool hitFound = false;

    if (_node.valid())
    {
        // now reorientate the coordinate frame to the frame coords.
        CoordinateFrame coordinateFrame =  getCoordinateFrame(_center);

        // need to reintersect with the terrain
        double distance = _node->getBound().radius()*0.25f;

        Vec3d ip1;
        Vec3d ip2;
        bool hit_ip1 = intersect(_center, _center + getUpVector(coordinateFrame) * distance, ip1);
        bool hit_ip2 = intersect(_center, _center - getUpVector(coordinateFrame) * distance, ip2);
        if (hit_ip1)
        {
            if (hit_ip2)
            {
                _center = (_center-ip1).length2() < (_center-ip2).length2() ?
                            ip1 :
                            ip2;

                hitFound = true;
            }
            else
            {
                _center = ip1;
                hitFound = true;
            }
        }
        else if (hit_ip2)
        {
            _center = ip2;
            hitFound = true;
        }

        if (!hitFound)
        {
            // ??
            OSG_INFO<<"TerrainManipulator unable to intersect with terrain."<<std::endl;
        }

        coordinateFrame = getCoordinateFrame(_center);
        Vec3d new_localUp = getUpVector(coordinateFrame);


        Quat pan_rotation;
        pan_rotation.makeRotate(localUp,new_localUp);

        if (!pan_rotation.zeroRotation())
        {
            _rotation = _rotation * pan_rotation;
            _previousUp = new_localUp;
            //OSG_NOTICE<<"Rotating from "<<localUp<<" to "<<new_localUp<<"  angle = "<<acos(localUp*new_localUp/(localUp.length()*new_localUp.length()))<<std::endl;

            //clampOrientation();
        }
        else
        {
            OSG_INFO<<"New up orientation nearly inline - no need to rotate"<<std::endl;
        }
    }

    return true;
}


bool TerrainManipulator::performMovementRightMouseButton( const double eventTimeDelta, const double /*dx*/, const double dy )
{
    // zoom model
    zoomModel( dy * getThrowScale( eventTimeDelta ), false );
    return true;
}


void TerrainManipulator::clampOrientation()
{
    if (!getVerticalAxisFixed())
    {
        Matrixd rotation_matrix;
        rotation_matrix.makeRotate(_rotation);

        Vec3d lookVector = -getUpVector(rotation_matrix);
        Vec3d upVector = getFrontVector(rotation_matrix);

        CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
        Vec3d localUp = getUpVector(coordinateFrame);
        //Vec3d localUp = _previousUp;

        Vec3d sideVector = lookVector ^ localUp;

        if (sideVector.length()<0.1)
        {
            OSG_INFO<<"Side vector short "<<sideVector.length()<<std::endl;

            sideVector = upVector^localUp;
            sideVector.normalize();
        }

        Vec3d newUpVector = sideVector^lookVector;
        newUpVector.normalize();

        Quat rotate_roll;
        rotate_roll.makeRotate(upVector,newUpVector);

        if (!rotate_roll.zeroRotation())
        {
            _rotation = _rotation * rotate_roll;
        }
    }
}
