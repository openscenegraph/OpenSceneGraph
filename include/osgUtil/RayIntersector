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

#ifndef OSGUTIL_RAYINTERSECTOR
#define OSGUTIL_RAYINTERSECTOR 1

#include <osgUtil/IntersectionVisitor>

namespace osgUtil
{

/** RayIntersector implements possibly-infinite line intersections with the scene graph.
  *
  * Compared with LineSegmentIntersector, RayIntersector supports infinite intersection
  * lines, start and end point can be given in homogeneous coordinates and projection
  * matrix is allowed to have z-far plane at infinity (often used in shadow volume
  * technique).
  *
  * Currently, picking of objects at infinity is not supported. Please, contribute.
  *
  * The class is be used in conjunction with IntersectionVisitor. */
class OSGUTIL_EXPORT RayIntersector : public Intersector
{
    public:

        /** Construct a RayIntersector. You will need to provide start and end point,
         *  or start point and direction. See setStart() and setDirecton(). */
        RayIntersector(CoordinateFrame cf = MODEL, RayIntersector* parent = NULL,
                       osgUtil::Intersector::IntersectionLimit intersectionLimit = osgUtil::Intersector::NO_LIMIT);

        /** Construct a RayIntersector that runs from start point in specified direction to the infinity.
         *  Start and direction are provided in MODEL coordinates. */
        RayIntersector(const osg::Vec3d& start, const osg::Vec3d& direction);

        /** Construct a RayIntersector the runs from start point in specified direction to the infinity in the specified coordinate frame. */
        RayIntersector(CoordinateFrame cf, const osg::Vec3d& start, const osg::Vec3d& direction, RayIntersector* parent = NULL,
                       osgUtil::Intersector::IntersectionLimit intersectionLimit = osgUtil::Intersector::NO_LIMIT);

        /** Convenience constructor for supporting picking in WINDOW and PROJECTION coordinates.
          * In WINDOW coordinates, it creates a start value of (x,y,0) and end value of (x,y,1).
          * In PROJECTION coordinates (clip space cube), it creates a start value of (x,y,-1) and end value of (x,y,1).
          * In VIEW and MODEL coordinates, it creates a start value of (x,y,0) and end value of (x,y,1).*/
        RayIntersector(CoordinateFrame cf, double x, double y);

        struct OSGUTIL_EXPORT Intersection
        {
            Intersection() : distance(-1.0), primitiveIndex(0) {}

            bool operator < (const Intersection& rhs) const { return distance < rhs.distance; }

            typedef std::vector<unsigned int>   IndexList;
            typedef std::vector<double>         RatioList;

            double                          distance;
            osg::NodePath                   nodePath;
            osg::ref_ptr<osg::Drawable>     drawable;
            osg::ref_ptr<osg::RefMatrix>    matrix;
            osg::Vec3d                      localIntersectionPoint;
            osg::Vec3                       localIntersectionNormal;
            IndexList                       indexList;
            RatioList                       ratioList;
            unsigned int                    primitiveIndex;

            const osg::Vec3d& getLocalIntersectPoint() const { return localIntersectionPoint; }
            osg::Vec3d getWorldIntersectPoint() const { return matrix.valid() ? localIntersectionPoint * (*matrix) : localIntersectionPoint; }

            const osg::Vec3& getLocalIntersectNormal() const { return localIntersectionNormal; }
            osg::Vec3 getWorldIntersectNormal() const { return matrix.valid() ? osg::Matrix::transform3x3(osg::Matrix::inverse(*matrix),localIntersectionNormal) : localIntersectionNormal; }

            /** Convenience function for mapping the intersection point to any textures assigned to the objects intersected.
             *  Returns the Texture pointer and texture coords of object hit when a texture is available on the object, returns NULL otherwise.*/
            osg::Texture* getTextureLookUp(osg::Vec3& tc) const;

        };

        typedef std::multiset<Intersection> Intersections;

        inline void insertIntersection(const Intersection& intersection) { getIntersections().insert(intersection); }
        inline Intersections& getIntersections() { return _parent ? _parent->_intersections : _intersections; }
        inline Intersection getFirstIntersection() { Intersections& intersections = getIntersections(); return intersections.empty() ? Intersection() : *(intersections.begin()); }

        virtual void setStart(const osg::Vec3d& start) { _start = start; }
        inline const osg::Vec3d& getStart() const { return _start; }

        virtual void setDirection(const osg::Vec3d& dir) { _direction = dir; }
        inline const osg::Vec3d& getDirection() const { return _direction; }

    public:

        virtual Intersector* clone(osgUtil::IntersectionVisitor& iv);

        virtual bool enter(const osg::Node& node);

        virtual void leave();

        virtual void intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable);

        virtual void reset();

        virtual bool containsIntersections() { return !getIntersections().empty(); }

    protected:

        virtual bool intersects(const osg::BoundingSphere& bs);
        bool intersectAndClip(osg::Vec3d& s, const osg::Vec3d& d, osg::Vec3d& e, const osg::BoundingBox& bb);

        RayIntersector* _parent;

        osg::Vec3d _start;
        osg::Vec3d _direction;

        Intersections _intersections;

};

}

#endif
