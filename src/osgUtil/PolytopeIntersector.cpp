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



class PolytopePrimitiveIntersector {
public:
#ifdef OSG_USE_DOUBLE_PLANE
    typedef double value_type;
    typedef osg::Vec3d Vec3_type;
#else
    typedef float value_type;
    typedef osg::Vec3f Vec3_type;
#endif

    typedef osg::Polytope::ClippingMask PlaneMask;
    typedef osg::Polytope::PlaneList PlaneList;
    typedef std::vector<std::pair<PlaneMask,Vec3_type> > CandList_t;

    /// a line defined by the intersection of two planes
    struct PlanesLine {
        PlanesLine(PlaneMask m, Vec3_type p, Vec3_type d) :
            mask(m), pos(p), dir(d) {}
        PlaneMask mask;
        Vec3_type pos;
        Vec3_type dir;
    };
    typedef std::vector<PlanesLine> LinesList;

    PolytopePrimitiveIntersector() :
        numPoints(0),
        numLines(0),
        numTriangles(0),
        numQuads(0),
        _numIntersections(0) {}

    value_type eps() { return 1e-6; }

    /// check which candidate points lie within the polytope volume
    /// mark outliers with mask == 0, return number of remaining candidates
    unsigned int checkCandidatePoints(PlaneMask inside_mask, CandList_t& cand) {
        PlaneMask selector_mask = 0x1;
        unsigned int numCands=cand.size();
        for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end() && numCands>0;
             ++it, selector_mask <<= 1) {
            const osg::Plane& plane=*it;
            if (selector_mask & inside_mask) continue;
            for (CandList_t::iterator pointIt=cand.begin(); pointIt!=cand.end(); ++pointIt) {
                PlaneMask mask=pointIt->first;
                if (mask==0) continue;
                if (selector_mask & mask) continue;
                if (plane.distance(pointIt->second)<0.0f) {
                    mask=0;  // mark as outside
                    --numCands;
                    if (numCands==0) return 0;
                }
            }
        }
        return numCands;
    }

    // handle points
    void operator()(const Vec3_type v1, bool treatVertexDataAsTemporary) {
        ++numPoints;
        for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end(); ++it) {
            const osg::Plane& plane=*it;
            const value_type d1=plane.distance(v1);
            if (d1<0.0f) return;   // point outside
        }
        ++_numIntersections;
    }

    // handle lines
    void operator()(const Vec3_type v1, const Vec3_type v2, bool treatVertexDataAsTemporary)
    {
        ++numLines;
        PlaneMask selector_mask = 0x1;
        PlaneMask inside_mask = 0x0;
        CandList_t cand;

        for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end(); ++it, selector_mask<<=1) {
            const osg::Plane& plane=*it;
            const value_type d1=plane.distance(v1);
            const value_type d2=plane.distance(v2);
            const bool d1IsNegative = (d1<0.0f);
            const bool d2IsNegative = (d2<0.0f);
            if (d1IsNegative && d2IsNegative) return;      // line outside
            if (!d1IsNegative && !d2IsNegative) {
                inside_mask |= selector_mask;
                continue;   // completly inside
            }
            if (d1==0.0f) {
                cand.push_back( CandList_t::value_type(selector_mask, v1) );
            } else if (d2==0.0f) {
                cand.push_back( CandList_t::value_type(selector_mask, v2) );
            } else if (d1IsNegative && !d2IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v1-(v2-v1)*(d1/(-d1+d2))) ) );
            } else if (!d1IsNegative && d2IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v1+(v2-v1)*(d1/(d1-d2))) ) );
            }

        }
        if (inside_mask==_plane_mask) {
            ++_numIntersections;
            return;
        }

        unsigned int numCands=checkCandidatePoints(inside_mask, cand);
        if (numCands>0) {
            ++_numIntersections;
        }

    }

    // handle triangles
    void operator()(const Vec3_type v1, const Vec3_type v2, const Vec3_type v3,
                    bool treatVertexDataAsTemporary)
    {
        ++numTriangles;
        PlaneMask selector_mask = 0x1;
        PlaneMask inside_mask = 0x0;
        CandList_t cand;
        for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end();
             ++it, selector_mask <<= 1) {
            const osg::Plane& plane=*it;
            const value_type d1=plane.distance(v1);
            const value_type d2=plane.distance(v2);
            const value_type d3=plane.distance(v3);
            const bool d1IsNegative = (d1<0.0f);
            const bool d2IsNegative = (d2<0.0f);
            const bool d3IsNegative = (d3<0.0f);
            if (d1IsNegative && d2IsNegative && d3IsNegative) return;      // triangle outside
            if (!d1IsNegative && !d2IsNegative && !d3IsNegative) {
                inside_mask |= selector_mask;
                continue;   // completly inside
            }

            // edge v1-v2 intersects
            if (d1==0.0f) {
                cand.push_back( CandList_t::value_type(selector_mask, v1) );
            } else if (d2==0.0f) {
                cand.push_back( CandList_t::value_type(selector_mask, v2) );
            } else if (d1IsNegative && !d2IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v1-(v2-v1)*(d1/(-d1+d2))) ) );
            } else if (!d1IsNegative && d2IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v1+(v2-v1)*(d1/(d1-d2))) ) );
            }

            // edge v1-v3 intersects
            if (d3==0.0f) {
                cand.push_back( CandList_t::value_type(selector_mask, v3) );
            } else if (d1IsNegative && !d3IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v1-(v3-v1)*(d1/(-d1+d3))) ) );
            } else if (!d1IsNegative && d3IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v1+(v3-v1)*(d1/(d1-d3))) ) );
            }

            // edge v2-v3 intersects
            if (d2IsNegative && !d3IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v2-(v3-v2)*(d2/(-d2+d3))) ) );
            } else if (!d2IsNegative && d3IsNegative) {
                cand.push_back( CandList_t::value_type(selector_mask, (v2+(v3-v2)*(d2/(d2-d3))) ) );
            }
        }

        if (_plane_mask==inside_mask) { // triangle lies inside of all planes
            ++_numIntersections;
            return;
        }

        if (cand.empty() && _planes.size()<3) {
            return;
        }

        unsigned int numCands=checkCandidatePoints(inside_mask, cand);

        if (numCands>0) {
            ++_numIntersections;
            return;
        }

        // handle case where the polytope goes through the triangle
        // without containing any point of it

        LinesList& lines=getPolytopeLines();
        cand.clear();

        // check all polytope lines against the triangle
        // use algorithm from "Real-time rendering" (second edition) pp.580
        const Vec3_type e1=v2-v1;
        const Vec3_type e2=v3-v1;

        for (LinesList::const_iterator it=lines.begin(); it!=lines.end(); ++it) {
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

            cand.push_back(CandList_t::value_type(line.mask, line.pos+line.dir*t));
        }

        numCands=checkCandidatePoints(inside_mask, cand);

        if (numCands>0) {
            ++_numIntersections;
            return;
        }

    }

    /// handle quads
    void operator()(const Vec3_type v1, const Vec3_type v2, const Vec3_type v3, const Vec3_type v4,
                    bool treatVertexDataAsTemporary) {
        ++numQuads;
        this->operator()(v1,v2,v3,treatVertexDataAsTemporary);
        this->operator()(v1,v3,v4,treatVertexDataAsTemporary);
        numTriangles-=2;
    }

    void setPolytope(osg::Polytope& polytope) {

        const PlaneMask currentMask = polytope.getCurrentMask();
        PlaneMask selector_mask = 0x1;

        const PlaneList& planeList = polytope.getPlaneList();
        unsigned int numActivePlanes = 0;

        PlaneList::const_iterator itr;
        for(itr=planeList.begin(); itr!=planeList.end(); ++itr) {
            if (currentMask&selector_mask) ++numActivePlanes;
            selector_mask <<= 1;
        }

        _plane_mask = 0x0;
        _planes.clear();
        _planes.reserve(numActivePlanes);
        _lines.clear();

        selector_mask=0x1;
        for(itr=planeList.begin(); itr!=planeList.end(); ++itr) {
            if (currentMask&selector_mask) {
                _planes.push_back(*itr);
                _plane_mask <<= 1;
                _plane_mask |= 0x1;
            }
            selector_mask <<= 1;
        }
    }

    /// get boundary lines of polytope
    LinesList& getPolytopeLines() {
        if (!_lines.empty()) return _lines;

        PlaneMask selector_mask = 0x1;
        for (PlaneList::const_iterator it=_planes.begin(); it!=_planes.end(); ++it, selector_mask <<= 1 ) {
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

    unsigned int getNumIntersections() const { return _numIntersections; }
    unsigned int getNumPrimitives() const { return numPoints+numLines+numTriangles; }
    unsigned int getNumPlanes() const { return _planes.size(); }

    unsigned int numPoints;
    unsigned int numLines;
    unsigned int numTriangles;
    unsigned int numQuads;

private:
    PlaneList _planes;                   ///< active planes extracted from polytope
    LinesList _lines;                    ///< all intersection lines of two polytope planes
    PlaneMask _plane_mask;               ///< mask for all planes of the polytope
    unsigned int _numIntersections;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PolytopeIntersector
//
PolytopeIntersector::PolytopeIntersector(const osg::Polytope& polytope):
    _parent(0),
    _polytope(polytope)
{
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, const osg::Polytope& polytope):
    Intersector(cf),
    _parent(0),
    _polytope(polytope)
{
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, double xMin, double yMin, double xMax, double yMax):
    Intersector(cf),
    _parent(0)
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
}

Intersector* PolytopeIntersector::clone(osgUtil::IntersectionVisitor& iv)
{
    if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
    {
        osg::ref_ptr<PolytopeIntersector> pi = new PolytopeIntersector(_polytope);
        pi->_parent = this;
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

    osg::TemplatePrimitiveFunctor<PolytopePrimitiveIntersector> func;
    func.setPolytope(_polytope);

    drawable->accept(func);

    if (func.getNumIntersections()==0) {
       return;
    }

    osg::notify(osg::INFO) << func.getNumIntersections() << " intersections with "
               << func.numPoints<<" points, "<< func.numLines
               <<" lines, "<< func.numTriangles
               <<" triangles, "<<func.numQuads<<" quads"<<std::endl;

    Intersection hit;
    hit.nodePath = iv.getNodePath();
    hit.drawable = drawable;

    insertIntersection(hit);
}


void PolytopeIntersector::reset()
{
    Intersector::reset();

    _intersections.clear();
}
