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

#include <osgUtil/DelaunayTriangulator>
// NB this algorithm makes heavy use of the osgUtil::Tesselator for constrained triangulation.
// truly it is built on the shoulders of giants.

#include <osg/GL>
#include <osg/Vec3>
#include <osg/Array>
#include <osg/Notify>

#include <algorithm>
#include <set>
#include <map> //GWM July 2005 map is used in constraints.
#include <osgUtil/Tesselator> // tesselator triangulates the constrained triangles

namespace osgUtil
{

//////////////////////////////////////////////////////////////////////////////////////
// MISC MATH FUNCTIONS


// Compute the circumcircle of a triangle (only x and y coordinates are used),
// return (Cx, Cy, r^2)
inline osg::Vec3 compute_circumcircle(
    const osg::Vec3 &a, 
    const osg::Vec3 &b, 
    const osg::Vec3 &c)
{
    float D = 
        (a.x() - c.x()) * (b.y() - c.y()) - 
        (b.x() - c.x()) * (a.y() - c.y());

    float cx, cy, r2;

    if(D==0.0)
    {
        // (Nearly) degenerate condition - either two of the points are equal (which we discount)
        // or the three points are colinear. In this case we just determine the average of
        // the three points as the centre for correctness, but squirt out a zero radius.
        // This method will produce a triangulation with zero area, so we have to check later
        cx = (a.x()+b.x()+c.x())/3.0;
        cy = (a.y()+b.y()+c.y())/3.0;
        r2 = 0.0;
    }
    else
    {
        cx = 
        (((a.x() - c.x()) * (a.x() + c.x()) + 
        (a.y() - c.y()) * (a.y() + c.y())) / 2 * (b.y() - c.y()) -
        ((b.x() - c.x()) * (b.x() + c.x()) +
        (b.y() - c.y()) * (b.y() + c.y())) / 2 * (a.y() - c.y())) / D;

        cy = 
        (((b.x() - c.x()) * (b.x() + c.x()) + 
        (b.y() - c.y()) * (b.y() + c.y())) / 2 * (a.x() - c.x()) -
        ((a.x() - c.x()) * (a.x() + c.x()) +
        (a.y() - c.y()) * (a.y() + c.y())) / 2 * (b.x() - c.x())) / D;

      //  r2 = (c.x() - cx) * (c.x() - cx) + (c.y() - cy) * (c.y() - cy);
        // the return r square is compared with r*r many times in an inner loop
        // so for efficiency use the inefficient sqrt once rather than 30* multiplies later.
        r2 = sqrt((c.x() - cx) * (c.x() - cx) + (c.y() - cy) * (c.y() - cy));
    }
    return osg::Vec3(cx, cy, r2);
}

// Test whether a point (only the x and y coordinates are used) lies inside
// a circle; the circle is passed as a vector: (Cx, Cy, r).

inline bool point_in_circle(const osg::Vec3 &point, const osg::Vec3 &circle)
{
    float r2 = 
        (point.x() - circle.x()) * (point.x() - circle.x()) + 
        (point.y() - circle.y()) * (point.y() - circle.y());
    return r2 <= circle.z()*circle.z();
//    return r2 <= circle.z();
}


//
//////////////////////////////////////////////////////////////////////////////////////


// data type for vertex indices
typedef GLuint Vertex_index;


// CLASS: Edge
// This class describes an edge of a triangle (it stores vertex indices to the two
// endpoints).

class Edge {
public:

    // Comparison object (for sorting)
    struct Less
    {
        inline bool operator()(const Edge &e1, const Edge &e2) const
        {
            if (e1.ibs() < e2.ibs()) return true;
            if (e1.ibs() > e2.ibs()) return false;
            if (e1.ies() < e2.ies()) return true;
            return false;
        }
    };

    Edge() {}
    Edge(Vertex_index ib, Vertex_index ie) : ib_(ib), ie_(ie), ibs_(osg::minimum(ib, ie)), ies_(osg::maximum(ib, ie)), duplicate_(false) {}

    // first endpoint
    inline Vertex_index ib() const { return ib_; }

    // second endpoint
    inline Vertex_index ie() const { return ie_; }

    // first sorted endpoint
    inline Vertex_index ibs() const { return ibs_; }

    // second sorted endpoint
    inline Vertex_index ies() const { return ies_; }

    // get the "duplicate" flag
    inline bool get_duplicate() const { return duplicate_; }

    // set the "duplicate" flag
    inline void set_duplicate(bool v) { duplicate_ = v; }

private:
    Vertex_index ib_, ie_;
    Vertex_index ibs_, ies_;
    bool duplicate_;
};


// CLASS: Triangle

class Triangle
{
public:
    Triangle(Vertex_index a, Vertex_index b, Vertex_index c, osg::Vec3Array *points)
        :    a_(a), 
            b_(b), 
            c_(c), 
            cc_(compute_circumcircle((*points)[a_], (*points)[b_], (*points)[c_]))
    {
        edge_[0] = Edge(a_, b_);
        edge_[1] = Edge(b_, c_);
        edge_[2] = Edge(c_, a_);
    }

    inline Vertex_index a() const { return a_; }
    inline Vertex_index b() const { return b_; }
    inline Vertex_index c() const { return c_; }

    inline const Edge &get_edge(int idx) const { return edge_[idx];    }
    inline const osg::Vec3 &get_circumcircle() const { return cc_; }

    inline osg::Vec3 compute_centroid(const osg::Vec3Array *points) const
    {
        return ((*points)[a_] +(*points)[b_] + (*points)[c_])/3;
    }

    inline osg::Vec3 compute_normal(osg::Vec3Array *points) const
    {
        osg::Vec3 N = ((*points)[b_] - (*points)[a_]) ^ ((*points)[c_] - (*points)[a_]);
        return N / N.length();
    }

    bool isedge(const unsigned int ip1,const unsigned int ip2) const
    { // is one of the edges of this triangle from ip1-ip2
        bool isedge=ip1==a() && ip2==b();
        if (!isedge)
        {
            isedge=ip1==b() && ip2==c();
            if (!isedge)
            {
                isedge=ip1==c() && ip2==a();
            }
        }
        return isedge;
    }
    // GWM July 2005 add test for triangle intersected by p1-p2.
    // return true for unused edge

    const bool intersected(const unsigned int ip1,const unsigned int ip2,const osg::Vec2 p1 ,const osg::Vec2 p2,const int iedge, osg::Vec3Array *points) const
    {
        // return true if edge iedge of triangle is intersected by ip1,ip2
        Vertex_index ie1,ie2;
        if (iedge==0)
        {
            ie1=a();
            ie2=b();
        }
        else if (iedge==1)
        {
            ie1=b();
            ie2=c();
        }
        else if (iedge==2)
        {
            ie1=c();
            ie2=a();
        } 
        if (ip1==ie1 || ip2==ie1) return false;
        if (ip1==ie2 || ip2==ie2) return false;
    
        osg::Vec2 tp1((*points)[ie1].x(),(*points)[ie1].y());
        osg::Vec2 tp2((*points)[ie2].x(),(*points)[ie2].y());
        return intersect(tp1,tp2,p1,p2);
    }
    
    bool intersectedby(const osg::Vec2 p1,const osg::Vec2 p2,osg::Vec3Array *points) const {
        // true if line [p1,p2] cuts at least one edge of this triangle
        osg::Vec2 tp1((*points)[a()].x(),(*points)[a()].y());
        osg::Vec2 tp2((*points)[b()].x(),(*points)[b()].y());
        osg::Vec2 tp3((*points)[c()].x(),(*points)[c()].y());
        bool ip=intersect(tp1,tp2,p1,p2);
        if (!ip)
        {
            ip=intersect(tp2,tp3,p1,p2);
            if (!ip)
            {
                ip=intersect(tp3,tp1,p1,p2);
            }
        }
        return ip;
    }
    int whichEdge(osg::Vec3Array *points,const osg::Vec2 p1, const osg::Vec2 p2,
        const unsigned int e1,const unsigned int e2) const
    {
        int icut=0;
        // find which edge of triangle is cut by line (p1-p2) and is NOT e1-e2 indices.
        // return 1 - cut is on edge b-c; 2==c-a
        osg::Vec2 tp1((*points)[a()].x(),(*points)[a()].y()); // triangle vertices
        osg::Vec2 tp2((*points)[b()].x(),(*points)[b()].y());
        osg::Vec2 tp3((*points)[c()].x(),(*points)[c()].y());
        bool ip=intersect(tp2,tp3,p1,p2);
        if (ip && (a()==e1 || a()==e2)) { return 1;}
        ip=intersect(tp3,tp1,p1,p2);
        if (ip && (b()==e1 || b()==e2)) { return 2;}
        ip=intersect(tp1,tp2,p1,p2);
        if (ip && (c()==e1 || c()==e2)) { return 3;}
        return icut;
    }

    bool usesVertex(const unsigned int ip) const
    {
        return ip==a_ || ip==b_ || ip==c_;
    }

    int lineBisectTest(const osg::Vec3 apt,const osg::Vec3 bpt,const osg::Vec3 cpt, const osg::Vec2 p2) const
    { 
        osg::Vec2 vp2tp=p2-osg::Vec2(apt.x(), apt.y()); // vector from p1 to a.
        // test is: cross product (z component) with ab,ac is opposite signs
        // AND dot product with ab,ac has at least one positive value.
        osg::Vec2 vecba=osg::Vec2(bpt.x(), bpt.y())-osg::Vec2(apt.x(), apt.y());
        osg::Vec2 vecca=osg::Vec2(cpt.x(), cpt.y())-osg::Vec2(apt.x(), apt.y());
        float cprodzba=vp2tp.x()*vecba.y() - vp2tp.y()*vecba.x();
        float cprodzca=vp2tp.x()*vecca.y() - vp2tp.y()*vecca.x();
    //    osg::notify(osg::WARN) << "linebisect test " << " tri " << a_<<","<< b_<<","<< c_<<std::endl;
        if (cprodzba*cprodzca<0)
        {
            // more tests - check dot products are at least partly parallel to test line.
            osg::Vec2 tp1(bpt.x(),bpt.y()); // triangle vertices
            osg::Vec2 tp2(cpt.x(),cpt.y());
            osg::Vec2 tp3(apt.x(),apt.y());
            bool ip=intersect(tp1,tp2,tp3,p2);
            if (ip) return 1;
        }
        return 0;
    }
    
    int lineBisects(osg::Vec3Array *points, const unsigned int ip1, const osg::Vec2 p2) const
    { 
        // return true if line starts at vertex <ip1> and lies between the 2 edges which meet at vertex
        // <vertex> is that which uses index ip1.
        // line is <vertex> to p2
        //    return value is 0 - no crossing; 1,2,3 - which edge of the triangle is cut.
        if (a_==ip1)
        {
            // first vertex is the vertex - test that a_ to p2 lies beteen edges a,b and a,c
            osg::Vec3 apt=(*points)[a_];
            osg::Vec3 bpt=(*points)[b_];
            osg::Vec3 cpt=(*points)[c_];
            return lineBisectTest(apt,bpt,cpt,p2)?1:0;
        }
        else if (b_==ip1)
        {
            // second vertex is the vertex - test that b_ to p2 lies beteen edges a,b and a,c
            osg::Vec3 apt=(*points)[b_];
            osg::Vec3 bpt=(*points)[c_];
            osg::Vec3 cpt=(*points)[a_];
            return lineBisectTest(apt,bpt,cpt,p2)?2:0;
        }
        else if (c_==ip1)
        {
            // 3rd vertex is the vertex - test that c_ to p2 lies beteen edges a,b and a,c
            osg::Vec3 apt=(*points)[c_];
            osg::Vec3 bpt=(*points)[a_];
            osg::Vec3 cpt=(*points)[b_];
            return lineBisectTest(apt,bpt,cpt,p2)?3:0;
        }
        return 0;
    }
    
private:

    bool intersect(const osg::Vec2 p1,const osg::Vec2 p2,const osg::Vec2 p3,const osg::Vec2 p4) const
    {
        // intersection point of p1,p2 and p3,p4
        // test from http://astronomy.swin.edu.au/~pbourke/geometry/lineline2d/
        // the intersection must be internal to the lines, not an end point.
        float det=((p4.y()-p3.y())*(p2.x()-p1.x())-(p4.x()-p3.x())*(p2.y()-p1.y()));
        if (det!=0)
        {
            // point on line is P=p1+ua.(p2-p1) and Q=p3+ub.(p4-p3) 
            float ua=((p4.x()-p3.x())*(p1.y()-p3.y())-(p4.y()-p3.y())*(p1.x()-p3.x()))/det;
            float ub=((p2.x()-p1.x())*(p1.y()-p3.y())-(p2.y()-p1.y())*(p1.x()-p3.x()))/det;
            if (ua> 0.00 && ua< 1 && ub> 0.0000  && ub< 1)
            {
                return true;
            }
        }
        return false;
    }
    
    Vertex_index a_;
    Vertex_index b_;
    Vertex_index c_;
    osg::Vec3 cc_;    
    Edge edge_[3];
};

typedef std::list<Triangle> Triangle_list;

// comparison function for sorting sample points by the X coordinate
bool Sample_point_compare(const osg::Vec3 &p1, const osg::Vec3 &p2)
{
    // replace pure sort by X coordinate with X then Y.
    // errors can occur if the delaunay triangulation specifies 2 points at same XY and different Z
    if (p1.x() != p2.x()) return p1.x() < p2.x();
    if (p1.y() != p2.y()) return p1.y() < p2.y(); // GWM 30.06.05 - further rule if x coords are same.
    osg::notify(osg::INFO) << "Two points are coincident at "<<p1.x() <<","<<p1.y() << std::endl;
    return p1.z() < p2.z(); // never get here unless 2 points coincide
}


// container types
typedef std::set<Edge, Edge::Less> Edge_set;


DelaunayTriangulator::DelaunayTriangulator():
    osg::Referenced()
{
}

DelaunayTriangulator::DelaunayTriangulator(osg::Vec3Array *points, osg::Vec3Array *normals):
    osg::Referenced(),
    points_(points),
    normals_(normals)
{
}

DelaunayTriangulator::DelaunayTriangulator(const DelaunayTriangulator &copy, const osg::CopyOp &copyop):
    osg::Referenced(copy),
    points_(static_cast<osg::Vec3Array *>(copyop(copy.points_.get()))),
    normals_(static_cast<osg::Vec3Array *>(copyop(copy.normals_.get()))),
    prim_tris_(static_cast<osg::DrawElementsUInt *>(copyop(copy.prim_tris_.get())))
{
}

DelaunayTriangulator::~DelaunayTriangulator()
{
}

const Triangle * getTriangleWithEdge(const unsigned int ip1,const unsigned int ip2, const Triangle_list *triangles)
{ // find triangle in list with edge from ip1 to ip2...
    Triangle_list::const_iterator trconnitr; // connecting triangle
    int idx=0;
    for (trconnitr=triangles->begin(); trconnitr!=triangles->end(); )
    {
        if (trconnitr->isedge (ip1,ip2))
        {
            // this is the triangle.
            return &(*trconnitr);
        }
        ++trconnitr;
        idx++;
    }
    return NULL; //-1;
}

int DelaunayTriangulator::getindex(const osg::Vec3 pt,const osg::Vec3Array *points)
{
    // return index of pt in points (or -1)
    for (unsigned int i=0; i<points->size(); i++)
    {
        if (pt.x()==(*points)[i].x() &&pt.y()==(*points)[i].y() )
        {
            return i;
        }
    }
    return -1;
}

Triangle_list fillHole(osg::Vec3Array *points,    std::vector<unsigned int> vindexlist)
{
    // eg clockwise vertex neighbours around the hole made by the constraint
    Triangle_list triangles; // returned list
    osg::ref_ptr<osg::Geometry> gtess=new osg::Geometry; // add all the contours to this for analysis
    osg::ref_ptr<osg::Vec3Array> constraintverts=new osg::Vec3Array;
    osg::ref_ptr<osgUtil::Tesselator> tscx=new osgUtil::Tesselator; // this assembles all the constraints
    
    for (std::vector<unsigned int>::iterator itint=vindexlist.begin(); itint!=vindexlist.end(); itint++)
    {
    //    osg::notify(osg::WARN)<< "add point " << (*itint) << " at " << (*points)[*itint].x() << ","<< (*points)[*itint].y() <<std::endl;
        constraintverts->push_back((*points)[*itint]);
    }
    
    unsigned int npts=vindexlist.size();

    gtess->setVertexArray(constraintverts.get());
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,npts));
    tscx->setTesselationNormal(osg::Vec3(0.0,0.0,1.0));
    tscx->setTesselationType(osgUtil::Tesselator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ODD); // the commonest tesselation is default, ODD. GE2 allows intersections of constraints to be found.
    tscx->retesselatePolygons(*(gtess.get())); // this should insert extra vertices where constraints overlap

    // extract triangles from gtess
    
    unsigned int ipr;
    for (ipr=0; ipr<gtess->getNumPrimitiveSets(); ipr++)
    {
        unsigned int ic;
        osg::PrimitiveSet* prset=gtess->getPrimitiveSet(ipr);
        //                    osg::notify(osg::WARN)<< "gtess set " << ipr << " nprims " << prset->getNumPrimitives() <<
        //                        " type " << prset->getMode() << std::endl;
        unsigned int pidx,pidx1,pidx2;
        switch (prset->getMode()) {
        case osg::PrimitiveSet::TRIANGLES:
            for (ic=0; ic<prset->getNumIndices()-2; ic+=3)
            {
                if (prset->index(ic)>=npts)
                {
                    // this is an added point.
                    points->push_back((*constraintverts)[prset->index(ic)]);
                    pidx=points->size();
                }
                else
                {
                    pidx=vindexlist[prset->index(ic)];
                }
                
                if (prset->index(ic+1)>=npts)
                {
                    // this is an added point.
                    points->push_back((*constraintverts)[prset->index(ic+1)]);
                    pidx1=points->size();
                }
                else
                {
                    pidx1=vindexlist[prset->index(ic+1)];
                }
                
                if (prset->index(ic+2)>=npts)
                {
                    // this is an added point.
                    points->push_back((*constraintverts)[prset->index(ic+2)]);
                    pidx2=points->size();
                }
                else
                {
                    pidx2=vindexlist[prset->index(ic+2)];
                }
                triangles.push_back(Triangle(pidx, pidx1, pidx2, points));
                //                    osg::notify(osg::WARN)<< "vert " << prset->index(ic) << " in array"<<std::endl;
            }
            break;
        case osg::PrimitiveSet::TRIANGLE_STRIP: // 123, 234, 345...

            for (ic=0; ic<prset->getNumIndices()-2; ic++)
            {
                if (prset->index(ic)>=npts)
                {
                    // this is an added point.
                    points->push_back((*constraintverts)[prset->index(ic)]);
                    pidx=points->size();
                } else {
                    pidx=vindexlist[prset->index(ic)];
                }
                if (prset->index(ic+1)>=npts)
                {
                    // this is an added point.
                    points->push_back((*constraintverts)[prset->index(ic+1)]);
                    pidx1=points->size();
                }
                else
                {
                    pidx1=vindexlist[prset->index(ic+1)];
                }
                
                if (prset->index(ic+2)>=npts)
                {
                    // this is an added point.
                    points->push_back((*constraintverts)[prset->index(ic+2)]);
                    pidx2=points->size();
                }
                else 
                {
                    pidx2=vindexlist[prset->index(ic+2)];
                }
                
                if (ic%2==0)
                {
                    triangles.push_back(Triangle(pidx, pidx1, pidx2, points));
                }
                else
                {
                    triangles.push_back(Triangle(pidx1, pidx, pidx2, points));
                }
                //                    osg::notify(osg::WARN)<< "vert " << prset->index(ic) << " in array"<<std::endl;
            }
            break;
            
        case osg::PrimitiveSet::TRIANGLE_FAN:
            {
                osg::Vec3 ptest=(*constraintverts)[prset->index(0)];
                if (prset->index(0)>=npts)
                {
                    // this is an added point.
                    points->push_back((*constraintverts)[prset->index(0)]);
                    pidx=points->size();
                }
                else
                {
                    pidx=vindexlist[prset->index(0)];
                }
                //        osg::notify(osg::WARN)<< "tfan has " << prset->getNumIndices() << " indices"<<std::endl;
                for (ic=1; ic<prset->getNumIndices()-1; ic++)
                {
                    if (prset->index(ic)>=npts)
                    {
                        // this is an added point.
                        points->push_back((*constraintverts)[prset->index(ic)]);
                        pidx1=points->size();
                    }
                    else
                    {
                        pidx1=vindexlist[prset->index(ic)];
                    }
                    
                    if (prset->index(ic+1)>=npts)
                    { // this is an added point.
                        points->push_back((*constraintverts)[prset->index(ic+1)]);
                        pidx2=points->size();
                    }
                    else
                    {
                        pidx2=vindexlist[prset->index(ic+1)];
                    }
                    triangles.push_back(Triangle(pidx, pidx1, pidx2, points));
                }
            }
            break;
        default:
            osg::notify(osg::WARN)<< "WARNING set " << ipr << " nprims " << prset->getNumPrimitives() <<
                " type " << prset->getMode() << " Type not triangle, tfan or strip"<< std::endl;
            break;
        }
    }
    return triangles;
}

void DelaunayConstraint::removeVerticesInside(const DelaunayConstraint *dco)
{    /** remove vertices from this which are internal to dco.
     * retains potins that are extremely close to edge of dco
      * defined as edge of dco subtends>acs(0.999999)
    */
    int nrem=0;
    osg::Vec3Array *vertices= dynamic_cast< osg::Vec3Array*>(getVertexArray()); 
    if (vertices)
    {
        for (osg::Vec3Array::iterator vitr=vertices->begin(); vitr!=vertices->end(); )
        {
            if (dco->contains(*vitr))
            {
                unsigned int idx=vitr-vertices->begin(); // index of vertex
                // remove vertex index from all the primitives
                for (unsigned int ipr=0; ipr<getNumPrimitiveSets(); ipr++)
                {
                    osg::PrimitiveSet* prset=getPrimitiveSet(ipr);
                    osg::DrawElementsUShort *dsup=dynamic_cast<osg::DrawElementsUShort *>(prset);
                    if (dsup) {
                        for (osg::DrawElementsUShort::iterator usitr=dsup->begin(); usitr!=dsup->end(); )
                        {
                            if ((*usitr)==idx)
                            { // remove entirely
                                usitr=dsup->erase(usitr);
                            }
                            else
                            {
                                if ((*usitr)>idx) (*usitr)--; // move indices down 1
                                usitr++; // next index
                            }
                        }
                    }
                    else
                    {
                        osg::notify(osg::WARN) << "Invalid prset " <<ipr<< " tp " << prset->getType() << " types PrimitiveType,DrawArraysPrimitiveType=1 etc" << std::endl;
                    }
                }
                vitr=vertices->erase(vitr);
                nrem++;

            }
            else
            {
                vitr++;
            }
        }
    }
}

void DelaunayConstraint::merge(DelaunayConstraint *dco)
{
    unsigned int ipr;
    osg::Vec3Array* vmerge=dynamic_cast<osg::Vec3Array*>(getVertexArray());
    if (!vmerge) vmerge=new osg::Vec3Array;
    setVertexArray(vmerge);
    for ( ipr=0; ipr<dco->getNumPrimitiveSets(); ipr++)
    {
        osg::PrimitiveSet* prset=dco->getPrimitiveSet(ipr);
        osg::DrawArrays *drarr=dynamic_cast<osg::DrawArrays *> (prset);
        if (drarr)
        {
            // need to add the offset of vmerge->size to each prset indices.
            unsigned int noff=vmerge->size();
            unsigned int n1=drarr->getFirst(); // usually 0
            unsigned int numv=drarr->getCount(); //
            addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,n1+noff,numv));
        }
    }
    osg::Vec3Array* varr=dynamic_cast<osg::Vec3Array*>(dco->getVertexArray());
    if (varr) vmerge->insert(vmerge->end(),varr->begin(),varr->end());
}

void DelaunayTriangulator::_uniqueifyPoints()
{
    std::sort( points_->begin(), points_->end() );

    osg::ref_ptr<osg::Vec3Array> temppts = new osg::Vec3Array;
    // This won't work... must write our own unique() that compares only the first
    // two terms of a Vec3 for equivalency
    //std::insert_iterator< osg::Vec3Array > ti( *(temppts.get()), temppts->begin() );
    //std::unique_copy( points_->begin(), points_->end(), ti );

    osg::Vec3Array::iterator p = points_->begin();
    osg::Vec3 v = *p;
    // Always push back the first point
    temppts->push_back( (v = *p));
    for( ; p != points_->end(); p++ )
    {
        if( v[0] == (*p)[0] && v[1] == (*p)[1] )
            continue;

        temppts->push_back( (v = *p));
    }

    points_->clear();
    std::insert_iterator< osg::Vec3Array > ci(*(points_.get()),points_->begin());
    std::copy( temppts->begin(), temppts->end(), ci );
}



bool DelaunayTriangulator::triangulate()
{
    // check validity of input array
    if (!points_.valid())
    {
        osg::notify(osg::WARN) << "Warning: DelaunayTriangulator::triangulate(): invalid sample point array" << std::endl;
        return false;
    }

    osg::Vec3Array *points = points_.get();

    if (points->size() < 1)
    {
        osg::notify(osg::WARN) << "Warning: DelaunayTriangulator::triangulate(): too few sample points" << std::endl;
        return false;
    }

    // Eliminate duplicate lat/lon points from input coordinates.
    _uniqueifyPoints();


    // initialize storage structures
    Triangle_list triangles;
    Triangle_list discarded_tris;    

    // GWM July 2005 add constraint vertices to terrain
    linelist::iterator linitr;
    for (linitr=constraint_lines.begin();linitr!=constraint_lines.end();linitr++)
    {
        DelaunayConstraint* dc=(*linitr).get();
        const osg::Vec3Array* vercon= dynamic_cast<const osg::Vec3Array*>(dc->getVertexArray());
        if (vercon)
        {
            int nadded=0;
            for (unsigned int icon=0;icon<vercon->size();icon++)
            {
                osg::Vec3 p1=(*vercon)[icon];
                int idx=getindex(p1,points_.get());
                if (idx<0)
                { // only unique vertices are permitted.
                    points_->push_back(p1); // add non-unique constraint points to triangulation
                    nadded++;
                }
                else
                {
                    osg::notify(osg::WARN) << "DelaunayTriangulator: ignore a duplicate point at "<< p1.x()<< " " << p1.y() << std::endl;;
                }
            }
        }
    //    osg::notify(osg::WARN)<< "constraint size "<<vercon->size()<<" " <<nadded<< std::endl;
    }
        // GWM July 2005 end

    // pre-sort sample points
    osg::notify(osg::INFO) << "DelaunayTriangulator: pre-sorting sample points\n";
    std::sort(points->begin(), points->end(), Sample_point_compare);    

    // set the last valid index for the point list
    GLuint last_valid_index = points->size() - 1;    

    // find the minimum and maximum x values in the point list
    float minx = (*points)[0].x();
    float maxx = (*points)[last_valid_index].x();


    // find the minimum and maximum x values in the point list    
    float miny = (*points)[0].y();
    float maxy = miny;
    
    osg::notify(osg::INFO) << "DelaunayTriangulator: finding minimum and maximum Y values\n";
    osg::Vec3Array::const_iterator mmi;
    for (mmi=points->begin(); mmi!=points->end(); ++mmi)
    {
        if (mmi->y() < miny) miny = mmi->y();
        if (mmi->y() > maxy) maxy = mmi->y();
    }
    
    // add supertriangle vertices to the point list
    // gwm added 1.05* to ensure that supervertices are outside the domain of points.
    // original value could make 2 coincident points for regular arrays of x,y,h data.
    // this mod allows regular spaced arrays to be used.
    points_->push_back(osg::Vec3(minx - 1.05*(maxx - minx), miny - 0.01*(maxy - miny), 0));
    points_->push_back(osg::Vec3(maxx + 0.01*(maxx - minx), miny - 0.01*(maxy - miny), 0));
    points_->push_back(osg::Vec3(maxx + 0.01*(maxx - minx), maxy + 1.05*(maxy - miny), 0));

    // add supertriangle to triangle list
    triangles.push_back(Triangle(last_valid_index+1, last_valid_index+2, last_valid_index+3, points));

    
    // begin triangulation    
    GLuint pidx = 0;
    osg::Vec3Array::const_iterator i;    
    
    osg::notify(osg::INFO) << "DelaunayTriangulator: triangulating vertex grid (" << (points->size()-3) <<" points)\n";    

    for (i=points->begin(); i!=points->end(); ++i, ++pidx)
    {

        // don't process supertriangle vertices
        if (pidx > last_valid_index) break;

        Edge_set edges;

        // iterate through triangles
        Triangle_list::iterator j, next_j;
        for (j=triangles.begin(); j!=triangles.end(); j = next_j)
        {

            next_j = j;
            ++next_j;

            // compute the circumcircle
            osg::Vec3 cc = j->get_circumcircle();

            // OPTIMIZATION: since points are pre-sorted by the X component,
            // check whether we can discard this triangle for future operations
            float xdist = i->x() - cc.x();
            // this is where the circumcircles radius rather than R^2 is faster.
            // original code used r^2 and needed to test xdist*xdist>cc.z && i->x()>cc.x().
            if ((xdist ) > cc.z() )
            {
                discarded_tris.push_back(*j);
                triangles.erase(j);
            }
            else
            {

                // if the point lies in the triangle's circumcircle then add
                // its edges to the edge list and remove the triangle
                if (point_in_circle(*i, cc))
                {
                    for (int ei=0; ei<3; ++ei)
                    {
                        std::pair<Edge_set::iterator, bool> result = edges.insert(j->get_edge(ei));
                        if (!result.second)
                        {
                            // cast away constness of a set element, which is
                            // safe in this case since the set_duplicate is
                            // not used as part of the Less operator.
                            Edge& edge = const_cast<Edge&>(*(result.first));
                            // not clear why this change is needed? But prevents removal of twice referenced edges??
                      //      edge.set_duplicate(true);
                            edge.set_duplicate(!edge.get_duplicate());
                        }
                    }                    
                    triangles.erase(j);
                }
            }
        }

        // remove duplicate edges and add new triangles
        Edge_set::iterator ci;
        for (ci=edges.begin(); ci!=edges.end(); ++ci)
        {
            if (!ci->get_duplicate())
            {
                triangles.push_back(Triangle(pidx, ci->ib(), ci->ie(), points));
            }
        }
    }

    osg::notify(osg::INFO) << "DelaunayTriangulator: finalizing and cleaning up structures\n";

    // remove supertriangle vertices
    points->pop_back();
    points->pop_back();
    points->pop_back();

    // rejoin the two triangle lists
    triangles.insert(triangles.begin(), discarded_tris.begin(), discarded_tris.end());

        // GWM July 2005 eliminate any triangle with an edge crossing a constraint line
    // http://www.geom.uiuc.edu/~samuelp/del_project.html
    // we could also implement the sourcecode in http://gts.sourceforge.net/reference/gts-delaunay-and-constrained-delaunay-triangulations.html
    // this uses the set of lines which are boundaries of the constraints, including points
    // added to the contours by tesselation.
    for (linelist::iterator dcitr=constraint_lines.begin();dcitr!=constraint_lines.end();dcitr++)
    {
        //DelaunayConstraint *dc=(*dcitr).get();
        const osg::Vec3Array* vercon = dynamic_cast<const osg::Vec3Array*>((*dcitr)->getVertexArray());
        if (vercon)
        {
            for (unsigned int ipr=0; ipr<(*dcitr)->getNumPrimitiveSets(); ipr++)
            {
                const osg::PrimitiveSet* prset=(*dcitr)->getPrimitiveSet(ipr);
                if (prset->getMode()==osg::PrimitiveSet::LINE_LOOP ||
                    prset->getMode()==osg::PrimitiveSet::LINE_STRIP)
                {
                    // loops or strips
                    // start with the last point on the loop
                    unsigned int ip1=getindex((*vercon)[prset->index (prset->getNumIndices()-1)],points_.get());
                    for (unsigned int i=0; i<prset->getNumIndices(); i++)
                    {
                        unsigned int ip2=getindex((*vercon)[prset->index(i)],points_.get());
                        if (i>0 || prset->getMode()==osg::PrimitiveSet::LINE_LOOP)
                        {
                            // dont check edge from end to start
                            // for strips
                            // 2 points on the constraint
                            bool edgused=false;// first check for exact edge indices are used.
                            Triangle_list::iterator titr;
                            const osg::Vec3 curp=(*vercon)[prset->index(i)];
                            int it=0;
                            for (titr=triangles.begin(); titr!=triangles.end() && !edgused; ++titr)
                            {
                                //check that the edge ip1-ip2 is not already part of the triangulation.
                                if (titr->isedge(ip1,ip2)) edgused=true;
                                if (titr->isedge(ip2,ip1)) edgused=true;
                                //        if (edgused) osg::notify(osg::WARN) << "Edge used in triangle " << it << " " << 
                                //            titr->a()<<","<< titr->b()<<","<< titr->c()<<  std::endl;
                                it++;
                            }
                            if (!edgused)
                            {
                                // then check for intermediate triangles, erase them and replace with constrained triangles.
                                // find triangle with point ip1 where the 2 edges from ip1 contain the line p1-p2.
                                osg::Vec2 p1((*points_)[ip1].x(),(*points_)[ip1].y()); // a constraint line joins p1-p2
                                osg::Vec2 p2((*points_)[ip2].x(),(*points_)[ip2].y());
                                int ntr=0;
                                std::vector<const Triangle *> trisToDelete; // array of triangles to delete from terrain.
                                // form 2 lists of vertices for the edges of the hole created.
                                // The hole joins vertex ip1 to ip2, and one list of edges lies to the left
                                // of the line ip1-ip2m the other to the right.
                                // a list of vertices forming 2 halves of the removed triangles.
                                // which in turn are filled in with the tesselator.
                                for (titr=triangles.begin(); titr!=triangles.end(); )
                                {
                                    int icut=titr->lineBisects(points_.get(),ip1,p2);
                                    //    osg::notify(osg::WARN) << "Testing triangle " << ntr << " "<< ip1 << " ti " <<
                                    //        titr->a()<< ","<<titr->b() <<"," <<titr->c() << std::endl;
                                    if (icut>0)
                                    {
                                        // triangle titr starts the constraint edge
                                        std::vector<unsigned int> edgeRight, edgeLeft; 
                                        edgeRight.push_back(ip1);
                                        edgeLeft.push_back(ip1);
                                        //        osg::notify(osg::WARN) << "hole first " << edgeLeft.back()<<  " rt " << edgeRight.back()<< std::endl;
                                        trisToDelete.push_back(&(*titr));
                                        // now find the unique triangle that shares the defined edge
                                        unsigned int e1, e2; // indices of ends of test triangle titr
                                        if    (icut==1)
                                        {
                                            // icut=1 implies vertex a is not involved
                                            e1=titr->b(); e2=titr->c();
                                        }
                                        else if (icut==2)
                                        {
                                            e1=titr->c(); e2=titr->a();
                                        }
                                        else if (icut==3)
                                        {
                                            e1=titr->a(); e2=titr->b();
                                        }
                                        edgeRight.push_back(e2);
                                        edgeLeft.push_back(e1);
                                        //        osg::notify(osg::WARN) << icut << "hole edges " << edgeLeft.back()<<  " rt " << edgeRight.back()<< std::endl;
                                        const Triangle *tradj=getTriangleWithEdge(e2,e1, &triangles);
                                        if (tradj)
                                        {
                                            while (tradj && !tradj->usesVertex(ip2) && trisToDelete.size()<999)
                                            {
                                                trisToDelete.push_back(tradj);
                                                icut=tradj->whichEdge(points_.get(),p1,p2,e1,e2);
                                                //    osg::notify(osg::WARN)  << ntr << " cur triedge " << icut << " " << ip1 <<
                                                //        " to " << ip2 << " tadj " << tradj->a()<< ","<<tradj->b() <<"," 
                                                //        <<tradj->c() <<std::endl;
                                                if        (icut==1) {e1=tradj->b(); e2=tradj->c();} // icut=1 implies vertex a is not involved
                                                else if (icut==2) {e1=tradj->c(); e2=tradj->a();}
                                                else if (icut==3) {e1=tradj->a(); e2=tradj->b();}
                                                if (edgeLeft.back()!=e1 && edgeRight.back()==e2 && e1!=ip2) {
                                                    edgeLeft.push_back(e1);
                                                } else if(edgeRight.back()!=e2 && edgeLeft.back()==e1 && e2!=ip2) {
                                                    edgeRight.push_back(e2);
                                                } else {
                                                    if (!tradj->usesVertex(ip2)) osg::notify(osg::WARN) << "tradj error " << tradj->a()<<  " , " << tradj->b()<<  " , " << tradj->c()<< std::endl;
                                                }
                                                tradj=getTriangleWithEdge(e2,e1, &triangles);
                                            }
                                            if (trisToDelete.size()>=900) {
                                                osg::notify(osg::WARN) << " found " << trisToDelete.size() << " adjacent tris " <<std::endl;
                                            }
                                        }

                                        // both lines end at ip2 point.
                                        edgeLeft.push_back(ip2);
                                        edgeRight.push_back(ip2);
                                        if (tradj) trisToDelete.push_back(tradj);
                                        //        osg::notify(osg::WARN) << icut << "hole last " << edgeLeft.back()<<  " rt " << edgeRight.back()<< std::endl;
                                        Triangle_list constrainedtris=fillHole(points_.get(),edgeLeft);
                                        triangles.insert(triangles.begin(), constrainedtris.begin(), constrainedtris.end());
                                        constrainedtris=fillHole(points_.get(),edgeRight);
                                        triangles.insert(triangles.begin(), constrainedtris.begin(), constrainedtris.end());

                                    }
                                    ++titr;
                                    ntr++;
                                }
                                // remove the triangles list
                                Triangle_list::iterator tri; // counts through triangles
                                for (tri=triangles.begin(); tri!=triangles.end(); )
                                {
                                    bool deleted=false;
                                    for (std::vector<const Triangle *>::const_iterator deleteTri=trisToDelete.begin(); 
                                    deleteTri!=trisToDelete.end(); deleteTri++)
                                    {
                                        if (&(*tri)==*deleteTri)
                                        {
                                            deleted=true;
                                            tri=triangles.erase(tri);
                                        }
                                    }
                                    if (!deleted) ++tri;
                                }
                            }
                        } // strip test

                        ip1=ip2; // next edge of line
                    }
                }
            }
        }
    }
    // GWM Sept 2005 end


    // initialize index storage vector
    std::vector<GLuint> pt_indices;
    pt_indices.reserve(triangles.size() * 3);

    // build osg primitive
    osg::notify(osg::INFO) << "DelaunayTriangulator: building primitive(s)\n";
    Triangle_list::const_iterator ti;
    for (ti=triangles.begin(); ti!=triangles.end(); ++ti)
    {

        // don't add this triangle to the primitive if it shares any vertex with
        // the supertriangle
          // Also don't add degenerate (zero radius) triangles
        if (ti->a() <= last_valid_index && ti->b() <= last_valid_index && ti->c() <= last_valid_index && ti->get_circumcircle().z()>0.0)
        {

            if (normals_.valid())
            {
                (normals_.get())->push_back(ti->compute_normal(points));
            }

            pt_indices.push_back(ti->a());
            pt_indices.push_back(ti->b());
            pt_indices.push_back(ti->c());
        }
    }

    prim_tris_ = new osg::DrawElementsUInt(GL_TRIANGLES, pt_indices.size(), &(pt_indices.front()));

    osg::notify(osg::WARN) << "DelaunayTriangulator: process done, " << prim_tris_->getNumPrimitives() << " triangles remain\n";
    
    return true;
}

void DelaunayTriangulator::removeInternalTriangles(DelaunayConstraint *dc )
{
    // Triangle_list *triangles
    // remove triangle from terrain prim_tris_ internal to each constraintline
    // and move to the constraint line to make an alternative geometry,
    // possibly with alternative texture, and texture map
    int ndel=0;
    osg::Vec3Array::iterator normitr;
    if( normals_.valid() )
        normitr = normals_->begin();
    
    //        osg::notify(osg::WARN) << "DelaunayTriangulator: removeinternals, " << std::endl;
    for (osg::DrawElementsUInt::iterator triit=prim_tris_->begin(); triit!=prim_tris_->end(); )
    {
        // triangle joins points_[itr, itr+1, itr+2]
        Triangle tritest((*triit), *(triit+1), *(triit+2), points_.get());
        if ((*triit==166 && *(triit+1)==162 && *(triit+2)==161) ||
            (*triit==166 && *(triit+1)==165 && *(triit+2)==164) )
        {
            osg::Vec3 ctr=tritest.compute_centroid( points_.get());
            osg::notify(osg::WARN) << "testverts: " << ((*points_)[(*triit)].x()) << "," << ((*points_)[*(triit)].y()) <<","<<((*points_)[*(triit)].z())<<std::endl;
            osg::notify(osg::WARN) << "testverts: " << ((*points_)[*(triit+1)].x()) << "," << ((*points_)[*(triit+1)].y()) <<","<<((*points_)[*(triit+1)].z())<<std::endl;
            osg::notify(osg::WARN) << "testverts: " << ((*points_)[*(triit+2)].x()) << "," << ((*points_)[*(triit+2)].y()) <<","<<((*points_)[*(triit+2)].z())<<std::endl;
            osg::notify(osg::WARN) << "DelaunayTriangulator: why remove, " << (*triit) << "," << *(triit+1) <<","<<*(triit+2)<<
                " " << (dc->windingNumber(ctr))<< std::endl;
        }
        if ( dc->contains(tritest.compute_centroid( points_.get()) ) )
        {
            // centroid is inside the triangle, so IF inside linear, remove
            // osg::notify(osg::WARN) << "DelaunayTriangulator: remove, " << (*triit) << "," << *(triit+1) <<","<<*(triit+2)<< std::endl;
            dc->addtriangle((*triit), *(triit+1), *(triit+2));
            triit=prim_tris_->erase(triit);
            triit=prim_tris_->erase(triit);
            triit=prim_tris_->erase(triit);
            if (normals_.valid())
            {
                // erase the corresponding normal
                normitr=normals_->erase(normitr);
            }
            ndel++;
        }
        else
        {
            if (normals_.valid())
            {
                normitr++;
            }
            
            triit+=3;
        }
    }

    osg::notify(osg::INFO) << "end of test dc, deleted " << ndel << std::endl;
}
//=== DelaunayConstraint functions

float DelaunayConstraint::windingNumber(const osg::Vec3 testpoint) const
{
    // return winding number of loop around testpoint. Only in 2D, x-y coordinates assumed!
    float theta=0; // sum of angles subtended by the line array - the winding number
    const osg::Vec3Array *vertices= dynamic_cast<const osg::Vec3Array*>(getVertexArray());
    if (vertices)
    {
        for (unsigned int ipr=0; ipr<getNumPrimitiveSets(); ipr++)
        {
            const osg::PrimitiveSet* prset=getPrimitiveSet(ipr);
            if (prset->getMode()==osg::PrimitiveSet::LINE_LOOP)
            {
                // nothing else loops
                // start with the last point on the loop
                const osg::Vec3 prev=(*vertices)[prset->index (prset->getNumIndices()-1)];
                osg::Vec3 pi(prev.x()-testpoint.x(),prev.y()-testpoint.y(),0);
                pi.normalize();
                for (unsigned int i=0; i<prset->getNumIndices(); i++)
                {
                    const osg::Vec3 curp=(*vertices)[prset->index (i)];
                    osg::Vec3 edge(curp.x()-testpoint.x(),curp.y()-testpoint.y(),0);
                    edge.normalize();
                    double cth=edge*pi;
                    if (cth<=-0.99999 )
                    {
                        // testpoint is on edge and between 2 points
                        return 0; //
                    }
                    else
                    {
                        if (cth<0.99999)
                        {
                            float dang=(cth<1 && cth>-1)?acos(edge*pi):0; // this is unsigned angle
                            float zsign=edge.x()*pi.y()-pi.x()*edge.y(); // z component of..(edge^pi).z();
                            if (zsign>0) theta+=dang; // add the angle subtended appropriately
                            else if (zsign<0) theta-=dang;
                        }
                    }
                    pi=edge;
                }
            }
        }
    }
    
    return theta/osg::PI/2.0; // should be 0 or 2 pi.
}
osg::DrawElementsUInt *DelaunayConstraint::makeDrawable()
{ 
    // initialize index storage vector for internal triangles.
    std::vector<GLuint> pt_indices;
    pt_indices.reserve(_interiorTris.size() * 3);
    trilist::const_iterator ti;
    for (ti=_interiorTris.begin(); ti!=_interiorTris.end(); ++ti)
    {
        
        //  if (normals_.valid()) {
        //        (normals_.get())->push_back(ti->compute_normal(points));
        //  }
        
        pt_indices.push_back((*ti)[0]);
        pt_indices.push_back((*ti)[1]);
        pt_indices.push_back((*ti)[2]);
    }
    prim_tris_ = new osg::DrawElementsUInt(GL_TRIANGLES, pt_indices.size(), &(pt_indices.front()));
    
    return prim_tris_.get();
}
bool DelaunayConstraint::contains(const osg::Vec3 testpoint) const 
{
    // true if point is internal to the loop.
    float theta=windingNumber(testpoint); // sum of angles subtended by the line array - the winding number
    return fabs(theta)>0.9; // should be 0 or 1 (or 2,3,4 for very complex not permitted loops).
}
bool DelaunayConstraint::outside(const osg::Vec3 testpoint) const 
{
    // true if point is outside the loop.
    float theta=windingNumber(testpoint); // sum of angles subtended by the line array - the winding number
    return fabs(theta)<.05; // should be 0 if outside.
}


void DelaunayConstraint::addtriangle(const int i1,const int i2, const int i3)
{
    // a triangle joins vertices i1,i2,i3 in the points of the delaunay triangles.
    // points is the array of poitns in the triangulator;
    // add triangle to the constraint
    int *ip=new int[3];
    ip[0]=i1;
    ip[1]=i2;
    ip[2]=i3;
    _interiorTris.push_back(ip);
}
osg::Vec3Array* DelaunayConstraint::getPoints(const osg::Vec3Array *points)
{
    //points_ is the array of points that can be used to render the triangles in this DC.
    osg::ref_ptr<osg::Vec3Array> points_=new osg::Vec3Array;
    trilist::iterator ti;
    for (ti=_interiorTris.begin(); ti!=_interiorTris.end(); ++ti) {
        int idx=0;
        int ip[3]={-1,-1,-1};
        // find if points[i1/i2/i3] already in the vertices points_
        for (osg::Vec3Array::iterator ivert=points_->begin(); ivert!=points_->end(); ivert++)
        {
            if (ip[0]<0 && *ivert==(*points)[(*ti)[0]])
            {
                (*ti)[0]=ip[0]=idx;
            }
            if (ip[1]<0 && *ivert==(*points)[(*ti)[1]])
            {
                (*ti)[1]=ip[1]=idx;
            }
            if (ip[2]<0 && *ivert==(*points)[(*ti)[2]])
            {
                (*ti)[2]=ip[2]=idx;
            }
            idx++;
        }
        if (ip[0]<0)
        {
            points_->push_back((*points)[(*ti)[0]]);
            (*ti)[0]=ip[0]=points_->size()-1;
        }
        if (ip[1]<0)
        {
            points_->push_back((*points)[(*ti)[1]]);
            (*ti)[1]=ip[1]=points_->size()-1;
        }
        if (ip[2]<0)
        {
            points_->push_back((*points)[(*ti)[2]]);
            (*ti)[2]=ip[2]=points_->size()-1;
        }
    }
    makeDrawable();
    return points_.release();
}

void DelaunayConstraint::handleOverlaps(void)
{
    // use tesselator to interpolate crossing vertices.
    osg::ref_ptr<osgUtil::Tesselator> tscx=new osgUtil::Tesselator; // this assembles all the constraints
    tscx->setTesselationType(osgUtil::Tesselator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(true);
    tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ODD); 
    //  ODD chooses the winding =1, NOT overlapping areas of constraints.
    // nb this includes all the edges where delaunay constraints meet
    // draw a case to convince yourself!.
    
    tscx->retesselatePolygons(*this); // find all edges
}

} // namespace osgutil
