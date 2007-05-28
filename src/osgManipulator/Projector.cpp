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

#include <osgManipulator/Projector>

using namespace osgManipulator;

namespace
{

bool computeClosestPoints(const osg::LineSegment& l1, const osg::LineSegment& l2,
                          osg::Vec3& p1, osg::Vec3& p2)
{
    // Computes the closest points (p1 and p2 on line l1 and l2 respectively) between the two lines
    // An explanation of the algorithm can be found at
    // http://www.geometryalgorithms.com/Archive/algorithm_0106/algorithm_0106.htm
    
    osg::Vec3 u = l1.end() - l1.start(); u.normalize();
    osg::Vec3 v = l2.end() - l2.start(); v.normalize();

    osg::Vec3 w0 = l1.start() - l2.start();

    float a = u * u;
    float b = u * v;
    float c = v * v;
    float d = u * w0;
    float e = v * w0;

    float denominator = a*c - b*b;

    // Test if lines are parallel
    if (denominator == 0.0) return false;

    float sc = (b*e - c*d)/denominator;
    float tc = (a*e - b*d)/denominator;

    p1 = l1.start() + u * sc;
    p2 = l2.start() + v * tc;

    return true;
}

bool computeClosestPointOnLine(const osg::Vec3& lineStart, const osg::Vec3& lineEnd,
                               const osg::Vec3& fromPoint, osg::Vec3& closestPoint)
{
    osg::Vec3 v = lineEnd - lineStart;
    osg::Vec3 w = fromPoint - lineStart;

    float c1 = w * v;
    float c2 = v * v;

    float almostZero = 0.000001;
    if (c2 < almostZero) return false;

    float b = c1 / c2;
    closestPoint = lineStart + v * b;

    return true;
}

bool getPlaneLineIntersection(const osg::Vec4& plane, 
                              const osg::Vec3& lineStart, const osg::Vec3& lineEnd, 
                              osg::Vec3& isect)
{
    const double deltaX = lineEnd.x() - lineStart.x();
    const double deltaY = lineEnd.y() - lineStart.y();
    const double deltaZ = lineEnd.z() - lineStart.z();

    const double denominator = (plane[0]*deltaX + plane[1]*deltaY + plane[2]*deltaZ);
    if (! denominator) return false;

    const double C = (plane[0]*lineStart.x() + plane[1]*lineStart.y() + plane[2]*lineStart.z() + plane[3]) / denominator;

    isect.x() = lineStart.x() - deltaX * C;
    isect.y() = lineStart.y() - deltaY * C;
    isect.z() = lineStart.z() - deltaZ * C;

    return true;
}

bool getSphereLineIntersection(const osg::Sphere& sphere, 
                               const osg::Vec3& lineStart, const osg::Vec3& lineEnd, 
                               osg::Vec3& frontISect, osg::Vec3& backISect)
{
    osg::Vec3 lineDirection = lineEnd - lineStart;
    lineDirection.normalize();

    osg::Vec3 v = lineStart - sphere.getCenter();
    float B = 2.0f * (lineDirection * v);
    float C = v * v - sphere.getRadius() * sphere.getRadius();
    
    float discriminant = B * B - 4.0f * C;

    if (discriminant < 0.0f) // Line and sphere don't intersect.
        return false;

    float discriminantSqroot = sqrtf(discriminant);
    float t0 = (-B - discriminantSqroot) * 0.5f;
    frontISect = lineStart + lineDirection * t0;

    float t1 = (-B + discriminantSqroot) * 0.5f;
    backISect = lineStart + lineDirection * t1;

    return true;
}

bool getUnitCylinderLineIntersection(const osg::Vec3& lineStart, const osg::Vec3& lineEnd, 
                                     osg::Vec3& isectFront, osg::Vec3& isectBack)
{
    osg::Vec3 dir = lineEnd - lineStart;
    dir.normalize();

    float a = dir[0] * dir[0] + dir[1] * dir[1];
    float b = 2.0f * (lineStart[0] * dir[0] + lineStart[1] * dir[1]);
    float c = lineStart[0] * lineStart[0] + lineStart[1] * lineStart[1] - 1;

    float d = b*b - 4*a*c;
    if (d < 0.0f) return false;

    float dSqroot = sqrtf(d);
    float t0, t1;
    if (b > 0.0f)
    {
        t0 = -(2.0f * c) / (dSqroot + b);
        t1 = -(dSqroot + b) / (2.0 * a);
    }
    else
    {
        t0 = (2.0f * c) / (dSqroot - b);
        t1 = (dSqroot - b) / (2.0 * a);
    }

    isectFront = lineStart + dir * t0;
    isectBack = lineStart + dir * t1;
    return true;
}

bool getCylinderLineIntersection(const osg::Cylinder& cylinder,
                                 const osg::Vec3& lineStart, const osg::Vec3& lineEnd, 
                                 osg::Vec3& isectFront, osg::Vec3& isectBack)
{
    // Compute matrix transformation that takes the cylinder to a unit cylinder with Z-axis as it's axis and
    // (0,0,0) as it's center.
    float oneOverRadius = 1.0f / cylinder.getRadius();
    osg::Matrix toUnitCylInZ = osg::Matrix::translate(-cylinder.getCenter())
                               * osg::Matrix::scale(oneOverRadius, oneOverRadius, oneOverRadius)
                               * osg::Matrix(cylinder.getRotation().inverse());
                               
    // Transform the lineStart and lineEnd into the unit cylinder space.
    osg::Vec3 unitCylLineStart = lineStart * toUnitCylInZ;
    osg::Vec3 unitCylLineEnd   = lineEnd * toUnitCylInZ;

    // Intersect line with unit cylinder.
    osg::Vec3 unitCylIsectFront, unitCylIsectBack;
    if (! getUnitCylinderLineIntersection(unitCylLineStart, unitCylLineEnd, unitCylIsectFront, unitCylIsectBack))
        return false;    

    // Transform back from unit cylinder space.
    osg::Matrix invToUnitCylInZ(osg::Matrix::inverse(toUnitCylInZ));
    isectFront = unitCylIsectFront * invToUnitCylInZ;
    isectBack = unitCylIsectBack * invToUnitCylInZ;    

    return true;
}

osg::Vec3 getLocalEyeDirection(const osg::Vec3& eyeDir, const osg::Matrix& localToWorld)
{
    // To take a normal from world to local you need to transform it by the transpose of the inverse of the 
    // world to local matrix. Pre-multipling is equivalent to doing a post-multiplication of the transpose.
    osg::Vec3 localEyeDir = localToWorld * eyeDir;
    localEyeDir.normalize();
    return localEyeDir;
}

osg::Plane computePlaneThruPointAndOrientedToEye(const osg::Vec3& eyeDir, const osg::Matrix& localToWorld,
                                                 const osg::Vec3& point, bool front)
{
    osg::Vec3 planeNormal = getLocalEyeDirection(eyeDir, localToWorld);
    if (! front) planeNormal = -planeNormal;

    osg::Plane plane;
    plane.set(planeNormal, point);
    return plane;
}

osg::Plane computePlaneParallelToAxisAndOrientedToEye(const osg::Vec3& eyeDir, const osg::Matrix& localToWorld,
                                                      const osg::Vec3& axisDir, float radius,
                                                      osg::Vec3& planeLineStart, osg::Vec3& planeLineEnd,
                                                      bool front)
{
    osg::Vec3 perpDir = axisDir ^ getLocalEyeDirection(eyeDir, localToWorld);
    osg::Vec3 planeDir = perpDir ^ axisDir;
    planeDir.normalize();
    if (! front)
        planeDir = -planeDir;

    osg::Vec3 planePoint = planeDir * radius + axisDir;
    osg::Plane plane;
    plane.set(planeDir, planePoint);

    planeLineStart = planePoint;
    planeLineEnd = planePoint + axisDir;
    return plane;
}

}


Projector::Projector() : _worldToLocalDirty(false)
{
}

Projector::~Projector()
{
}

LineProjector::LineProjector()
{
    _line = new osg::LineSegment(osg::Vec3(0.0,0.0,0.0), osg::Vec3(1.0,0.0,0.0));
}

LineProjector::LineProjector(const osg::Vec3& s, const osg::Vec3& e)
{
    _line = new osg::LineSegment(s,e);
}

LineProjector::~LineProjector()
{
}

bool LineProjector::project(const PointerInfo& pi, osg::Vec3& projectedPoint) const
{
    if (!_line->valid())
    {
        osg::notify(osg::WARN) << "Warning: Invalid line set. LineProjector::project() failed."<<std::endl;
        return false;
    }

    // Transform the line to world/object coordinate space.
    osg::ref_ptr<osg::LineSegment> objectLine = new osg::LineSegment;
    objectLine->mult(*_line, getLocalToWorld());

    // Get the near and far points for the mouse point.
    osg::Vec3 nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);
    osg::ref_ptr<osg::LineSegment> pointerLine = new osg::LineSegment(nearPoint,farPoint);

    osg::Vec3 closestPtLine, closestPtProjWorkingLine;
    if (! computeClosestPoints(*objectLine, *pointerLine, closestPtLine, closestPtProjWorkingLine))
        return false;

    osg::Vec3 localClosestPtLine = closestPtLine * getWorldToLocal();

    projectedPoint = localClosestPtLine;

    return true;
}

PlaneProjector::PlaneProjector()
{
}

PlaneProjector::PlaneProjector(const osg::Plane& plane)
{
    _plane = plane;
}


PlaneProjector::~PlaneProjector()
{
}

bool PlaneProjector::project(const PointerInfo& pi, osg::Vec3& projectedPoint) const
{
    if (!_plane.valid())
    {
        osg::notify(osg::WARN) << "Warning: Invalid plane set. PlaneProjector::project() failed."<< std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3 nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the plane with the line (formed by the near and far points in local coordinates).
    return getPlaneLineIntersection(_plane.asVec4(), objectNearPoint, objectFarPoint, projectedPoint);
}


SphereProjector::SphereProjector() : _sphere(new osg::Sphere), _front(true)
{
}

SphereProjector::SphereProjector(osg::Sphere* sphere) : _sphere(sphere), _front(true)
{
}


SphereProjector::~SphereProjector()
{
}

bool SphereProjector::project(const PointerInfo& pi, osg::Vec3& projectedPoint) const
{
    if (!_sphere->valid())
    {
        osg::notify(osg::WARN) << "Warning: Invalid sphere. SphereProjector::project() failed." << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3 nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::Vec3 dontCare;
    if (_front)
        return getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, projectedPoint, dontCare);
    return getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, dontCare, projectedPoint);
}

bool SphereProjector::isPointInFront(const PointerInfo& pi, const osg::Matrix& localToWorld) const
{
    osg::Vec3 centerToPoint = getSphere()->getCenter() - pi.getLocalIntersectPoint();
    if (centerToPoint * getLocalEyeDirection(pi.getEyeDir(), localToWorld) < 0.0)
        return false;
    return true;
}


SpherePlaneProjector::SpherePlaneProjector()
{
}

SpherePlaneProjector::SpherePlaneProjector(osg::Sphere* sphere) : SphereProjector(sphere)
{
}


SpherePlaneProjector::~SpherePlaneProjector()
{
}

osg::Quat SpherePlaneProjector::getRotation(const osg::Vec3& p1, bool p1OnSphere, const osg::Vec3& p2, bool p2OnSphere,
                                            float radialFactor) const
{
    if (p1OnSphere && p2OnSphere)
    {
        osg::Quat rotation;
        if (_front)
            rotation.makeRotate(p1 - getSphere()->getCenter(), p2 - getSphere()->getCenter());
        else
            rotation.makeRotate(p2 - getSphere()->getCenter(), p1 - getSphere()->getCenter());
        return rotation;
    }
    else if (!p1OnSphere && !p2OnSphere)
    {
        osg::Quat rotation;
        rotation.makeRotate(p1 - getSphere()->getCenter(), p2 - getSphere()->getCenter());

        osg::Vec3 axis; double angle;
        rotation.getRotate(angle, axis);

        osg::Vec3 realAxis;
        if (axis * _plane.getNormal() > 0.0f)
            realAxis = _plane.getNormal();
        else
            realAxis = - _plane.getNormal();

        osg::Quat rollRotation(angle, realAxis);

        osg::Vec3 diff1 = p1 - getSphere()->getCenter();
        osg::Vec3 diff2 = p2 - getSphere()->getCenter();
        float d = diff2.length() - diff1.length();

        float theta = d / getSphere()->getRadius();
        if (fabs(theta) < 0.000001 || fabs(theta) > 1.0)
            return rollRotation;

        diff1.normalize();
        osg::Vec3 pullAxis = diff1 ^ _plane.getNormal();
        pullAxis.normalize();
        osg::Quat pullRotation(radialFactor * theta, pullAxis);

        osg::Quat totalRotation = pullRotation * rollRotation;
        return totalRotation;
    }
    else
    {
        const osg::Vec3& planePoint = getSphere()->getCenter();

        osg::Vec3 intersection, dontCare;
        if (p1OnSphere)
            getSphereLineIntersection(*getSphere(), p2, planePoint, intersection, dontCare);
        else
            getSphereLineIntersection(*getSphere(), p1, planePoint, intersection, dontCare);

        osg::Quat rotation;
        if (p1OnSphere)
            rotation.makeRotate(p1 - getSphere()->getCenter(), intersection - getSphere()->getCenter());
        else
            rotation.makeRotate(intersection - getSphere()->getCenter(), p2 - getSphere()->getCenter());
        return rotation;
    }
}

bool SpherePlaneProjector::project(const PointerInfo& pi, osg::Vec3& projectedPoint) const
{
    if (!_sphere->valid())
    {
        osg::notify(osg::WARN) << "Warning: Invalid sphere. SpherePlaneProjector::project() failed." << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3 nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::Vec3 sphereIntersection, dontCare;
    bool hitSphere = false;
    if (_front)
        hitSphere = getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, sphereIntersection, dontCare);
    else
        hitSphere = getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, dontCare, sphereIntersection);

    // Compute plane oriented to the eye.
    _plane = computePlaneThruPointAndOrientedToEye(pi.getEyeDir(), getLocalToWorld(), getSphere()->getCenter(), _front);

    // Find the intersection on the plane.
    osg::Vec3 planeIntersection;
    if (hitSphere)
    {
        if (! getPlaneLineIntersection(_plane.asVec4(), sphereIntersection, sphereIntersection + _plane.getNormal(), planeIntersection))
            return false;
    }
    else
    {
        if (! getPlaneLineIntersection(_plane.asVec4(), objectNearPoint, objectFarPoint, planeIntersection))
            return false;
    }

    // Distance from the plane intersection point to the center of the sphere.
    float dist = (planeIntersection - getSphere()->getCenter()).length();

    // If the distance is less that the sphere radius choose the sphere intersection else choose
    // the plane intersection.
    if (dist < getSphere()->getRadius())
    {
        if (! hitSphere) return false;
        projectedPoint = sphereIntersection;
        _onSphere = true;
    }
    else
    {
        projectedPoint = planeIntersection;
        _onSphere = false;
    }
    return true;
}

CylinderProjector::CylinderProjector() : _cylinder(new osg::Cylinder()), _cylinderAxis(0.0,0.0,1.0), _front(true)
{
}

CylinderProjector::CylinderProjector(osg::Cylinder* cylinder) : _front(true)
{
    setCylinder(cylinder);
}

CylinderProjector::~CylinderProjector()
{
}

bool CylinderProjector::project(const PointerInfo& pi, osg::Vec3& projectedPoint) const
{
    if (!_cylinder.valid())
    {
        osg::notify(osg::WARN) << "Warning: Invalid cylinder. CylinderProjector::project() failed." 
                               << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3 nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::Vec3 dontCare;
    return getCylinderLineIntersection(*_cylinder, objectNearPoint, objectFarPoint, projectedPoint, dontCare);
}

bool CylinderProjector::isPointInFront(const PointerInfo& pi, const osg::Matrix& localToWorld) const
{
    osg::Vec3 closestPointOnAxis;
    computeClosestPointOnLine(getCylinder()->getCenter(), getCylinder()->getCenter() + _cylinderAxis,
                              pi.getLocalIntersectPoint(), closestPointOnAxis);

    osg::Vec3 perpPoint = pi.getLocalIntersectPoint() - closestPointOnAxis;
    if (perpPoint * getLocalEyeDirection(pi.getEyeDir(), localToWorld) < 0.0)
        return false;
    return true;
}

CylinderPlaneProjector::CylinderPlaneProjector()
{
}

CylinderPlaneProjector::CylinderPlaneProjector(osg::Cylinder* cylinder) : CylinderProjector(cylinder)
{
}

CylinderPlaneProjector::~CylinderPlaneProjector()
{
}

bool CylinderPlaneProjector::project(const PointerInfo& pi, osg::Vec3& projectedPoint) const
{
    if (!_cylinder.valid())
    {
        osg::notify(osg::WARN) << "Warning: Invalid cylinder. CylinderProjector::project() failed." 
                               << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3 nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::Vec3 cylIntersection;
    bool hitCylinder = false;
    if (_front)
    {
        osg::Vec3 dontCare;
        hitCylinder = getCylinderLineIntersection(*_cylinder, objectNearPoint, objectFarPoint, cylIntersection, dontCare);
    }
    else
    {
        osg::Vec3 dontCare;
        hitCylinder = getCylinderLineIntersection(*_cylinder, objectNearPoint, objectFarPoint, dontCare, cylIntersection);
    }

    // Compute plane oriented to the eye.
    _plane = computePlaneParallelToAxisAndOrientedToEye(pi.getEyeDir(), getLocalToWorld(), _cylinderAxis,
                                                        getCylinder()->getRadius(), _planeLineStart, _planeLineEnd,
                                   _front);

    // Find the intersection on the plane.
    osg::Vec3 planeIntersection;
    getPlaneLineIntersection(_plane.asVec4(), objectNearPoint, objectFarPoint, planeIntersection);

    if (hitCylinder)
    {
        osg::Vec3 projectIntersection;
        getPlaneLineIntersection(_plane.asVec4(), cylIntersection, cylIntersection + _plane.getNormal(), projectIntersection);

        osg::Vec3 closestPointToCylAxis;
        computeClosestPointOnLine(getCylinder()->getCenter(), getCylinder()->getCenter() + _cylinderAxis,
                                  projectIntersection, closestPointToCylAxis);

        // Distance from the plane intersection point to the closest point on the cylinder axis.
        float dist = (projectIntersection - closestPointToCylAxis).length();

        if (dist < getCylinder()->getRadius())
        {
            if (!hitCylinder) return false;
            projectedPoint = cylIntersection;
            _onCylinder = true;
        }
        else
        {
            projectedPoint = planeIntersection;
            _onCylinder = false;
        }
    }
    else
    {
        projectedPoint = planeIntersection;
        _onCylinder = false;
    }

    return true;
}

osg::Quat CylinderPlaneProjector::getRotation(const osg::Vec3& p1, bool p1OnCyl, const osg::Vec3& p2, bool p2OnCyl) const
{
    if (p1OnCyl && p2OnCyl)
    {
        osg::Vec3 closestPointToCylAxis1, closestPointToCylAxis2;
        computeClosestPointOnLine(getCylinder()->getCenter(), getCylinder()->getCenter() + _cylinderAxis * getCylinder()->getHeight(),
                                  p1, closestPointToCylAxis1);
        computeClosestPointOnLine(getCylinder()->getCenter(), getCylinder()->getCenter() + _cylinderAxis * getCylinder()->getHeight(),
                                  p2, closestPointToCylAxis2);

        osg::Vec3 v1 = p1 - closestPointToCylAxis1;
        osg::Vec3 v2 = p2 - closestPointToCylAxis2;

        float cosAngle = v1 * v2 / (v1.length() * v2.length());

        if (cosAngle > 1.0 || cosAngle < -1.0)
            return osg::Quat();

        float angle = acosf(cosAngle);
        osg::Vec3 rotAxis = v1 ^ v2;

        return osg::Quat(angle, rotAxis);
    }
    else if (!p1OnCyl && !p2OnCyl)
    {
        osg::Vec3 closestPointToPlaneLine1, closestPointToPlaneLine2;
        computeClosestPointOnLine(_planeLineStart, _planeLineEnd,
                                  p1, closestPointToPlaneLine1);
        computeClosestPointOnLine(_planeLineStart, _planeLineEnd,
                                  p2, closestPointToPlaneLine2);

        osg::Vec3 v1 = p1 - closestPointToPlaneLine1;
        osg::Vec3 v2 = p2 - closestPointToPlaneLine2;

        osg::Vec3 diff = v2 - v1;
        float d = diff.length();

        float angle = (getCylinder()->getRadius() == 0.0) ? 0.0 : (d / getCylinder()->getRadius());
        osg::Vec3 rotAxis = _plane.getNormal() ^ v1;

        if (v2.length() > v1.length())
            return osg::Quat(angle, rotAxis);
        else
            return osg::Quat(-angle, rotAxis);

    }
    else
    {
        osg::Vec3 offCylinderPt = (p1OnCyl) ? p2 : p1;

        osg::Vec3 linePtNearest;
        computeClosestPointOnLine(_planeLineStart, _planeLineEnd,
                                  offCylinderPt, linePtNearest);
        osg::Vec3 dirToOffCylinderPt = offCylinderPt - linePtNearest;
        dirToOffCylinderPt.normalize();

        osg::Vec3 ptOnCylinder = linePtNearest + dirToOffCylinderPt * getCylinder()->getRadius();

        if (p1OnCyl)
            return (getRotation(p1, true, ptOnCylinder, true) *
                    getRotation(ptOnCylinder, false, p2, false));
        else
            return (getRotation(p1, false, ptOnCylinder, false) *
                    getRotation(ptOnCylinder, true, p2, true));
    }
}
