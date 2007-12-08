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


#include <osgUtil/PolytopeIntersector>

#include <osg/Geometry>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TemplatePrimitiveFunctor>

using namespace osgUtil;


namespace PolytopeIntersectorUtils
{
#ifdef OSG_USE_FLOAT_PLANE
    typedef float value_type;
    typedef osg::Vec3f Vec3_type;
#else
    typedef double value_type;
    typedef osg::Vec3d Vec3_type;
#endif
    typedef osg::Polytope::ClippingMask PlaneMask;
    typedef std::vector<std::pair<PlaneMask,Vec3_type> > CandList_t;


    class PolytopeIntersection {
    public:
       enum { MaxNumIntesections = PolytopeIntersector::Intersection::MaxNumIntesectionPoints };

        PolytopeIntersection(unsigned int index, const CandList_t& cands, const osg::Plane &referencePlane) :
            _maxDistance(-1.0), _index(index-1), _numPoints(0)
        {
            Vec3_type center;
            for (CandList_t::const_iterator it=cands.begin(); it!=cands.end(); ++it)
            {
                PlaneMask mask = it->first;
                if (mask==0) continue;

                _points[_numPoints++] = it->second;
                center += it->second;
                value_type distance = referencePlane.distance(it->second);
                if (distance > _maxDistance) _maxDistance = distance;
                if (_numPoints==MaxNumIntesections) break;
            }
            center /= value_type(_numPoints);
            _distance = referencePlane.distance( center );
        }
        bool operator<(const PolytopeIntersection& rhs) const { return _distance < rhs._distance; }

        value_type    _distance;    ///< distance from reference plane
        value_type    _maxDistance;    ///< maximum distance of intersection points from reference plane
        unsigned int    _index;         ///< primitive index
        unsigned int    _numPoints;
        osg::Vec3       _points[MaxNumIntesections];
    }; // class PolytopeIntersection

    typedef std::vector<PolytopeIntersection> Intersections;


    class PolytopePrimitiveIntersector {
    public:

        typedef osg::Polytope::PlaneList PlaneList;

        /// a line defined by the intersection of two planes
        struct PlanesLine 
        {
            PlanesLine(PlaneMask m, Vec3_type p, Vec3_type d) :
            mask(m), pos(p), dir(d) {}
            PlaneMask mask;
            Vec3_type pos;
            Vec3_type dir;
        };
        typedef std::vector<PlanesLine> LinesList;

        PolytopePrimitiveIntersector() :
            _index(0),
             _dimensionMask( PolytopeIntersector::AllDims ),
            _candidates(20) {}

            void addIntersection(unsigned int index, const CandList_t& cands) {
            intersections.push_back( PolytopeIntersection( index, cands, _referencePlane ) );
        }

        value_type eps() { return 1e-6; }

        /// check which candidate points lie within the polytope volume
        /// mark outliers with mask == 0, return number of remaining candidates
        unsigned int checkCandidatePoints(PlaneMask inside_mask)
        {
            PlaneMask selector_mask = 0x1;
            unsigned int numCands=_candidates.size();
            for(PlaneList::const_iterator it=_planes.begin();
                it!=_planes.end() && numCands>0;
                ++it, selector_mask <<= 1)
            {
                const osg::Plane& plane=*it;
                if (selector_mask & inside_mask) continue;
                
                for (CandList_t::iterator pointIt=_candidates.begin(); pointIt!=_candidates.end(); ++pointIt)
                {
                    PlaneMask& mask=pointIt->first;
                    if (mask==0) continue;
                    if (selector_mask & mask) continue;
                    if (plane.distance(pointIt->second)<0.0f)
                    {
                        mask=0;  // mark as outside
                        --numCands;
                        if (numCands==0) return 0;
                    }
                }
            }
            return numCands;
        }

        // handle points
        void operator()(const Vec3_type v1, bool treatVertexDataAsTemporary)
        {
            ++_index;
            if ((_dimensionMask & PolytopeIntersector::DimZero) == 0) return;
            for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end(); ++it)
            {
                const osg::Plane& plane=*it;
                const value_type d1=plane.distance(v1);
                if (d1<0.0f) return;   // point outside
            }
            _candidates.clear();
            _candidates.push_back( CandList_t::value_type(_plane_mask, v1));
            addIntersection(_index, _candidates);
        }

        // handle lines
        void operator()(const Vec3_type v1, const Vec3_type v2, bool treatVertexDataAsTemporary)
        {
            ++_index;
            if ((_dimensionMask & PolytopeIntersector::DimOne) == 0) return;
            
            PlaneMask selector_mask = 0x1;
            PlaneMask inside_mask = 0x0;
            _candidates.clear();

            bool v1Inside = true;
            bool v2Inside = true;
            for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end(); ++it, selector_mask<<=1)
            {
                const osg::Plane& plane=*it;
                const value_type d1=plane.distance(v1);
                const value_type d2=plane.distance(v2);
                const bool d1IsNegative = (d1<0.0f);
                const bool d2IsNegative = (d2<0.0f);
                if (d1IsNegative && d2IsNegative) return;      // line outside
                
                if (!d1IsNegative && !d2IsNegative)
                {
                    inside_mask |= selector_mask;
                    continue;   // completly inside
                }
                if (d1IsNegative) v1Inside = false;
                if (d2IsNegative) v2Inside = false;
                if (d1==0.0f)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, v1) );
                }
                else if (d2==0.0f)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, v2) );
                }
                else if (d1IsNegative && !d2IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v1-(v2-v1)*(d1/(-d1+d2))) ) );
                } else if (!d1IsNegative && d2IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v1+(v2-v1)*(d1/(d1-d2))) ) );
                }

                }
                if (inside_mask==_plane_mask)
                {
                    _candidates.push_back( CandList_t::value_type(_plane_mask, v1) );
                    _candidates.push_back( CandList_t::value_type(_plane_mask, v2) );
                    addIntersection(_index, _candidates);
                    return;
                }

            unsigned int numCands=checkCandidatePoints(inside_mask);
            if (numCands>0)
            {
                if (v1Inside) _candidates.push_back( CandList_t::value_type(_plane_mask, v1) );
                if (v2Inside) _candidates.push_back( CandList_t::value_type(_plane_mask, v2) );
                addIntersection(_index, _candidates);
            }

        }

        // handle triangles
        void operator()(const Vec3_type v1, const Vec3_type v2, const Vec3_type v3, bool treatVertexDataAsTemporary)
        {
            ++_index;
            if ((_dimensionMask & PolytopeIntersector::DimTwo) == 0) return;

            PlaneMask selector_mask = 0x1;
            PlaneMask inside_mask = 0x0;
            _candidates.clear();

            for(PlaneList::const_iterator it=_planes.begin();
                it!=_planes.end();
                ++it, selector_mask <<= 1)
            {
                const osg::Plane& plane=*it;
                const value_type d1=plane.distance(v1);
                const value_type d2=plane.distance(v2);
                const value_type d3=plane.distance(v3);
                const bool d1IsNegative = (d1<0.0f);
                const bool d2IsNegative = (d2<0.0f);
                const bool d3IsNegative = (d3<0.0f);
                if (d1IsNegative && d2IsNegative && d3IsNegative) return;      // triangle outside
                if (!d1IsNegative && !d2IsNegative && !d3IsNegative)
                {
                    inside_mask |= selector_mask;
                    continue;   // completly inside
                }

                // edge v1-v2 intersects
                if (d1==0.0f)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, v1) );
                }
                else if (d2==0.0f)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, v2) );
                }
                else if (d1IsNegative && !d2IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v1-(v2-v1)*(d1/(-d1+d2))) ) );
                }
                else if (!d1IsNegative && d2IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v1+(v2-v1)*(d1/(d1-d2))) ) );
                }

                // edge v1-v3 intersects
                if (d3==0.0f)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, v3) );
                }
                else if (d1IsNegative && !d3IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v1-(v3-v1)*(d1/(-d1+d3))) ) );
                }
                else if (!d1IsNegative && d3IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v1+(v3-v1)*(d1/(d1-d3))) ) );
                }

                // edge v2-v3 intersects
                if (d2IsNegative && !d3IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v2-(v3-v2)*(d2/(-d2+d3))) ) );
                } else if (!d2IsNegative && d3IsNegative)
                {
                    _candidates.push_back( CandList_t::value_type(selector_mask, (v2+(v3-v2)*(d2/(d2-d3))) ) );
                }
            }

            if (_plane_mask==inside_mask)
            { // triangle lies inside of all planes
                _candidates.push_back( CandList_t::value_type(_plane_mask, v1) );
                _candidates.push_back( CandList_t::value_type(_plane_mask, v2) );
                _candidates.push_back( CandList_t::value_type(_plane_mask, v3) );
                addIntersection(_index, _candidates);
                return;
            }

            if (_candidates.empty() && _planes.size()<3) return;

            unsigned int numCands=checkCandidatePoints(inside_mask);

            if (numCands>0)
            {
                addIntersection(_index, _candidates);
                return;
            }

            // handle case where the polytope goes through the triangle
            // without containing any point of it

            LinesList& lines=getPolytopeLines();
            _candidates.clear();

            // check all polytope lines against the triangle
            // use algorithm from "Real-time rendering" (second edition) pp.580
            const Vec3_type e1=v2-v1;
            const Vec3_type e2=v3-v1;

            for (LinesList::const_iterator it=lines.begin(); it!=lines.end(); ++it)
            {
                const PlanesLine& line=*it;

                Vec3_type p=line.dir^e2;
                const value_type a=e1*p;
                if (osg::absolute(a)<eps()) continue;
                
                const value_type f=1.0f/a;
                const Vec3_type s=(line.pos-v1);
                const value_type u=f*(s*p);
                if (u<0.0f || u>1.0f) continue;
                
                const Vec3_type q=s^e1;
                const value_type v=f*(line.dir*q);
                if (v<0.0f || u+v>1.0f) continue;
                
                const value_type t=f*(e2*q);

                _candidates.push_back(CandList_t::value_type(line.mask, line.pos+line.dir*t));
            }

            numCands=checkCandidatePoints(inside_mask);

            if (numCands>0)
            {
                addIntersection(_index, _candidates);
                return;
            }

        }

        /// handle quads
        void operator()(const Vec3_type v1, const Vec3_type v2, const Vec3_type v3, const Vec3_type v4, bool treatVertexDataAsTemporary)
        {
            if ((_dimensionMask & PolytopeIntersector::DimTwo) == 0)
            {
                ++_index;
                return;
            }
            this->operator()(v1,v2,v3,treatVertexDataAsTemporary);
            
            --_index;
            
            this->operator()(v1,v3,v4,treatVertexDataAsTemporary);
        }

        void setDimensionMask(unsigned int dimensionMask) { _dimensionMask = dimensionMask; }

        void setPolytope(osg::Polytope& polytope, osg::Plane& referencePlane)
        {
            _referencePlane = referencePlane;

            const PlaneMask currentMask = polytope.getCurrentMask();
            PlaneMask selector_mask = 0x1;

            const PlaneList& planeList = polytope.getPlaneList();
            unsigned int numActivePlanes = 0;

            PlaneList::const_iterator itr;
            for(itr=planeList.begin(); itr!=planeList.end(); ++itr)
            {
                if (currentMask&selector_mask) ++numActivePlanes;
                selector_mask <<= 1;
            }

            _plane_mask = 0x0;
            _planes.clear();
            _planes.reserve(numActivePlanes);
            _lines.clear();

            selector_mask=0x1;
            for(itr=planeList.begin(); itr!=planeList.end(); ++itr)
            {
                if (currentMask&selector_mask)
                {
                    _planes.push_back(*itr);
                    _plane_mask <<= 1;
                    _plane_mask |= 0x1;
                }
                selector_mask <<= 1;
            }
        }


        /// get boundary lines of polytope
        LinesList& getPolytopeLines()
        {
            if (!_lines.empty()) return _lines;

            PlaneMask selector_mask = 0x1;
            for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end();
             ++it, selector_mask <<= 1 ) {
            const osg::Plane& plane1=*it;
            const Vec3_type normal1=plane1.getNormal();
            const Vec3_type point1=normal1*(-plane1[3]);   /// canonical point on plane1
            PlaneMask sub_selector_mask = (selector_mask<<1);
            for (PlaneList::const_iterator jt=it+1; jt!=_planes.end(); ++jt, sub_selector_mask <<= 1 ) {
                const osg::Plane& plane2=*jt;
                const Vec3_type normal2=plane2.getNormal();
                if (osg::absolute(normal1*normal2) > (1.0-eps())) continue;
                const Vec3_type lineDirection = normal1^normal2;

                const Vec3_type searchDirection = lineDirection^normal1; /// search dir in plane1
                const value_type seachDist = -plane2.distance(point1)/(searchDirection*normal2);
                if (osg::isNaN(seachDist)) continue;
                const Vec3_type linePoint=point1+searchDirection*seachDist;
                _lines.push_back(PlanesLine(selector_mask|sub_selector_mask, linePoint, lineDirection));
            }
            }
            return _lines;
        }

        unsigned int getNumPlanes() const { return _planes.size(); }

        Intersections intersections;
        osg::Plane _referencePlane;

        unsigned int _index;

        private:
        unsigned int _dimensionMask;
        PlaneList _planes;                   ///< active planes extracted from polytope
        LinesList _lines;                    ///< all intersection lines of two polytope planes
        PlaneMask _plane_mask;               ///< mask for all planes of the polytope
        CandList_t _candidates;
    }; // class PolytopePrimitiveIntersector

} // namespace PolytopeIntersectorUtils


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PolytopeIntersector
//
PolytopeIntersector::PolytopeIntersector(const osg::Polytope& polytope):
    _parent(0),
    _polytope(polytope),
    _dimensionMask( AllDims )
{
    if (!_polytope.getPlaneList().empty())
    {
        _referencePlane = _polytope.getPlaneList().back();
    }
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, const osg::Polytope& polytope):
    Intersector(cf),
    _parent(0),
    _polytope(polytope),
    _dimensionMask( AllDims )
{
    if (!_polytope.getPlaneList().empty())
    {
        _referencePlane = _polytope.getPlaneList().back();
    }
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, double xMin, double yMin, double xMax, double yMax):
    Intersector(cf),
    _parent(0),
    _dimensionMask( AllDims )
{
    double zNear = 0.0;
    switch(cf)
    {
        case WINDOW : zNear = 0.0; break;
        case PROJECTION : zNear = 1.0; break;
        case VIEW : zNear = 0.0; break;
        case MODEL : zNear = 0.0; break;
    }

    _polytope.add(osg::Plane(1.0, 0.0, 0.0, -xMin));
    _polytope.add(osg::Plane(-1.0,0.0 ,0.0, xMax));
    _polytope.add(osg::Plane(0.0, 1.0, 0.0,-yMin));
    _polytope.add(osg::Plane(0.0,-1.0,0.0, yMax));
    _polytope.add(osg::Plane(0.0,0.0,1.0, zNear));

    _referencePlane = _polytope.getPlaneList().back();
}

Intersector* PolytopeIntersector::clone(osgUtil::IntersectionVisitor& iv)
{
    if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
    {
        osg::ref_ptr<PolytopeIntersector> pi = new PolytopeIntersector(_polytope);
        pi->_parent = this;
        pi->_dimensionMask = this->_dimensionMask;
        pi->_referencePlane = this->_referencePlane;
        return pi.release();
    }

    // compute the matrix that takes this Intersector from its CoordinateFrame into the local MODEL coordinate frame
    // that geometry in the scene graph will always be in.
    osg::Matrix matrix;
    switch (_coordinateFrame)
    {
        case(WINDOW):
            if (iv.getWindowMatrix()) matrix.preMult( *iv.getWindowMatrix() );
            if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case(PROJECTION):
            if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case(VIEW):
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case(MODEL):
            if (iv.getModelMatrix()) matrix = *iv.getModelMatrix();
            break;
    }

    osg::Polytope transformedPolytope;
    transformedPolytope.setAndTransformProvidingInverse(_polytope, matrix);

    osg::ref_ptr<PolytopeIntersector> pi = new PolytopeIntersector(transformedPolytope);
    pi->_parent = this;
    pi->_dimensionMask = this->_dimensionMask;
    pi->_referencePlane = this->_referencePlane;
    pi->_referencePlane.transformProvidingInverse(matrix);
    return pi.release();
}

bool PolytopeIntersector::enter(const osg::Node& node)
{
    return !node.isCullingActive() || _polytope.contains( node.getBound() );
}


void PolytopeIntersector::leave()
{
    // do nothing.
}


void PolytopeIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
    if ( !_polytope.contains( drawable->getBound() ) ) return;

    osg::TemplatePrimitiveFunctor<PolytopeIntersectorUtils::PolytopePrimitiveIntersector> func;
    func.setPolytope( _polytope, _referencePlane );
    func.setDimensionMask( _dimensionMask );

    drawable->accept(func);

    if (func.intersections.empty()) return;


    for(PolytopeIntersectorUtils::Intersections::const_iterator it=func.intersections.begin();
        it!=func.intersections.end();
        ++it)
    {
        const PolytopeIntersectorUtils::PolytopeIntersection& intersection = *it;

        Intersection hit;
        hit.distance = intersection._distance;
        hit.maxDistance = intersection._maxDistance;
        hit.primitiveIndex = intersection._index;
        hit.nodePath = iv.getNodePath();
        hit.drawable = drawable;
        hit.matrix = iv.getModelMatrix();

        osg::Vec3 center;
        for (unsigned int i=0; i<intersection._numPoints; ++i)
        {
            center += intersection._points[i];
        }
        center /= float(intersection._numPoints);
        hit.localIntersectionPoint = center;

        hit.numIntersectionPoints = intersection._numPoints;
        std::copy(&intersection._points[0], &intersection._points[intersection._numPoints],
              &hit.intersectionPoints[0]);

        insertIntersection(hit);
    }
}


void PolytopeIntersector::reset()
{
    Intersector::reset();

    _intersections.clear();
}

