/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield
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

#ifndef OSG_LINESEGMENT
#define OSG_LINESEGMENT 1

#include <osg/Matrix>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>

namespace osg {

/** LineSegment class for representing a line segment. */
class OSG_EXPORT LineSegment : public Referenced
{
    public:

        typedef Vec3d vec_type;
        typedef vec_type::value_type value_type;

        LineSegment() {};
        LineSegment(const LineSegment& seg) : Referenced(),_s(seg._s),_e(seg._e) {}
        LineSegment(const vec_type& s,const vec_type& e) : _s(s),_e(e) {}

        LineSegment& operator = (const LineSegment& seg) { _s = seg._s;  _e = seg._e; return *this; }

        inline void set(const vec_type& s,const vec_type& e) { _s=s; _e=e; }

        inline vec_type& start() { return _s; }
        inline const vec_type& start() const { return _s; }

        inline vec_type& end() { return _e; }
        inline const vec_type& end() const { return _e; }

        inline bool valid() const { return _s.valid() && _e.valid() && _s!=_e; }


        /** return true if segment intersects BoundingBox. */
        bool intersect(const BoundingBox& bb) const;

        /** return true if segment intersects BoundingBox and
          * set float ratios for the first and second intersections, where the ratio is 0.0 at the segment start point, and 1.0 at the segment end point.
        */
        bool intersectAndComputeRatios(const BoundingBox& bb, float& ratioFromStartToEnd1, float& ratioFromStartToEnd2) const;

        /** return true if segment intersects BoundingBox and
          * set double ratios for the first and second intersections, where the ratio is 0.0 at the segment start point, and 1.0 at the segment end point.
        */
        bool intersectAndComputeRatios(const BoundingBox& bb, double& ratioFromStartToEnd1, double& ratioFromStartToEnd2) const;


        /** return true if segment intersects BoundingSphere. */
        bool intersect(const BoundingSphere& bs) const;

        /** return true if segment intersects BoundingSphere and
          * set float ratios for the first and second intersections, where the ratio is 0.0 at the segment start point, and 1.0 at the segment end point.
        */
        bool intersectAndComputeRatios(const BoundingSphere& bs, float& ratioFromStartToEnd1, float& ratioFromStartToEnd2) const;

        /** return true if segment intersects BoundingSphere and
          * set double ratios for the first and second intersections, where the ratio is 0.0 at the segment start point, and 1.0 at the segment end point.
        */
        bool intersectAndComputeRatios(const BoundingSphere& bs,double& ratioFromStartToEnd1, double& ratioFromStartToEnd2) const;

        /** return true if segment intersects triangle and
          * set float ratios where the ratio is 0.0 at the segment start point, and 1.0 at the segment end point.
        */
        bool intersect(const Vec3f& v1,const Vec3f& v2,const Vec3f& v3,float& ratioFromStartToEnd);

        /** return true if segment intersects triangle and
          * set double ratios where the ratio is 0.0 at the segment start point, and 1.0 at the segment end point.
        */
        bool intersect(const Vec3d& v1,const Vec3d& v2,const Vec3d& v3,double& ratioFromStartToEnd);


        /** post multiply a segment by matrix.*/
        inline void mult(const LineSegment& seg,const Matrix& m) { _s = seg._s*m; _e = seg._e*m; }

        /** pre multiply a segment by matrix.*/
        inline void mult(const Matrix& m,const LineSegment& seg) { _s = m*seg._s; _e = m*seg._e; }

    protected:

        virtual ~LineSegment();

        static bool intersectAndClip(vec_type& s,vec_type& e,const BoundingBox& bb);

        vec_type _s;
        vec_type _e;
};

}

#endif
