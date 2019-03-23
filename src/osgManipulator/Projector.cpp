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

// When the squared magnitude (length2) of the cross product of 2
// angles is less than this tolerance, they are considered parallel.
// osg::Vec3 a, b; (a ^ b).length2()
#define CROSS_PRODUCT_ANGLE_TOLERANCE 1.0e-1

namespace
{

bool computeClosestPoints(const osg::LineSegment& l1, const osg::LineSegment& l2,
                          osg::Vec3d& p1, osg::Vec3d& p2)
{
    // Computes the closest points (p1 and p2 on line l1 and l2 respectively) between the two lines
    // An explanation of the algorithm can be found at
    // http://www.geometryalgorithms.com/Archive/algorithm_0106/algorithm_0106.htm

    osg::LineSegment::vec_type u = l1.end() - l1.start(); u.normalize();
    osg::LineSegment::vec_type v = l2.end() - l2.start(); v.normalize();

    osg::LineSegment::vec_type w0 = l1.start() - l2.start();

    double a = u * u;
    double b = u * v;
    double c = v * v;
    double d = u * w0;
    double e = v * w0;

    double denominator = a*c - b*b;

    // Test if lines are parallel
    if (denominator == 0.0) return false;

    double sc = (b*e - c*d)/denominator;
    double tc = (a*e - b*d)/denominator;

    p1 = l1.start() + u * sc;
    p2 = l2.start() + v * tc;

    return true;
}

bool computeClosestPointOnLine(const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd,
                               const osg::Vec3d& fromPoint, osg::Vec3d& closestPoint)
{
    osg::Vec3d v = lineEnd - lineStart;
    osg::Vec3d w = fromPoint - lineStart;

    double c1 = w * v;
    double c2 = v * v;

    double almostZero = 0.000001;
    if (c2 < almostZero) return false;

    double b = c1 / c2;
    closestPoint = lineStart + v * b;

    return true;
}

bool getPlaneLineIntersection(const osg::Vec4d& plane,
                              const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd,
                              osg::Vec3d& isect)
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
                               const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd,
                               osg::Vec3d& frontISect, osg::Vec3d& backISect)
{
    osg::Vec3d lineDirection = lineEnd - lineStart;
    lineDirection.normalize();

    osg::Vec3d v = lineStart - sphere.getCenter();
    double B = 2.0f * (lineDirection * v);
    double C = v * v - sphere.getRadius() * sphere.getRadius();

    double discriminant = B * B - 4.0f * C;

    if (discriminant < 0.0f) // Line and sphere don't intersect.
        return false;

    double discriminantSqroot = sqrtf(discriminant);
    double t0 = (-B - discriminantSqroot) * 0.5f;
    frontISect = lineStart + lineDirection * t0;

    double t1 = (-B + discriminantSqroot) * 0.5f;
    backISect = lineStart + lineDirection * t1;

    return true;
}

bool getUnitCylinderLineIntersection(const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd,
                                     osg::Vec3d& isectFront, osg::Vec3d& isectBack)
{
    osg::Vec3d dir = lineEnd - lineStart;
    dir.normalize();

    double a = dir[0] * dir[0] + dir[1] * dir[1];
    double b = 2.0f * (lineStart[0] * dir[0] + lineStart[1] * dir[1]);
    double c = lineStart[0] * lineStart[0] + lineStart[1] * lineStart[1] - 1;

    double d = b*b - 4*a*c;
    if (d < 0.0f) return false;

    double dSqroot = sqrtf(d);
    double t0, t1;
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
                                 const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd,
                                 osg::Vec3d& isectFront, osg::Vec3d& isectBack)
{
    // Compute matrix transformation that takes the cylinder to a unit cylinder with Z-axis as it's axis and
    // (0,0,0) as it's center.
    double oneOverRadius = 1.0f / cylinder.getRadius();
    osg::Matrix toUnitCylInZ = osg::Matrix::translate(-cylinder.getCenter())
                               * osg::Matrix::scale(oneOverRadius, oneOverRadius, oneOverRadius)
                               * osg::Matrix(cylinder.getRotation().inverse());

    // Transform the lineStart and lineEnd into the unit cylinder space.
    osg::Vec3d unitCylLineStart = lineStart * toUnitCylInZ;
    osg::Vec3d unitCylLineEnd   = lineEnd * toUnitCylInZ;

    // Intersect line with unit cylinder.
    osg::Vec3d unitCylIsectFront, unitCylIsectBack;
    if (! getUnitCylinderLineIntersection(unitCylLineStart, unitCylLineEnd, unitCylIsectFront, unitCylIsectBack))
        return false;

    // Transform back from unit cylinder space.
    osg::Matrix invToUnitCylInZ(osg::Matrix::inverse(toUnitCylInZ));
    isectFront = unitCylIsectFront * invToUnitCylInZ;
    isectBack = unitCylIsectBack * invToUnitCylInZ;

    return true;
}

osg::Vec3d getLocalEyeDirection(const osg::Vec3d& eyeDir, const osg::Matrix& localToWorld)
{
    // To take a normal from world to local you need to transform it by the transpose of the inverse of the
    // world to local matrix. Pre-multiplying is equivalent to doing a post-multiplication of the transpose.
    osg::Vec3d localEyeDir = localToWorld * eyeDir;
    localEyeDir.normalize();
    return localEyeDir;
}

osg::Plane computePlaneThruPointAndOrientedToEye(const osg::Vec3d& eyeDir, const osg::Matrix& localToWorld,
                                                 const osg::Vec3d& point, bool front)
{
    osg::Vec3d planeNormal = getLocalEyeDirection(eyeDir, localToWorld);
    if (! front) planeNormal = -planeNormal;

    osg::Plane plane;
    plane.set(planeNormal, point);
    return plane;
}

// Computes a plane to be used as a basis for determining a displacement.  When eyeDir is close
// to the cylinder axis, then the plane will be set to be perpendicular to the cylinder axis.
// Otherwise it will be set to be parallel to the cylinder axis and oriented towards eyeDir.
osg::Plane computeIntersectionPlane(const osg::Vec3d& eyeDir, const osg::Matrix& localToWorld,
                                    const osg::Vec3d& axisDir, const osg::Cylinder& cylinder,
                                    osg::Vec3d& planeLineStart, osg::Vec3d& planeLineEnd,
                                    bool& parallelPlane, bool front)
{
    osg::Plane plane;

    osg::Vec3d unitAxisDir = axisDir;
    unitAxisDir.normalize();
    osg::Vec3d perpDir = unitAxisDir ^ getLocalEyeDirection(eyeDir, localToWorld);

    // Check to make sure eye and cylinder axis are not too close
    if(perpDir.length2() < CROSS_PRODUCT_ANGLE_TOLERANCE)
    {
        // Too close, so instead return plane perpendicular to cylinder axis.
        plane.set(unitAxisDir, cylinder.getCenter());
        parallelPlane = false;
        return plane;
    }

    // Otherwise compute plane along axisDir oriented towards eye
    osg::Vec3d planeDir = perpDir ^ axisDir;
    planeDir.normalize();
    if (! front)
        planeDir = -planeDir;

    osg::Vec3d planePoint = planeDir * cylinder.getRadius() + axisDir;
    plane.set(planeDir, planePoint);

    planeLineStart = planePoint;
    planeLineEnd = planePoint + axisDir;
    parallelPlane = true;
    return plane;
}

} // namespace


Projector::Projector() : _worldToLocalDirty(false)
{
}

Projector::~Projector()
{
}

LineProjector::LineProjector()
{
    _line = new osg::LineSegment(osg::LineSegment::vec_type(0.0,0.0,0.0), osg::LineSegment::vec_type(1.0,0.0,0.0));
}

LineProjector::LineProjector(const osg::LineSegment::vec_type& s, const osg::LineSegment::vec_type& e)
{
    _line = new osg::LineSegment(s,e);
}

LineProjector::~LineProjector()
{
}

bool LineProjector::project(const PointerInfo& pi, osg::Vec3d& projectedPoint) const
{
    if (!_line->valid())
    {
        OSG_WARN << "Warning: Invalid line set. LineProjector::project() failed."<<std::endl;
        return false;
    }

    // Transform the line to world/object coordinate space.
    osg::ref_ptr<osg::LineSegment> objectLine = new osg::LineSegment;
    objectLine->mult(*_line, getLocalToWorld());

    // Get the near and far points for the mouse point.
    osg::Vec3d nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);
    osg::ref_ptr<osg::LineSegment> pointerLine = new osg::LineSegment(nearPoint,farPoint);

    osg::Vec3d closestPtLine, closestPtProjWorkingLine;
    if (! computeClosestPoints(*objectLine, *pointerLine, closestPtLine, closestPtProjWorkingLine))
        return false;

    osg::Vec3d localClosestPtLine = closestPtLine * getWorldToLocal();

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

bool PlaneProjector::project(const PointerInfo& pi, osg::Vec3d& projectedPoint) const
{
    if (!_plane.valid())
    {
        OSG_WARN << "Warning: Invalid plane set. PlaneProjector::project() failed."<< std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3d nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3d objectNearPoint, objectFarPoint;
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

bool SphereProjector::project(const PointerInfo& pi, osg::Vec3d& projectedPoint) const
{
    if (!_sphere->valid())
    {
        OSG_WARN << "Warning: Invalid sphere. SphereProjector::project() failed." << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3d nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3d objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::Vec3d dontCare;
    if (_front)
        return getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, projectedPoint, dontCare);
    return getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, dontCare, projectedPoint);
}

bool SphereProjector::isPointInFront(const PointerInfo& pi, const osg::Matrix& localToWorld) const
{
    osg::Vec3d centerToPoint = getSphere()->getCenter() - pi.getLocalIntersectPoint();
    if (centerToPoint * getLocalEyeDirection(pi.getEyeDir(), localToWorld) < 0.0)
        return false;
    return true;
}


SpherePlaneProjector::SpherePlaneProjector():
    _onSphere(false)
{
}

SpherePlaneProjector::SpherePlaneProjector(osg::Sphere* sphere) :
    SphereProjector(sphere),
    _onSphere(false)
{
}


SpherePlaneProjector::~SpherePlaneProjector()
{
}

osg::Quat SpherePlaneProjector::getRotation(const osg::Vec3d& p1, bool p1OnSphere, const osg::Vec3d& p2, bool p2OnSphere,
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

        osg::Vec3d axis; osg::Quat::value_type angle;
        rotation.getRotate(angle, axis);

        osg::Vec3d realAxis;
        if (axis * _plane.getNormal() > 0.0f)
            realAxis = _plane.getNormal();
        else
            realAxis = - _plane.getNormal();

        osg::Quat rollRotation(angle, realAxis);

        osg::Vec3d diff1 = p1 - getSphere()->getCenter();
        osg::Vec3d diff2 = p2 - getSphere()->getCenter();
        double d = diff2.length() - diff1.length();

        double theta = d / getSphere()->getRadius();
        if (fabs(theta) < 0.000001 || fabs(theta) > 1.0)
            return rollRotation;

        diff1.normalize();
        osg::Vec3d pullAxis = diff1 ^ _plane.getNormal();
        pullAxis.normalize();
        osg::Quat pullRotation(radialFactor * theta, pullAxis);

        osg::Quat totalRotation = pullRotation * rollRotation;
        return totalRotation;
    }
    else
    {
        const osg::Vec3d& planePoint = getSphere()->getCenter();

        osg::Vec3d intersection, dontCare;
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

bool SpherePlaneProjector::project(const PointerInfo& pi, osg::Vec3d& projectedPoint) const
{
    if (!_sphere->valid())
    {
        OSG_WARN << "Warning: Invalid sphere. SpherePlaneProjector::project() failed." << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3d nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3d objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::Vec3d sphereIntersection, dontCare;
    bool hitSphere = false;
    if (_front)
        hitSphere = getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, sphereIntersection, dontCare);
    else
        hitSphere = getSphereLineIntersection(*_sphere, objectNearPoint, objectFarPoint, dontCare, sphereIntersection);

    // Compute plane oriented to the eye.
    _plane = computePlaneThruPointAndOrientedToEye(pi.getEyeDir(), getLocalToWorld(), getSphere()->getCenter(), _front);

    // Find the intersection on the plane.
    osg::Vec3d planeIntersection;
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
    double dist = (planeIntersection - getSphere()->getCenter()).length();

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

bool CylinderProjector::project(const PointerInfo& pi, osg::Vec3d& projectedPoint) const
{
    if (!_cylinder.valid())
    {
        OSG_WARN << "Warning: Invalid cylinder. CylinderProjector::project() failed."
                               << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3d nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3d objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::Vec3d dontCare;
    return getCylinderLineIntersection(*_cylinder, objectNearPoint, objectFarPoint, projectedPoint, dontCare);
}

bool CylinderProjector::isPointInFront(const PointerInfo& pi, const osg::Matrix& localToWorld) const
{
    osg::Vec3d closestPointOnAxis;
    computeClosestPointOnLine(getCylinder()->getCenter(), getCylinder()->getCenter() + _cylinderAxis,
                              pi.getLocalIntersectPoint(), closestPointOnAxis);

    osg::Vec3d perpPoint = pi.getLocalIntersectPoint() - closestPointOnAxis;
    if (perpPoint * getLocalEyeDirection(pi.getEyeDir(), localToWorld) < 0.0)
        return false;
    return true;
}

CylinderPlaneProjector::CylinderPlaneProjector():
    _parallelPlane(false)
{
}

CylinderPlaneProjector::CylinderPlaneProjector(osg::Cylinder* cylinder):
    CylinderProjector(cylinder),
    _parallelPlane(false)
{
}

CylinderPlaneProjector::~CylinderPlaneProjector()
{
}

bool CylinderPlaneProjector::project(const PointerInfo& pi, osg::Vec3d& projectedPoint) const
{
    if (!_cylinder.valid())
    {
        OSG_WARN << "Warning: Invalid cylinder. CylinderProjector::project() failed."
                               << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3d nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3d objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Computes either a plane parallel to cylinder axis oriented to the eye or the plane
    // perpendicular to the cylinder axis if the eye-cylinder angle is close.
    _plane = computeIntersectionPlane(pi.getEyeDir(), getLocalToWorld(), _cylinderAxis,
                                      *_cylinder, _planeLineStart, _planeLineEnd,
                                     _parallelPlane, _front);

    // Now find the point of intersection on our newly-calculated plane.
    getPlaneLineIntersection(_plane.asVec4(), objectNearPoint, objectFarPoint, projectedPoint);
    return true;
}

osg::Quat CylinderPlaneProjector::getRotation(const osg::Vec3d& p1, const osg::Vec3d& p2) const
{
    if(_parallelPlane)
    {
        osg::Vec3d closestPointToPlaneLine1, closestPointToPlaneLine2;
        computeClosestPointOnLine(_planeLineStart, _planeLineEnd,
                                  p1, closestPointToPlaneLine1);
        computeClosestPointOnLine(_planeLineStart, _planeLineEnd,
                                  p2, closestPointToPlaneLine2);

        osg::Vec3d v1 = p1 - closestPointToPlaneLine1;
        osg::Vec3d v2 = p2 - closestPointToPlaneLine2;

        osg::Vec3d diff = v2 - v1;
        double d = diff.length();

        // The amount of rotation is inversely proportional to the size of the cylinder
        double angle = (getCylinder()->getRadius() == 0.0) ? 0.0 : (d / getCylinder()->getRadius());
        osg::Vec3d rotAxis = _plane.getNormal() ^ v1;

        if (v2.length() > v1.length())
           return osg::Quat(angle, rotAxis);
        else
           return osg::Quat(-angle, rotAxis);
    }
    else
    {
        osg::Vec3d v1 = p1 - getCylinder()->getCenter();
        osg::Vec3d v2 = p2 - getCylinder()->getCenter();

        double cosAngle = v1 * v2 / (v1.length() * v2.length());

        if (cosAngle > 1.0 || cosAngle < -1.0)
            return osg::Quat();

        double angle = acosf(cosAngle);
        osg::Vec3d rotAxis = v1 ^ v2;

        return osg::Quat(angle, rotAxis);
    }
}
