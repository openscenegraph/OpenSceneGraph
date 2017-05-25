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

#ifndef OSGUTIL_POLYTOPEINTERSECTOR
#define OSGUTIL_POLYTOPEINTERSECTOR 1

#include <osgUtil/IntersectionVisitor>

namespace osgUtil
{

/** Concrete class for implementing polytope intersections with the scene graph.
  * To be used in conjunction with IntersectionVisitor. */
class OSGUTIL_EXPORT PolytopeIntersector : public Intersector
{
    public:

        /** Construct a PolytopeIntersector using specified polytope in MODEL coordinates.*/
        PolytopeIntersector(const osg::Polytope& polytope);

        /** Construct a PolytopeIntersector using specified polytope in specified coordinate frame.*/
        PolytopeIntersector(CoordinateFrame cf, const osg::Polytope& polytope);

        /** Convenience constructor for supporting picking in WINDOW, or PROJECTION coordinates
          * In WINDOW coordinates (clip space cube) creates a five sided polytope box that has a front face at 0.0 and sides around box xMin, yMin, xMax, yMax.
          * In PROJECTION coordinates (clip space cube) creates a five sided polytope box that has a front face at -1 and sides around box xMin, yMin, xMax, yMax.
          * In VIEW and MODEL coordinates (clip space cube) creates a five sided polytope box that has a front face at 0.0 and sides around box xMin, yMin, xMax, yMax.*/
        PolytopeIntersector(CoordinateFrame cf, double xMin, double yMin, double xMax, double yMax);

        /** Get the Polytope used by the intersector.*/
        osg::Polytope& getPolytope() { return _polytope;}

        /** Get the const Polytope used by the intersector.*/
        const osg::Polytope& getPolytope() const { return _polytope;}


        typedef osg::Plane::Vec3_type Vec3_type;

        struct Intersection
        {
            Intersection():
                distance(0.0),
                maxDistance(0.0),
                numIntersectionPoints(0),
                primitiveIndex(0) {}

            bool operator < (const Intersection& rhs) const
            {
                if (distance < rhs.distance) return true;
                if (rhs.distance < distance) return false;
                if (primitiveIndex < rhs.primitiveIndex) return true;
                if (rhs.primitiveIndex < primitiveIndex) return false;
                if (nodePath < rhs.nodePath) return true;
                if (rhs.nodePath < nodePath ) return false;
                return (drawable < rhs.drawable);
            }

            enum { MaxNumIntesectionPoints=6 };

            double                          distance;     ///< distance from reference plane
            double                          maxDistance;  ///< maximum distance of intersection points from reference plane
            osg::NodePath                   nodePath;
            osg::ref_ptr<osg::Drawable>     drawable;
            osg::ref_ptr<osg::RefMatrix>    matrix;
            Vec3_type                       localIntersectionPoint;  ///< center of all intersection points
            unsigned int                    numIntersectionPoints;
            Vec3_type                       intersectionPoints[MaxNumIntesectionPoints];
            unsigned int                    primitiveIndex; ///< primitive index
        };

        typedef std::set<Intersection> Intersections;

        inline void insertIntersection(const Intersection& intersection) { getIntersections().insert(intersection); }

        inline Intersections& getIntersections() { return _parent ? _parent->_intersections : _intersections; }

        inline Intersection getFirstIntersection() { Intersections& intersections = getIntersections(); return intersections.empty() ? Intersection() : *(intersections.begin()); }


        /// dimension enum to specify primitive types to check.
        enum {
            POINT_PRIMITIVES = (1<<0),      /// check for points
            LINE_PRIMITIVES = (1<<1),       /// check for lines
            TRIANGLE_PRIMITIVES = (1<<2),   /// check for triangles and other primitives like quad, polygons that can be decomposed into triangles
            ALL_PRIMITIVES = ( POINT_PRIMITIVES | LINE_PRIMITIVES | TRIANGLE_PRIMITIVES )
        };

        /** Set which Primitives should be tested for intersections.*/
        void setPrimitiveMask(unsigned int mask) { _primitiveMask = mask; }

        /** Get which Primitives should be tested for intersections.*/
        unsigned int getPrimitiveMask() const { return _primitiveMask; }

        /** set the plane used to sort the intersections.
         * The intersections are sorted by the distance of the localIntersectionPoint
         * and the reference plane. The default for the reference plane is the
         * last plane of the polytope.
         */
        inline void setReferencePlane(const osg::Plane& plane) { _referencePlane = plane; }

        inline const osg::Plane& getReferencePlane() const { return _referencePlane; }

#ifdef OSG_USE_DEPRECATED_API
        enum {
            DimZero = POINT_PRIMITIVES,    /// deprecated, use POINT_PRIMITIVES
            DimOne = LINE_PRIMITIVES,      /// deprecated, use POINT_PRIMITIVES
            DimTwo = TRIANGLE_PRIMITIVES,  /// deprecated, use POINT_PRIMITIVES
            AllDims =  ALL_PRIMITIVES      /// deprecated, use ALL_PRIMITIVES
        };

        /** deprecated, use setPrimtiveMask() */
        inline void setDimensionMask(unsigned int mask) { setPrimitiveMask(mask); }

        /** deprecated, use getPrimtiveMask() */
        inline unsigned int getDimensionMask() const { return getPrimitiveMask(); }
#endif

public:

        virtual Intersector* clone(osgUtil::IntersectionVisitor& iv);

        virtual bool enter(const osg::Node& node);

        virtual void leave();

        virtual void intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable);

        virtual void reset();

        virtual bool containsIntersections() { return !getIntersections().empty(); }

    protected:

        PolytopeIntersector* _parent;

        osg::Polytope _polytope;

        unsigned int _primitiveMask; ///< mask which dimensions should be checked
        osg::Plane _referencePlane; ///< plane to use for sorting intersections

        Intersections _intersections;

};

}

#endif

