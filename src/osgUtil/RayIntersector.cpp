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


#include <osgUtil/RayIntersector>
#include <osgUtil/LineSegmentIntersector>
#include <osg/KdTree>
#include <osg/Notify>
#include <osg/TexMat>
#include <limits>
#include <cmath>

using namespace osg;
using namespace osgUtil;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  RayIntersector
//

RayIntersector::RayIntersector(CoordinateFrame cf, RayIntersector* parent,
                               Intersector::IntersectionLimit intersectionLimit) :
    Intersector(cf, intersectionLimit),
    _parent(parent)
{
    if (parent) setPrecisionHint(parent->getPrecisionHint());
}

RayIntersector::RayIntersector(const Vec3d& start, const Vec3d& direction) :
    Intersector(),
    _parent(0),
    _start(start),
    _direction(direction)
{
}

RayIntersector::RayIntersector(CoordinateFrame cf, const Vec3d& start, const Vec3d& direction,
                               RayIntersector* parent, Intersector::IntersectionLimit intersectionLimit) :
    Intersector(cf, intersectionLimit),
    _parent(parent),
    _start(start),
    _direction(direction)
{
    if (parent) setPrecisionHint(parent->getPrecisionHint());
}

RayIntersector::RayIntersector(CoordinateFrame cf, double x, double y) :
    Intersector(cf),
    _parent(0)
{
    switch(cf)
    {
        case WINDOW:     setStart(Vec3d(x,y,0.));  setDirection(Vec3d(0.,0.,1.)); break;
        case PROJECTION: setStart(Vec3d(x,y,-1.)); setDirection(Vec3d(0.,0.,1.)); break;
        case VIEW:       setStart(Vec3d(x,y,0.));  setDirection(Vec3d(0.,0.,1.)); break;
        case MODEL:      setStart(Vec3d(x,y,0.));  setDirection(Vec3d(0.,0.,1.)); break;
    }
}

Intersector* RayIntersector::clone(IntersectionVisitor& iv)
{
    if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
    {
        return new RayIntersector(MODEL, _start, _direction, this, _intersectionLimit);
    }

    Matrix matrix(LineSegmentIntersector::getTransformation(iv, _coordinateFrame));

    Vec3d newStart = _start * matrix;
    Vec4d tmp = Vec4d(_start + _direction, 1.) * matrix;
    Vec3d newEnd = Vec3d(tmp.x(), tmp.y(), tmp.z()) - (newStart * tmp.w());
    return new RayIntersector(MODEL, newStart, newEnd, this, _intersectionLimit);
}

bool RayIntersector::enter(const Node& node)
{
    if (reachedLimit()) return false;
    return !node.isCullingActive() || intersects( node.getBound() );
}

void RayIntersector::leave()
{
    // do nothing
}

void RayIntersector::reset()
{
    Intersector::reset();

    _intersections.clear();
}

void RayIntersector::intersect(IntersectionVisitor& iv, Drawable* drawable)
{
    // did we reached what we wanted as specified by setIntersectionLimit()?
    if (reachedLimit()) return;

    // clip ray to finite line segment
    Vec3d s(_start), e;
    if (!intersectAndClip(s, _direction, e, drawable->getBoundingBox())) return;

    // dummy traversal
    if (iv.getDoDummyTraversal()) return;

    // get intersections using LineSegmentIntersector
    LineSegmentIntersector lsi(MODEL, s, e, NULL, _intersectionLimit);
    lsi.setPrecisionHint(getPrecisionHint());
    lsi.intersect(iv, drawable, s, e);

    // copy intersections from LineSegmentIntersector
    LineSegmentIntersector::Intersections intersections = lsi.getIntersections();
    if (!intersections.empty())
    {
        double preLength = (s - _start).length();
        double esLength = (e - s).length();

        for(LineSegmentIntersector::Intersections::iterator it = intersections.begin();
            it != intersections.end(); it++)
        {
            Intersection hit;
            hit.distance = preLength + it->ratio * esLength;
            hit.matrix = it->matrix;
            hit.nodePath = it->nodePath;
            hit.drawable = it->drawable;
            hit.primitiveIndex = it->primitiveIndex;

            hit.localIntersectionPoint = it->localIntersectionPoint;
            hit.localIntersectionNormal = it->localIntersectionNormal;

            hit.indexList = it->indexList;
            hit.ratioList = it->ratioList;

            insertIntersection(hit);
        }
    }
}

bool RayIntersector::intersects(const BoundingSphere& bs)
{
    // if bs not valid then return true based on the assumption that an invalid sphere is yet to be defined.
    if (!bs.valid()) return true;

    // test for _start inside the bounding sphere
    Vec3d sm = _start - bs._center;
    double c = sm.length2() - bs._radius * bs._radius;
    if (c<0.0) return true;

    // solve quadratic equation
    double a = _direction.length2();
    double b = (sm * _direction) * 2.0;
    double d = b * b - 4.0 * a * c;

    // no intersections if d<0
    if (d<0.0) return false;

    // compute two solutions of quadratic equation
    d = sqrt(d);
    double div = 1.0/(2.0*a);
    double r1 = (-b-d)*div;
    double r2 = (-b+d)*div;

    // return false if both intersections are before the ray start
    if (r1<=0.0 && r2<=0.0) return false;

    // if LIMIT_NEAREST and closest point of bounding sphere is further than already found intersection, return false
    if (_intersectionLimit == LIMIT_NEAREST && !getIntersections().empty())
    {
        double minDistance = sm.length() - bs._radius;
        if (minDistance >= getIntersections().begin()->distance) return false;
    }

    // passed all the rejection tests so line must intersect bounding sphere, return true.
    return true;
}

bool RayIntersector::intersectAndClip(Vec3d& s, const Vec3d& d, Vec3d& e, const BoundingBox& bbInput)
{
    // bounding box min and max
    Vec3d bb_min(bbInput._min);
    Vec3d bb_max(bbInput._max);

    // Expand the extents of the bounding box by the epsilon to prevent numerical errors resulting in misses.
    const double epsilon = 1e-6;

    // clip s against all three components of the Min to Max range of bb
    for (int i=0; i<3; i++)
    {
        // test direction
        if (d[i] >= 0.)
        {
            // trivial reject of segment wholly outside
            if (s[i] > bb_max[i]) return false;

            if ((d[i] > epsilon) && (s[i] < bb_min[i]))
            {
                // clip s to xMin
                double t = (bb_min[i]-s[i])/d[i] - epsilon;
                if (t>0.0) s = s + d*t;
            }
        }
        else
        {
            // trivial reject of segment wholly outside
            if (s[i] < bb_min[i]) return false;

            if ((d[i] < -epsilon) && (s[i] > bb_max[i]))
            {
                // clip s to xMax
                double t = (bb_max[i]-s[i])/d[i] - epsilon;
                if (t>0.0) s = s + d*t;
            }
        }
    }

    // t for ending point of clipped ray
    double end_t = std::numeric_limits<double>::infinity();

    // get end point by clipping the ray by bb
    // note: this can not be done in previous loop as start point s is moving
    for (int i=0; i<3; i++)
    {
        // test direction
        if (d[i] >= epsilon)
        {
            // compute end_t based on xMax
            double t = (bb_max[i]-s[i])/d[i] + epsilon;
            if (t < end_t)
                end_t = t;
        }
        else if (d[i] <= -epsilon)
        {
            // compute end_t based on xMin
            double t = (bb_min[i]-s[i])/d[i] + epsilon;
            if (t < end_t)
                end_t = t;
        }
    }

    // if we failed to clamp the end point return false
    if (end_t==std::numeric_limits<double>::infinity()) return false;

    // compute e
    e = s + d*end_t;

    return true;
}

Texture* RayIntersector::Intersection::getTextureLookUp(Vec3& tc) const
{
    Geometry* geometry = drawable.valid() ? drawable->asGeometry() : 0;
    Vec3Array* vertices = geometry ? dynamic_cast<Vec3Array*>(geometry->getVertexArray()) : 0;

    if (vertices)
    {
        if (indexList.size()==3 && ratioList.size()==3)
        {
            unsigned int i1 = indexList[0];
            unsigned int i2 = indexList[1];
            unsigned int i3 = indexList[2];

            float r1 = ratioList[0];
            float r2 = ratioList[1];
            float r3 = ratioList[2];

            Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
            FloatArray* texcoords_FloatArray = dynamic_cast<FloatArray*>(texcoords);
            Vec2Array* texcoords_Vec2Array = dynamic_cast<Vec2Array*>(texcoords);
            Vec3Array* texcoords_Vec3Array = dynamic_cast<Vec3Array*>(texcoords);
            if (texcoords_FloatArray)
            {
                // we have tex coord array so now we can compute the final tex coord at the point of intersection.
                float tc1 = (*texcoords_FloatArray)[i1];
                float tc2 = (*texcoords_FloatArray)[i2];
                float tc3 = (*texcoords_FloatArray)[i3];
                tc.x() = tc1*r1 + tc2*r2 + tc3*r3;
            }
            else if (texcoords_Vec2Array)
            {
                // we have tex coord array so now we can compute the final tex coord at the point of intersection.
                const Vec2& tc1 = (*texcoords_Vec2Array)[i1];
                const Vec2& tc2 = (*texcoords_Vec2Array)[i2];
                const Vec2& tc3 = (*texcoords_Vec2Array)[i3];
                tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
                tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
            }
            else if (texcoords_Vec3Array)
            {
                // we have tex coord array so now we can compute the final tex coord at the point of intersection.
                const Vec3& tc1 = (*texcoords_Vec3Array)[i1];
                const Vec3& tc2 = (*texcoords_Vec3Array)[i2];
                const Vec3& tc3 = (*texcoords_Vec3Array)[i3];
                tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
                tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
                tc.z() = tc1.z()*r1 + tc2.z()*r2 + tc3.z()*r3;
            }
            else
            {
                return 0;
            }
        }

        const TexMat* activeTexMat = 0;
        const Texture* activeTexture = 0;

        if (drawable->getStateSet())
        {
            const TexMat* texMat = dynamic_cast<TexMat*>(drawable->getStateSet()->getTextureAttribute(0,StateAttribute::TEXMAT));
            if (texMat) activeTexMat = texMat;

            const Texture* texture = dynamic_cast<Texture*>(drawable->getStateSet()->getTextureAttribute(0,StateAttribute::TEXTURE));
            if (texture) activeTexture = texture;
        }

        for(NodePath::const_reverse_iterator itr = nodePath.rbegin();
            itr != nodePath.rend() && (!activeTexMat || !activeTexture);
            ++itr)
            {
                const Node* node = *itr;
                if (node->getStateSet())
                {
                    if (!activeTexMat)
                    {
                        const TexMat* texMat = dynamic_cast<const TexMat*>(node->getStateSet()->getTextureAttribute(0,StateAttribute::TEXMAT));
                        if (texMat) activeTexMat = texMat;
                    }

                    if (!activeTexture)
                    {
                        const Texture* texture = dynamic_cast<const Texture*>(node->getStateSet()->getTextureAttribute(0,StateAttribute::TEXTURE));
                        if (texture) activeTexture = texture;
                    }
                }
            }

            if (activeTexMat)
            {
                Vec4 tc_transformed = Vec4(tc.x(), tc.y(), tc.z() ,0.0f) * activeTexMat->getMatrix();
                tc.x() = tc_transformed.x();
                tc.y() = tc_transformed.y();
                tc.z() = tc_transformed.z();

                if (activeTexture && activeTexMat->getScaleByTextureRectangleSize())
                {
                    tc.x() *= static_cast<float>(activeTexture->getTextureWidth());
                    tc.y() *= static_cast<float>(activeTexture->getTextureHeight());
                    tc.z() *= static_cast<float>(activeTexture->getTextureDepth());
                }
            }

            return const_cast<Texture*>(activeTexture);

    }
    return 0;
}
