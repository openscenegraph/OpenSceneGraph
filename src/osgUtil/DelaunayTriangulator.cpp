#include <osgUtil/DelaunayTriangulator>

#include <osg/GL>
#include <osg/Vec3>
#include <osg/Array>
#include <osg/Notify>

#include <algorithm>
#include <list>
#include <set>

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

    float cx = 
        (((a.x() - c.x()) * (a.x() + c.x()) + 
        (a.y() - c.y()) * (a.y() + c.y())) / 2 * (b.y() - c.y()) -
        ((b.x() - c.x()) * (b.x() + c.x()) +
        (b.y() - c.y()) * (b.y() + c.y())) / 2 * (a.y() - c.y())) / D;

    float cy = 
        (((b.x() - c.x()) * (b.x() + c.x()) + 
        (b.y() - c.y()) * (b.y() + c.y())) / 2 * (a.x() - c.x()) -
        ((a.x() - c.x()) * (a.x() + c.x()) +
        (a.y() - c.y()) * (a.y() + c.y())) / 2 * (b.x() - c.x())) / D;

    float r2 = (c.x() - cx) * (c.x() - cx) + (c.y() - cy) * (c.y() - cy);

    return osg::Vec3(cx, cy, r2);
}


// Test whether a point (only the x and y coordinates are used) lies inside
// a circle; the circle is passed as a vector: (Cx, Cy, r^2).

inline bool point_in_circle(const osg::Vec3 &point, const osg::Vec3 &circle)
{
    float r2 = 
        (point.x() - circle.x()) * (point.x() - circle.x()) + 
        (point.y() - circle.y()) * (point.y() - circle.y());
    return r2 <= circle.z();
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
    struct Less {
        inline bool operator()(const Edge &e1, const Edge &e2) const
        {
            if (e1.ibs() < e2.ibs()) return true;
            if (e1.ibs() > e2.ibs()) return false;
            if (e1.ies() < e2.ies()) return true;
            return false;
        }
    };

    Edge() {}
    Edge(Vertex_index ib, Vertex_index ie) : ib_(ib), ie_(ie), ibs_(std::min(ib, ie)), ies_(std::max(ib, ie)), duplicate_(false) {}

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

class Triangle {
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

    inline osg::Vec3 compute_normal(osg::Vec3Array *points) const
    {
        osg::Vec3 N = ((*points)[b_] - (*points)[a_]) ^ ((*points)[c_] - (*points)[a_]);
        return N / N.length();
    }


private:
    Vertex_index a_;
    Vertex_index b_;
    Vertex_index c_;
    osg::Vec3 cc_;    
    Edge edge_[3];
};


// comparison function for sorting sample points by the X coordinate
bool Sample_point_compare(const osg::Vec3 &p1, const osg::Vec3 &p2)
{
    return p1.x() < p2.x();
}


// container types
typedef std::set<Edge, Edge::Less> Edge_set;
typedef std::list<Triangle> Triangle_list;



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

bool DelaunayTriangulator::triangulate()
{
    // check validity of input array
    if (!points_.valid()) {
        osg::notify(osg::WARN) << "Warning: DelaunayTriangulator::triangulate(): invalid sample point array" << std::endl;
        return false;
    }

    osg::Vec3Array *points = points_.get();

    if (points->size() < 1) {
        osg::notify(osg::WARN) << "Warning: DelaunayTriangulator::triangulate(): too few sample points" << std::endl;
        return false;
    }

    // initialize storage structures
    Triangle_list triangles;
    Triangle_list discarded_tris;    

    // pre-sort sample points
    osg::notify(osg::INFO) << "DelaunayTriangulator: pre-sorting sample points\n";
    std::sort(points->begin(), points->end(), Sample_point_compare);    

    // set the last valid index for the point list
    GLuint last_valid_index = points->size() - 1;    

    // find the minimum and maximum x values in the point list
    float minx = (*points)[0].x();
    float maxx = minx;

    // find the minimum and maximum x values in the point list    
    float miny = (*points)[0].y();
    float maxy = miny;
    osg::notify(osg::INFO) << "DelaunayTriangulator: finding minimum and maximum Y values\n";
    osg::Vec3Array::const_iterator mmi;
    for (mmi=points->begin(); mmi!=points->end(); ++mmi) {
        if (mmi->y() < miny) miny = mmi->y();
        if (mmi->y() > maxy) maxy = mmi->y();
    }
    
    // add supertriangle vertices to the point list
    points->push_back(osg::Vec3(minx - (maxx - minx), miny, 0));
    points->push_back(osg::Vec3(maxx, miny, 0));
    points->push_back(osg::Vec3(maxx, maxy + (maxy - miny), 0));

    // add supertriangle to triangle list
    triangles.push_back(Triangle(last_valid_index+1, last_valid_index+2, last_valid_index+3, points));

    
    // begin triangulation    
    GLuint pidx = 0;
    osg::Vec3Array::const_iterator i;    
    
    osg::notify(osg::INFO) << "DelaunayTriangulator: triangulating vertex grid (" << (points->size()-3) <<" points)\n";    

    for (i=points->begin(); i!=points->end(); ++i, ++pidx) {

        // don't process supertriangle vertices
        if (pidx > last_valid_index) break;

        Edge_set edges;

        // iterate through triangles
        Triangle_list::iterator j, next_j;
        for (j=triangles.begin(); j!=triangles.end(); j = next_j) {

            next_j = j;
            ++next_j;

            // compute the circumcircle
            osg::Vec3 cc = j->get_circumcircle();

            // OPTIMIZATION: since points are pre-sorted by the X component,
            // check whether we can discard this triangle for future operations
            float xdist = i->x() - cc.x();
            if ((xdist * xdist) > cc.z() && i->x() > cc.x()) {
                discarded_tris.push_back(*j);
                triangles.erase(j);
            } else {

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
                            edge.set_duplicate(true);
                        }
                    }                    
                    triangles.erase(j);
                }
            }
        }

        // remove duplicate edges and add new triangles
        Edge_set::iterator ci;
        for (ci=edges.begin(); ci!=edges.end(); ++ci) {
            if (!ci->get_duplicate()) {
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

    // initialize index storage vector
    std::vector<GLuint> pt_indices;
    pt_indices.reserve(triangles.size() * 3);

    // build osg primitive
    osg::notify(osg::INFO) << "DelaunayTriangulator: building primitive(s)\n";
    Triangle_list::const_iterator ti;
    for (ti=triangles.begin(); ti!=triangles.end(); ++ti) {

        // don't add this triangle to the primitive if it shares any vertex with
        // the supertriangle
        if (ti->a() <= last_valid_index && ti->b() <= last_valid_index && ti->c() <= last_valid_index) {

            if (normals_.valid()) {
                (normals_.get())->push_back(ti->compute_normal(points));
            }

            pt_indices.push_back(ti->a());
            pt_indices.push_back(ti->b());
            pt_indices.push_back(ti->c());
        }
    }

    prim_tris_ = new osg::DrawElementsUInt(GL_TRIANGLES, pt_indices.size(), &(pt_indices.front()));

    osg::notify(osg::INFO) << "DelaunayTriangulator: process done, " << (pt_indices.size() / 3) << " triangles created\n";
    
    return true;
}


}
