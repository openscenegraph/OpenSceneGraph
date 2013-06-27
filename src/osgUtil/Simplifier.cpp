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

#include <osg/TriangleIndexFunctor>

#include <osgUtil/Simplifier>

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TriStripVisitor>

#include <set>
#include <list>
#include <algorithm>

#include <iterator>

using namespace osgUtil;

struct dereference_less
{
    template<class T, class U>
    inline bool operator() (const T& lhs,const U& rhs) const
    {
        return *lhs < *rhs;
    }
};

template<class T>
bool dereference_check_less(const T& lhs,const T& rhs)
{
    if (lhs==rhs) return false;
    if (!lhs) return true;
    if (!rhs) return false;
    return *lhs < *rhs;
}

struct dereference_clear
{
    template<class T>
    inline void operator() (const T& t)
    {
        T& non_const_t = const_cast<T&>(t);
        non_const_t->clear();
    }
};

class EdgeCollapse
{
public:


#if 1
    typedef float error_type;
#else
    typedef double error_type;
#endif

    struct Triangle;
    struct Edge;
    struct Point;


    EdgeCollapse():
        _geometry(0),
        _computeErrorMetricUsingLength(false)  {}

    ~EdgeCollapse();

    void setGeometry(osg::Geometry* geometry, const Simplifier::IndexList& protectedPoints);
    osg::Geometry* getGeometry() { return _geometry; }

    void setComputeErrorMetricUsingLength(bool flag) { _computeErrorMetricUsingLength = flag; }
    bool getComputeErrorMetricUsingLength() const { return _computeErrorMetricUsingLength; }

    unsigned int getNumOfTriangles() { return _triangleSet.size(); }

    Point* computeInterpolatedPoint(Edge* edge,float r) const
    {
        Point* point = new Point;
        float r1 = 1.0f-r;
        float r2 = r;
        Point* p1 = edge->_p1.get();
        Point* p2 = edge->_p2.get();

        if (p1==0 || p2==0)
        {
            OSG_NOTICE<<"Error computeInterpolatedPoint("<<edge<<",r) p1 and/or p2==0"<<std::endl;
            return 0;
        }

        point->_vertex = p1->_vertex * r1 + p2->_vertex * r2;
        unsigned int s = osg::minimum(p1->_attributes.size(),p2->_attributes.size());
        for(unsigned int i=0;i<s;++i)
        {
            point->_attributes.push_back(p1->_attributes[i]*r1 + p2->_attributes[i]*r2);
        }
        return point;
    }

    Point* computeOptimalPoint(Edge* edge) const
    {
        return computeInterpolatedPoint(edge,0.5f);
    }

    error_type computeErrorMetric(Edge* edge,Point* point) const
    {
        if (_computeErrorMetricUsingLength)
        {
            error_type dx = error_type(edge->_p1->_vertex.x()) - error_type(edge->_p2->_vertex.x());
            error_type dy = error_type(edge->_p1->_vertex.y()) - error_type(edge->_p2->_vertex.y());
            error_type dz = error_type(edge->_p1->_vertex.z()) - error_type(edge->_p2->_vertex.z());
            return sqrt(dx*dx + dy*dy + dz*dz);
        }
        else if (point)
        {
            typedef std::set< osg::ref_ptr<Triangle> > LocalTriangleSet ;
            LocalTriangleSet triangles;
            std::copy( edge->_p1->_triangles.begin(), edge->_p1->_triangles.end(), std::inserter(triangles,triangles.begin()));
            std::copy( edge->_p2->_triangles.begin(), edge->_p2->_triangles.end(), std::inserter(triangles,triangles.begin()));

            const osg::Vec3& vertex = point->_vertex;
            error_type error = 0.0;

            if (triangles.empty()) return 0.0;

            for(LocalTriangleSet::iterator itr=triangles.begin();
                itr!=triangles.end();
                ++itr)
            {
                error += fabs( (*itr)->distance(vertex) );
            }

            // use average of error
            error /= error_type(triangles.size());

            return error;
        }
        else
        {
            return 0;
        }
    }

    void updateErrorMetricForEdge(Edge* edge)
    {
        if (!edge->_p1 || !edge->_p2)
        {
            OSG_NOTICE<<"Error updateErrorMetricForEdge("<<edge<<") p1 and/or p2==0"<<std::endl;
            return;
        }


        osg::ref_ptr<Edge> keep_local_reference_to_edge(edge);

        if (_edgeSet.count(keep_local_reference_to_edge)!=0)
        {
            _edgeSet.erase(keep_local_reference_to_edge);
        }

        edge->_proposedPoint = computeOptimalPoint(edge);

        if (_computeErrorMetricUsingLength)
        {
            edge->setErrorMetric( computeErrorMetric( edge, edge->_proposedPoint.get()));
        }
        else
        {
            edge->updateMaxNormalDeviationOnEdgeCollapse();

            if (edge->getMaxNormalDeviationOnEdgeCollapse()<=1.0 && !edge->isAdjacentToBoundary())
                edge->setErrorMetric( computeErrorMetric( edge, edge->_proposedPoint.get()));
            else
                edge->setErrorMetric( FLT_MAX );
         }

        _edgeSet.insert(keep_local_reference_to_edge);
    }

    void updateErrorMetricForAllEdges()
    {
        typedef std::vector< osg::ref_ptr<Edge> > LocalEdgeList ;
        LocalEdgeList edges;
        std::copy( _edgeSet.begin(), _edgeSet.end(), std::back_inserter(edges));

        _edgeSet.clear();

        for(LocalEdgeList::iterator itr=edges.begin();
            itr!=edges.end();
            ++itr)
        {
            Edge* edge = itr->get();

            if (_computeErrorMetricUsingLength)
            {
                edge->setErrorMetric( computeErrorMetric( edge, edge->_proposedPoint.get()));
            }
            else
            {
                edge->_proposedPoint = computeOptimalPoint(edge);
                edge->updateMaxNormalDeviationOnEdgeCollapse();

                if (edge->getMaxNormalDeviationOnEdgeCollapse()<=1.0 && !edge->isAdjacentToBoundary())
                    edge->setErrorMetric( computeErrorMetric( edge, edge->_proposedPoint.get()));
                else
                    edge->setErrorMetric( FLT_MAX );

            }
            _edgeSet.insert(edge);
        }
    }

    bool collapseMinimumErrorEdge()
    {
        if (!_edgeSet.empty())
        {
            Edge* edge = const_cast<Edge*>(_edgeSet.begin()->get());

            if (edge->getErrorMetric()==FLT_MAX)
            {
                OSG_INFO<<"collapseMinimumErrorEdge() return false due to edge->getErrorMetric()==FLT_MAX"<<std::endl;
                return false;
            }

            osg::ref_ptr<Point> pNew = edge->_proposedPoint.valid()? edge->_proposedPoint.get() : computeInterpolatedPoint(edge,0.5f);
            return (collapseEdge(edge,pNew.get()));
        }
        OSG_INFO<<"collapseMinimumErrorEdge() return false due to _edgeSet.empty()"<<std::endl;
        return false;
    }


    bool divideLongestEdge()
    {
        if (!_edgeSet.empty())
        {
            Edge* edge = const_cast<Edge*>(_edgeSet.rbegin()->get());

            if (edge->getErrorMetric()==FLT_MAX)
            {
                OSG_INFO<<"divideLongestEdge() return false due to edge->getErrorMetric()==FLT_MAX"<<std::endl;
                return false;
            }

            osg::ref_ptr<Point> pNew = edge->_proposedPoint.valid()? edge->_proposedPoint.get() : computeInterpolatedPoint(edge,0.5f);
            return (divideEdge(edge,pNew.get()));
        }
        OSG_INFO<<"divideLongestEdge() return false due to _edgeSet.empty()"<<std::endl;
        return false;
    }

    void copyBackToGeometry();

    typedef std::vector<float>                                                  FloatList;
    typedef std::set<osg::ref_ptr<Edge>,dereference_less >                      EdgeSet;
    typedef std::set< osg::ref_ptr<Point>,dereference_less >                    PointSet;
    typedef std::vector< osg::ref_ptr<Point> >                                  PointList;
    typedef std::list< osg::ref_ptr<Triangle> >                                 TriangleList;
    typedef std::set< osg::ref_ptr<Triangle> >                                  TriangleSet;
    typedef std::map< osg::ref_ptr<Triangle>, unsigned int, dereference_less >  TriangleMap;

    struct Point : public osg::Referenced
    {
        Point(): _protected(false), _index(0) {}

        bool _protected;

        unsigned int _index;

        osg::Vec3           _vertex;
        FloatList           _attributes;
        TriangleSet         _triangles;

        void clear()
        {
            _attributes.clear();
            _triangles.clear();
        }

        bool operator < ( const Point& rhs) const
        {
            if (_vertex < rhs._vertex) return true;
            if (rhs._vertex < _vertex) return false;

            return _attributes < rhs._attributes;
        }

        bool isBoundaryPoint() const
        {
            if (_protected) return true;

            for(TriangleSet::const_iterator itr=_triangles.begin();
                itr!=_triangles.end();
                ++itr)
            {
                const Triangle* triangle = itr->get();
                if ((triangle->_e1->_p1==this || triangle->_e1->_p2==this) && triangle->_e1->isBoundaryEdge()) return true;
                if ((triangle->_e2->_p1==this || triangle->_e2->_p2==this) && triangle->_e2->isBoundaryEdge()) return true;
                if ((triangle->_e3->_p1==this || triangle->_e3->_p2==this) && triangle->_e3->isBoundaryEdge()) return true;

                //if ((*itr)->isBoundaryTriangle()) return true;
            }
            return false;
        }

    };

    struct Edge : public osg::Referenced
    {
        Edge(): _errorMetric(0.0), _maximumDeviation(1.0) {}

        void clear()
        {
            _p1 = 0;
            _p2 = 0;
            _triangles.clear();
        }

        osg::ref_ptr<Point> _p1;
        osg::ref_ptr<Point> _p2;

        TriangleSet _triangles;

        error_type _errorMetric;
        error_type _maximumDeviation;

        osg::ref_ptr<Point> _proposedPoint;

        void setErrorMetric(error_type errorMetric) { _errorMetric = errorMetric; }
        error_type getErrorMetric() const { return _errorMetric; }

        bool operator < ( const Edge& rhs) const
        {
            // both error metrics are computed
            if (getErrorMetric()<rhs.getErrorMetric()) return true;
            else if (rhs.getErrorMetric()<getErrorMetric()) return false;

            if (dereference_check_less(_p1,rhs._p1)) return true;
            if (dereference_check_less(rhs._p1,_p1)) return false;

            return dereference_check_less(_p2,rhs._p2);
        }

        bool operator == ( const Edge& rhs) const
        {
            if (&rhs==this) return true;
            if (*this<rhs) return false;
            if (rhs<*this) return false;
            return true;
        }

        bool operator != ( const Edge& rhs) const
        {
            if (&rhs==this) return false;
            if (*this<rhs) return true;
            if (rhs<*this) return true;
            return false;
        }

        void addTriangle(Triangle* triangle)
        {
            _triangles.insert(triangle);
            // if (_triangles.size()>2) OSG_NOTICE<<"Warning too many triangles ("<<_triangles.size()<<") sharing edge "<<std::endl;
        }

        bool isBoundaryEdge() const
        {
            return _triangles.size()<=1;
        }

        bool isAdjacentToBoundary() const
        {
            return isBoundaryEdge() || _p1->isBoundaryPoint() || _p2->isBoundaryPoint();
        }


        void updateMaxNormalDeviationOnEdgeCollapse()
        {
            //OSG_NOTICE<<"updateMaxNormalDeviationOnEdgeCollapse()"<<std::endl;
            _maximumDeviation = 0.0f;
            for(TriangleSet::iterator itr1=_p1->_triangles.begin();
                itr1!=_p1->_triangles.end();
                ++itr1)
            {
                if (_triangles.count(*itr1)==0)
                {
                    _maximumDeviation = osg::maximum(_maximumDeviation, (*itr1)->computeNormalDeviationOnEdgeCollapse(this,_proposedPoint.get()));
                }
            }
            for(TriangleSet::iterator itr2=_p2->_triangles.begin();
                itr2!=_p2->_triangles.end();
                ++itr2)
            {
                if (_triangles.count(*itr2)==0)
                {
                    _maximumDeviation = osg::maximum(_maximumDeviation, (*itr2)->computeNormalDeviationOnEdgeCollapse(this,_proposedPoint.get()));
                }
            }
        }

        error_type getMaxNormalDeviationOnEdgeCollapse() const { return _maximumDeviation; }

    };

    struct Triangle : public osg::Referenced
    {
        Triangle() {}

        void clear()
        {
            _p1 = 0;
            _p2 = 0;
            _p3 = 0;

            _e1 = 0;
            _e2 = 0;
            _e3 = 0;
        }

        inline bool operator < (const Triangle& rhs) const
        {
            if (dereference_check_less(_p1,rhs._p1)) return true;
            if (dereference_check_less(rhs._p1,_p1)) return false;


            const Point* lhs_lower = dereference_check_less(_p2,_p3) ? _p2.get() : _p3.get();
            const Point* rhs_lower = dereference_check_less(rhs._p2,rhs._p3) ? rhs._p2.get() : rhs._p3.get();

            if (dereference_check_less(lhs_lower,rhs_lower)) return true;
            if (dereference_check_less(rhs_lower,lhs_lower)) return false;

            const Point* lhs_upper = dereference_check_less(_p2,_p3) ? _p3.get() : _p2.get();
            const Point* rhs_upper = dereference_check_less(rhs._p2,rhs._p3) ? rhs._p3.get() : rhs._p2.get();

            return dereference_check_less(lhs_upper,rhs_upper);
        }


        void setOrderedPoints(Point* p1, Point* p2, Point* p3)
        {
            Point* points[3];
            points[0] = p1;
            points[1] = p2;
            points[2] = p3;

            // find the lowest value point in the list.
            unsigned int lowest = 0;
            if (dereference_check_less(points[1],points[lowest])) lowest = 1;
            if (dereference_check_less(points[2],points[lowest])) lowest = 2;

            _p1 = points[lowest];
            _p2 = points[(lowest+1)%3];
            _p3 = points[(lowest+2)%3];
        }

        void update()
        {
            _plane.set(_p1->_vertex,_p2->_vertex,_p3->_vertex);

        }

        osg::Plane computeNewPlaneOnEdgeCollapse(Edge* edge,Point* pNew) const
        {
            const Point* p1 = (_p1==edge->_p1 || _p1==edge->_p2) ? pNew : _p1.get();
            const Point* p2 = (_p2==edge->_p1 || _p2==edge->_p2) ? pNew : _p2.get();
            const Point* p3 = (_p3==edge->_p1 || _p3==edge->_p2) ? pNew : _p3.get();

            return osg::Plane(p1->_vertex,p2->_vertex,p3->_vertex);
        }

        // note return 1 - dotproduct, so that deviation is in the range of 0.0 to 2.0, where 0 is coincident, 1.0 is 90 degrees, and 2.0 is 180 degrees.
        error_type computeNormalDeviationOnEdgeCollapse(Edge* edge,Point* pNew) const
        {
            const Point* p1 = (_p1==edge->_p1 || _p1==edge->_p2) ? pNew : _p1.get();
            const Point* p2 = (_p2==edge->_p1 || _p2==edge->_p2) ? pNew : _p2.get();
            const Point* p3 = (_p3==edge->_p1 || _p3==edge->_p2) ? pNew : _p3.get();

            osg::Vec3 new_normal = (p2->_vertex - p1->_vertex) ^ (p3->_vertex - p2->_vertex);
            new_normal.normalize();

            error_type result = 1.0 - (new_normal.x() * _plane[0] + new_normal.y() * _plane[1] + new_normal.z() * _plane[2]);
            return result;
        }

        error_type distance(const osg::Vec3& vertex) const
        {
            return error_type(_plane[0])*error_type(vertex.x())+
                   error_type(_plane[1])*error_type(vertex.y())+
                   error_type(_plane[2])*error_type(vertex.z())+
                   error_type(_plane[3]);
        }

        bool isBoundaryTriangle() const
        {
            return (_e1->isBoundaryEdge() || _e2->isBoundaryEdge() ||  _e3->isBoundaryEdge());
        }


        osg::ref_ptr<Point> _p1;
        osg::ref_ptr<Point> _p2;
        osg::ref_ptr<Point> _p3;

        osg::ref_ptr<Edge> _e1;
        osg::ref_ptr<Edge> _e2;
        osg::ref_ptr<Edge> _e3;

        osg::Plane _plane;

    };


    Triangle* addTriangle(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        //OSG_NOTICE<<"addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;

        // detect if triangle is degenerate.
        if (p1==p2 || p2==p3 || p1==p3) return 0;

        Triangle* triangle = new Triangle;

        Point* points[3];
        points[0] = addPoint(triangle, p1);
        points[1] = addPoint(triangle, p2);
        points[2] = addPoint(triangle, p3);

        // find the lowest value point in the list.
        unsigned int lowest = 0;
        if (dereference_check_less(points[1],points[lowest])) lowest = 1;
        if (dereference_check_less(points[2],points[lowest])) lowest = 2;

        triangle->_p1 = points[lowest];
        triangle->_p2 = points[(lowest+1)%3];
        triangle->_p3 = points[(lowest+2)%3];

        triangle->_e1 = addEdge(triangle, triangle->_p1.get(), triangle->_p2.get());
        triangle->_e2 = addEdge(triangle, triangle->_p2.get(), triangle->_p3.get());
        triangle->_e3 = addEdge(triangle, triangle->_p3.get(), triangle->_p1.get());

        triangle->update();

        _triangleSet.insert(triangle);

        return triangle;
    }

    Triangle* addTriangle(Point* p1, Point* p2, Point* p3)
    {
        // OSG_NOTICE<<"      addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;

        // detect if triangle is degenerate.
        if (p1==p2 || p2==p3 || p1==p3)
        {
            // OSG_NOTICE<<"    **** addTriangle failed - p1==p2 || p2==p3 || p1==p3"<<std::endl;
            return 0;
        }

        Triangle* triangle = new Triangle;

        Point* points[3];
        points[0] = addPoint(triangle, p1);
        points[1] = addPoint(triangle, p2);
        points[2] = addPoint(triangle, p3);

        // find the lowest value point in the list.
        unsigned int lowest = 0;
        if (dereference_check_less(points[1],points[lowest])) lowest = 1;
        if (dereference_check_less(points[2],points[lowest])) lowest = 2;

        triangle->_p1 = points[lowest];
        triangle->_p2 = points[(lowest+1)%3];
        triangle->_p3 = points[(lowest+2)%3];

        triangle->_e1 = addEdge(triangle, triangle->_p1.get(), triangle->_p2.get());
        triangle->_e2 = addEdge(triangle, triangle->_p2.get(), triangle->_p3.get());
        triangle->_e3 = addEdge(triangle, triangle->_p3.get(), triangle->_p1.get());

        triangle->update();

        _triangleSet.insert(triangle);

        return triangle;
    }

    void removeTriangle(Triangle* triangle)
    {
        if (triangle->_p1.valid()) removePoint(triangle,triangle->_p1.get());
        if (triangle->_p2.valid()) removePoint(triangle,triangle->_p2.get());
        if (triangle->_p3.valid()) removePoint(triangle,triangle->_p3.get());

        if (triangle->_e1.valid()) removeEdge(triangle,triangle->_e1.get());
        if (triangle->_e2.valid()) removeEdge(triangle,triangle->_e2.get());
        if (triangle->_e3.valid()) removeEdge(triangle,triangle->_e3.get());

        _triangleSet.erase(triangle);
    }

    void replaceTrianglePoint(Triangle* triangle, Point* pOriginal, Point* pNew)
    {
        if (triangle->_p1==pOriginal || triangle->_p2==pOriginal || triangle->_p3==pOriginal)
        {
            // fix the corner points to use the new point
            if (triangle->_p1==pOriginal) triangle->_p1=pNew;
            if (triangle->_p2==pOriginal) triangle->_p2=pNew;
            if (triangle->_p3==pOriginal) triangle->_p3=pNew;

            // fixes the edges so they point to use the new point
            triangle->_e1 = replaceEdgePoint(triangle->_e1.get(),pOriginal,pNew);
            triangle->_e2 = replaceEdgePoint(triangle->_e2.get(),pOriginal,pNew);
            triangle->_e3 = replaceEdgePoint(triangle->_e3.get(),pOriginal,pNew);

            // remove the triangle form the original point, and possibly the point if its the last triangle to use it
            removePoint(triangle, pOriginal);

            // add the triangle to that point
            addPoint(triangle,pNew);
        }

    }

    unsigned int testTriangle(Triangle* triangle)
    {
        unsigned int result = 0;
        if (!(triangle->_p1))
        {
            OSG_NOTICE<<"testTriangle("<<triangle<<") _p1==NULL"<<std::endl;
            ++result;
        }
        else if (triangle->_p1->_triangles.count(triangle)==0)
        {
            OSG_NOTICE<<"testTriangle("<<triangle<<") _p1->_triangles does not contain triangle"<<std::endl;
            ++result;
        }

        if (!(triangle->_p2))
        {
            OSG_NOTICE<<"testTriangle("<<triangle<<") _p2==NULL"<<std::endl;
            ++result;
        }
        else if (triangle->_p2->_triangles.count(triangle)==0)
        {
            OSG_NOTICE<<"testTriangle("<<triangle<<") _p2->_triangles does not contain triangle"<<std::endl;
            ++result;
        }

        if (!(triangle->_p3))
        {
            OSG_NOTICE<<"testTriangle("<<triangle<<") _p3==NULL"<<std::endl;
            ++result;
        }
        else if (triangle->_p3->_triangles.count(triangle)==0)
        {
            OSG_NOTICE<<"testTriangle("<<triangle<<") _p3->_triangles does not contain triangle"<<std::endl;
            ++result;
        }

        if (testEdge(triangle->_e1.get()))
        {
            ++result;
            OSG_NOTICE<<"testTriangle("<<triangle<<") _e1 test failed"<<std::endl;
        }

        if (testEdge(triangle->_e2.get()))
        {
            ++result;
            OSG_NOTICE<<"testTriangle("<<triangle<<") _e2 test failed"<<std::endl;
        }

        if (testEdge(triangle->_e3.get()))
        {
            OSG_NOTICE<<"testTriangle("<<triangle<<") _e3 test failed"<<std::endl;
            ++result;
        }

        return result;
    }

    unsigned int testAllTriangles()
    {
        unsigned int numErrors = 0;
        for(TriangleSet::iterator itr=_triangleSet.begin();
            itr!=_triangleSet.end();
            ++itr)
        {
            numErrors += testTriangle(const_cast<Triangle*>(itr->get()));
        }
        return numErrors;
    }

    Edge* addEdge(Triangle* triangle, Point* p1, Point* p2)
    {
        // OSG_NOTICE<<"        addEdge("<<p1<<","<<p2<<")"<<std::endl;
        osg::ref_ptr<Edge> edge = new Edge;
        if (dereference_check_less(p1, p2))
        {
            edge->_p1 = p1;
            edge->_p2 = p2;
        }
        else
        {
            edge->_p1 = p2;
            edge->_p2 = p1;
        }

        edge->setErrorMetric( computeErrorMetric( edge.get(), edge->_proposedPoint.get()));

        EdgeSet::iterator itr = _edgeSet.find(edge);
        if (itr==_edgeSet.end())
        {
            // OSG_NOTICE<<"          addEdge("<<edge.get()<<") edge->_p1="<<edge->_p1.get()<<" _p2="<<edge->_p2.get()<<std::endl;
            _edgeSet.insert(edge);
        }
        else
        {
            // OSG_NOTICE<<"          reuseEdge("<<edge.get()<<") edge->_p1="<<edge->_p1.get()<<" _p2="<<edge->_p2.get()<<std::endl;
            edge = *itr;
        }

        edge->addTriangle(triangle);

        return edge.get();
    }

    void removeEdge(Triangle* triangle, Edge* edge)
    {
        EdgeSet::iterator itr = _edgeSet.find(edge);
        if (itr!=_edgeSet.end())
        {
            edge->_triangles.erase(triangle);
            if (edge->_triangles.empty())
            {
                edge->_p1 = 0;
                edge->_p2 = 0;

                // edge no longer in use, so need to delete.
                _edgeSet.erase(itr);
            }
        }
    }

    Edge* replaceEdgePoint(Edge* edge, Point* pOriginal, Point* pNew)
    {
        if (edge->_p1==pOriginal || edge->_p2==pOriginal)
        {
            EdgeSet::iterator itr = _edgeSet.find(edge);
            if (itr!=_edgeSet.end())
            {
                // remove the edge from the list, as its positoin in the list
                // may need to change once its values have been ammended
                _edgeSet.erase(itr);
            }

            // modify its values
            if (edge->_p1==pOriginal) edge->_p1=pNew;
            if (edge->_p2==pOriginal) edge->_p2=pNew;

            if (dereference_check_less(edge->_p2,edge->_p1))
            {
                edge->_p1.swap(edge->_p2);
            }

            itr = _edgeSet.find(edge);
            if (itr!=_edgeSet.end())
            {
                // reuse existing edge.
                edge = const_cast<Edge*>(itr->get());
            }
            else
            {
                // put it back in.
                _edgeSet.insert(edge);
            }
            return edge;
        }
        else
        {
            return edge;
        }

    }


    bool collapseEdge(Edge* edge, Point* pNew)
    {
        //if (edge->_triangles.size()<2) return false;
        //if (edge->_triangles.size()>2) return false;

#ifdef ORIGIANAL_CODE
        if (edge->getMaxNormalDeviationOnEdgeCollapse()>1.0)
        {
            OSG_NOTICE<<"collapseEdge("<<edge<<") refused due to edge->getMaxNormalDeviationOnEdgeCollapse() = "<<edge->getMaxNormalDeviationOnEdgeCollapse()<<std::endl;
           return false;
        }
        else
        {
            //OSG_NOTICE<<"collapseEdge("<<edge<<") edge->getMaxNormalDeviationOnEdgeCollapse() = "<<edge->getMaxNormalDeviationOnEdgeCollapse()<<std::endl;
        }
#endif

        typedef std::set< osg::ref_ptr<Edge> > LocalEdgeList;

        osg::ref_ptr<Edge> keep_edge_locally_referenced_to_prevent_premature_deletion = edge;
        osg::ref_ptr<Point> keep_point_locally_referenced_to_prevent_premature_deletion = pNew;
        osg::ref_ptr<Point> edge_p1 = edge->_p1;
        osg::ref_ptr<Point> edge_p2 = edge->_p2;

        TriangleMap  triangleMap;
        TriangleList triangles_p1;
        TriangleList triangles_p2;
        LocalEdgeList oldEdges;


        if (edge_p1 != pNew)
        {
            for(TriangleSet::iterator itr=edge_p1->_triangles.begin();
                itr!=edge_p1->_triangles.end();
                ++itr)
            {
                if (edge->_triangles.count(*itr)==0)
                {
                    Triangle* triangle = const_cast<Triangle*>(itr->get());
                    triangles_p1.push_back(triangle);
                    oldEdges.insert(triangle->_e1);
                    oldEdges.insert(triangle->_e2);
                    oldEdges.insert(triangle->_e3);
                }
            }

            //triangles_p1 = edge_p1->_triangles;
        }

        if (edge_p2 != pNew)
        {
            for(TriangleSet::iterator itr=edge_p2->_triangles.begin();
                itr!=edge_p2->_triangles.end();
                ++itr)
            {
                if (edge->_triangles.count(*itr)==0)
                {
                    Triangle* triangle = const_cast<Triangle*>(itr->get());
                    triangles_p2.push_back(triangle);
                    oldEdges.insert(triangle->_e1);
                    oldEdges.insert(triangle->_e2);
                    oldEdges.insert(triangle->_e3);
                }
            }
            //triangles_p2 = edge_p2->_triangles;
        }

        for(LocalEdgeList::iterator oeitr=oldEdges.begin();
            oeitr!=oldEdges.end();
            ++oeitr)
        {
            _edgeSet.erase(*oeitr);

            const_cast<Edge*>(oeitr->get())->setErrorMetric(0.0f);

            _edgeSet.insert(*oeitr);
        }

        TriangleList::iterator titr_p1, titr_p2;

        for(titr_p1 = triangles_p1.begin();
            titr_p1 != triangles_p1.end();
            ++titr_p1)
        {
            removeTriangle(const_cast<Triangle*>(titr_p1->get()));
        }

        for(titr_p2 = triangles_p2.begin();
            titr_p2 != triangles_p2.end();
            ++titr_p2)
        {
            removeTriangle(const_cast<Triangle*>(titr_p2->get()));
        }

        //OSG_NOTICE<<"  pNew="<<pNew<<"\tedge_p1"<<edge_p1.get()<<"\tedge_p2"<<edge_p2.get()<<std::endl;

        // we copy the edge's _triangles and interate the copy of the triangle set to avoid invalidating iterators.
        TriangleSet trianglesToRemove = edge->_triangles;
        for(TriangleSet::iterator teitr=trianglesToRemove.begin();
            teitr!=trianglesToRemove.end();
            ++teitr)
        {
            Triangle* triangle = const_cast<Triangle*>(teitr->get());
            removeTriangle(triangle);
        }

        LocalEdgeList newEdges;


        for(titr_p1 = triangles_p1.begin();
            titr_p1 != triangles_p1.end();
            ++titr_p1)
        {
            Triangle* triangle = const_cast<Triangle*>(titr_p1->get());

            Point* p1 = (triangle->_p1==edge_p1 || triangle->_p1==edge_p2)? pNew : triangle->_p1.get();
            Point* p2 = (triangle->_p2==edge_p1 || triangle->_p2==edge_p2)? pNew : triangle->_p2.get();
            Point* p3 = (triangle->_p3==edge_p1 || triangle->_p3==edge_p2)? pNew : triangle->_p3.get();

            Triangle* newTri = addTriangle(p1,p2,p3);

            if (newTri)
            {
                newEdges.insert(newTri->_e1);
                newEdges.insert(newTri->_e2);
                newEdges.insert(newTri->_e3);
            }
        }


        for(titr_p2 = triangles_p2.begin();
            titr_p2 != triangles_p2.end();
            ++titr_p2)
        {
            Triangle* triangle = const_cast<Triangle*>(titr_p2->get());

            Point* p1 = (triangle->_p1==edge_p1 || triangle->_p1==edge_p2)? pNew : triangle->_p1.get();
            Point* p2 = (triangle->_p2==edge_p1 || triangle->_p2==edge_p2)? pNew : triangle->_p2.get();
            Point* p3 = (triangle->_p3==edge_p1 || triangle->_p3==edge_p2)? pNew : triangle->_p3.get();

            Triangle* newTri = addTriangle(p1,p2,p3);

            if (newTri)
            {
                newEdges.insert(newTri->_e1);
                newEdges.insert(newTri->_e2);
                newEdges.insert(newTri->_e3);
            }
        }

        LocalEdgeList edges2UpdateErrorMetric;

        LocalEdgeList::const_iterator newEdgeIt(newEdges.begin());
        while (newEdgeIt != newEdges.end())
        {
            const Point* p = 0;
            if (newEdgeIt->get()->_p1.get() != pNew)
                p = newEdgeIt->get()->_p1.get();
            else
                p = newEdgeIt->get()->_p2.get();

            TriangleSet::const_iterator triangleIt(p->_triangles.begin());
            while (triangleIt != p->_triangles.end())
            {
                const Triangle* triangle = triangleIt->get();
                if (triangle->_e1->_p1 == p || triangle->_e1->_p2 == p)
                    edges2UpdateErrorMetric.insert(triangle->_e1);
                if (triangle->_e2->_p1 == p || triangle->_e2->_p2 == p)
                    edges2UpdateErrorMetric.insert(triangle->_e2);
                if (triangle->_e3->_p1 == p || triangle->_e3->_p2 == p)
                    edges2UpdateErrorMetric.insert(triangle->_e3);

                ++triangleIt;
            }

            ++newEdgeIt;
        }

        edges2UpdateErrorMetric.insert(newEdges.begin(), newEdges.end());

        // OSG_NOTICE<<"Edges to recalibarate "<<edges2UpdateErrorMetric.size()<<std::endl;

        for(LocalEdgeList::iterator itr=edges2UpdateErrorMetric.begin();
            itr!=edges2UpdateErrorMetric.end();
            ++itr)
        {
            //OSG_NOTICE<<"updateErrorMetricForEdge("<<itr->get()<<")"<<std::endl;
            updateErrorMetricForEdge(const_cast<Edge*>(itr->get()));
        }

        return true;
    }


    bool divideEdge(Edge* edge, Point* pNew)
    {
         // OSG_NOTICE<<"divideEdge("<<edge<<") before _edgeSet.size()="<<_edgeSet.size()<<" _triangleSet.size()="<<_triangleSet.size()<<std::endl;

        // first collect the triangles associaged with edges that need deleting
        osg::ref_ptr<Edge> keep_edge_locally_referenced_to_prevent_premature_deletion = edge;
        TriangleSet triangles = edge->_triangles;

        // OSG_NOTICE<<"   numTriangles on edges "<<triangles.size()<<std::endl;

        // unsigned int numTriangles1 = _triangleSet.size();
        // unsigned int numBoundaryEdges1 = computeNumBoundaryEdges();
        // unsigned int numEdges1 = _edgeSet.size();

        typedef std::set< osg::ref_ptr<Edge> > LocalEdgeList;
        LocalEdgeList edges2UpdateErrorMetric;
        TriangleSet::iterator titr;


        // for each deleted triangle insert two new triangles
        for(titr = triangles.begin();
            titr != triangles.end();
            ++titr)
        {
            Triangle* tri = const_cast<Triangle*>(titr->get());
            int edgeToReplace = 0;
            if (edge->_p1 == tri->_p1)
            {
                if (edge->_p2 == tri->_p2.get()) edgeToReplace = 1; // edge p1,p2
                else if (edge->_p2 == tri->_p3.get()) edgeToReplace = 3; // edge p3, p1
            }
            else if (edge->_p1 == tri->_p2.get())
            {
                if (edge->_p2 == tri->_p3.get()) edgeToReplace = 2; // edge p2,p3
                else if (edge->_p2 == tri->_p1.get()) edgeToReplace = 1; // edge p1, p2
            }
            else if (edge->_p1 == tri->_p3.get())
            {
                if (edge->_p2 == tri->_p1.get()) edgeToReplace = 3; // edge p3,p1
                else if (edge->_p2 == tri->_p2.get()) edgeToReplace = 2; // edge p2, p3
            }

            Triangle* newTri1 = 0;
            Triangle* newTri2 = 0;
            switch(edgeToReplace)
            {
                case(0): // error, shouldn't get here.
                    OSG_NOTICE<<"Error EdgeCollapse::divideEdge(Edge*,Point*) passed invalid edge."<<std::endl;
                    return false;
                case(1): // p1, p2
                    // OSG_NOTICE<<"   // p1, p2 "<<std::endl;
                    // OSG_NOTICE<<"   newTri1 = addTriangle(tri->_p1.get(), pNew, tri->_p3.get());"<<std::endl;
                    newTri1 = addTriangle(tri->_p1.get(), pNew, tri->_p3.get());
                    // OSG_NOTICE<<"   newTri2 = addTriangle(pNew, tri->_p2.get(), tri->_p3.get());"<<std::endl;
                    newTri2 = addTriangle(pNew, tri->_p2.get(), tri->_p3.get());
                    break;
                case(2): // p2, p3
                    // OSG_NOTICE<<"   // p2, p3"<<std::endl;
                    // OSG_NOTICE<<"   newTri1 = addTriangle(tri->_p1.get(), tri->_p2.get(), pNew);"<<std::endl;
                    newTri1 = addTriangle(tri->_p1.get(), tri->_p2.get(), pNew);
                    //OSG_NOTICE<<"   newTri2 = addTriangle(tri->_p1.get(), pNew, tri->_p3.get());"<<std::endl;
                    newTri2 = addTriangle(tri->_p1.get(), pNew, tri->_p3.get());
                    break;
                case(3): // p3, p1
                    // OSG_NOTICE<<"   // p3, p1"<<std::endl;
                    // OSG_NOTICE<<"   newTri1 = addTriangle(tri->_p1.get(), tri->_p2.get(), pNew);"<<std::endl;
                    newTri1 = addTriangle(tri->_p1.get(), tri->_p2.get(), pNew);
                    // OSG_NOTICE<<"   newTri2 = addTriangle(pNew, tri->_p2.get(), tri->_p3.get());"<<std::endl;
                    newTri2 = addTriangle(pNew, tri->_p2.get(), tri->_p3.get());
                    break;
            }

            if (newTri1)
            {
                edges2UpdateErrorMetric.insert(newTri1->_e1.get());
                edges2UpdateErrorMetric.insert(newTri1->_e2.get());
                edges2UpdateErrorMetric.insert(newTri1->_e3.get());
            }
            if (newTri2)
            {
                edges2UpdateErrorMetric.insert(newTri2->_e1.get());
                edges2UpdateErrorMetric.insert(newTri2->_e2.get());
                edges2UpdateErrorMetric.insert(newTri2->_e3.get());
            }
        }

        // unsigned int numTriangles2 = _triangleSet.size();
        // unsigned int numEdges2 = _edgeSet.size();
        // unsigned int numBoundaryEdges2 = computeNumBoundaryEdges();

        // remove all the triangles associated with edge
        for(titr = triangles.begin();
            titr != triangles.end();
            ++titr)
        {
            removeTriangle(const_cast<Triangle*>(titr->get()));
        }

        for(LocalEdgeList::iterator itr=edges2UpdateErrorMetric.begin();
            itr!=edges2UpdateErrorMetric.end();
            ++itr)
        {
            //OSG_NOTICE<<"updateErrorMetricForEdge("<<itr->get()<<")"<<std::endl;
            if (itr->valid()) updateErrorMetricForEdge(const_cast<Edge*>(itr->get()));
        }

        // unsigned int numBoundaryEdges3 = computeNumBoundaryEdges();
        // unsigned int numEdges3 = _edgeSet.size();
        // unsigned int numTriangles3 = _triangleSet.size();

        // OSG_NOTICE<<"   numTriangles1="<<numTriangles1<<"   numTriangles2="<<numTriangles2<<"   numTriangles3="<<numTriangles3<<std::endl;
        // OSG_NOTICE<<"   numEdges1="<<numEdges1<<"   numEdges2="<<numEdges2<<"   numEdges3="<<numEdges3<<std::endl;
        // OSG_NOTICE<<"   numBoundaryEdges1="<<numBoundaryEdges1<<"   numBoundaryEdges2="<<numBoundaryEdges2<<"   numBoundaryEdges3="<<numBoundaryEdges3<<std::endl;
        // OSG_NOTICE<<"divideEdge("<<edge<<") after _edgeSet.size()="<<_edgeSet.size()<<" _triangleSet.size()="<<_triangleSet.size()<<std::endl;

        return true;
    }

    unsigned int testEdge(Edge* edge)
    {
        unsigned int numErrors = 0;
        for(TriangleSet::iterator teitr=edge->_triangles.begin();
            teitr!=edge->_triangles.end();
            ++teitr)
        {
            Triangle* triangle = const_cast<Triangle*>(teitr->get());
            if (!(triangle->_e1 == edge || triangle->_e2 == edge || triangle->_e3 == edge))
            {
                OSG_NOTICE<<"testEdge("<<edge<<"). triangle != point back to this edge"<<std::endl;
                OSG_NOTICE<<"                     triangle->_e1=="<<triangle->_e1.get()<<std::endl;
                OSG_NOTICE<<"                     triangle->_e2=="<<triangle->_e2.get()<<std::endl;
                OSG_NOTICE<<"                     triangle->_e3=="<<triangle->_e3.get()<<std::endl;
                ++numErrors;
            }
        }

        if (edge->_triangles.empty())
        {
            OSG_NOTICE<<"testEdge("<<edge<<")._triangles is empty"<<std::endl;
            ++numErrors;
        }
        return numErrors;
    }

    unsigned int testAllEdges()
    {
        unsigned int numErrors = 0;
        for(EdgeSet::iterator itr = _edgeSet.begin();
            itr!=_edgeSet.end();
            ++itr)
        {
            numErrors += testEdge(const_cast<Edge*>(itr->get()));
        }
        return numErrors;
    }

    unsigned int computeNumBoundaryEdges()
    {
        unsigned int numBoundaryEdges = 0;
        for(EdgeSet::iterator itr = _edgeSet.begin();
            itr!=_edgeSet.end();
            ++itr)
        {
            if ((*itr)->isBoundaryEdge()) ++numBoundaryEdges;
        }
        return numBoundaryEdges;
    }


    Point* addPoint(Triangle* triangle, unsigned int p1)
    {
        return addPoint(triangle,_originalPointList[p1].get());
    }

    Point* addPoint(Triangle* triangle, Point* point)
    {

        PointSet::iterator itr = _pointSet.find(point);
        if (itr==_pointSet.end())
        {
            //OSG_NOTICE<<"  addPoint("<<point.get()<<")"<<std::endl;
            _pointSet.insert(point);
        }
        else
        {
            point = const_cast<Point*>(itr->get());
            //OSG_NOTICE<<"  reusePoint("<<point.get()<<")"<<std::endl;
        }

        point->_triangles.insert(triangle);

        return point;
    }

    void removePoint(Triangle* triangle, Point* point)
    {
        PointSet::iterator itr = _pointSet.find(point);
        if (itr!=_pointSet.end())
        {
            point->_triangles.erase(triangle);

            if (point->_triangles.empty())
            {
                // point no longer in use, so need to delete.
                _pointSet.erase(itr);
            }
        }

    }

    unsigned int testPoint(Point* point)
    {
        unsigned int numErrors = 0;

        for(TriangleSet::iterator itr=point->_triangles.begin();
            itr!=point->_triangles.end();
            ++itr)
        {
            Triangle* triangle = const_cast<Triangle*>(itr->get());
            if (!(triangle->_p1 == point || triangle->_p2 == point || triangle->_p3 == point))
            {
                OSG_NOTICE<<"testPoint("<<point<<") error, triangle "<<triangle<<" does not point back to this point"<<std::endl;
                OSG_NOTICE<<"             triangle->_p1 "<<triangle->_p1.get()<<std::endl;
                OSG_NOTICE<<"             triangle->_p2 "<<triangle->_p2.get()<<std::endl;
                OSG_NOTICE<<"             triangle->_p3 "<<triangle->_p3.get()<<std::endl;
                ++numErrors;
            }
        }

        return numErrors;
    }

    unsigned int testAllPoints()
    {
        unsigned int numErrors = 0;
        for(PointSet::iterator itr = _pointSet.begin();
            itr!=_pointSet.end();
            ++itr)
        {
            numErrors += testPoint(const_cast<Point*>(itr->get()));
        }
        return numErrors;
    }

//protected:

    typedef std::vector< osg::ref_ptr<osg::Array> > ArrayList;

    osg::Geometry*                  _geometry;

    bool                            _computeErrorMetricUsingLength;
    EdgeSet                         _edgeSet;
    TriangleSet                     _triangleSet;
    PointSet                        _pointSet;
    PointList                       _originalPointList;

};

struct CollectTriangleOperator
{

    CollectTriangleOperator():_ec(0) {}

    void setEdgeCollapse(EdgeCollapse* ec) { _ec = ec; }

    EdgeCollapse* _ec;

    // for use  in the triangle functor.
    inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        _ec->addTriangle(p1,p2,p3);
    }

};

EdgeCollapse::~EdgeCollapse()
{
    std::for_each(_edgeSet.begin(),_edgeSet.end(),dereference_clear());

    std::for_each(_triangleSet.begin(),_triangleSet.end(),dereference_clear());
    std::for_each(_pointSet.begin(),_pointSet.end(),dereference_clear());
    std::for_each(_originalPointList.begin(),_originalPointList.end(),dereference_clear());
}


typedef osg::TriangleIndexFunctor<CollectTriangleOperator> CollectTriangleIndexFunctor;

class CopyArrayToPointsVisitor : public osg::ArrayVisitor
{
    public:
        CopyArrayToPointsVisitor(EdgeCollapse::PointList& pointList):
            _pointList(pointList) {}

        template<class T>
        void copy(T& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
                _pointList[i]->_attributes.push_back((float)array[i]);
        }

        virtual void apply(osg::Array&) {}
        virtual void apply(osg::ByteArray& array) { copy(array); }
        virtual void apply(osg::ShortArray& array) { copy(array); }
        virtual void apply(osg::IntArray& array) { copy(array); }
        virtual void apply(osg::UByteArray& array) { copy(array); }
        virtual void apply(osg::UShortArray& array) { copy(array); }
        virtual void apply(osg::UIntArray& array) { copy(array); }
        virtual void apply(osg::FloatArray& array) { copy(array); }

        virtual void apply(osg::Vec4ubArray& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                osg::Vec4ub& value = array[i];
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                attributes.push_back((float)value.r());
                attributes.push_back((float)value.g());
                attributes.push_back((float)value.b());
                attributes.push_back((float)value.a());
            }
        }

        virtual void apply(osg::Vec2Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                osg::Vec2& value = array[i];
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                attributes.push_back(value.x());
                attributes.push_back(value.y());
            }
        }

        virtual void apply(osg::Vec3Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                osg::Vec3& value = array[i];
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                attributes.push_back(value.x());
                attributes.push_back(value.y());
                attributes.push_back(value.z());
            }
        }

        virtual void apply(osg::Vec4Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                osg::Vec4& value = array[i];
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                attributes.push_back(value.x());
                attributes.push_back(value.y());
                attributes.push_back(value.z());
                attributes.push_back(value.w());
            }
        }

        EdgeCollapse::PointList& _pointList;


    protected:

        CopyArrayToPointsVisitor& operator = (const CopyArrayToPointsVisitor&) { return *this; }
};

class CopyVertexArrayToPointsVisitor : public osg::ArrayVisitor
{
    public:
        CopyVertexArrayToPointsVisitor(EdgeCollapse::PointList& pointList):
            _pointList(pointList) {}

        virtual void apply(osg::Vec2Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollapse::Point;
                _pointList[i]->_index = i;

                osg::Vec2& value = array[i];
                osg::Vec3& vertex = _pointList[i]->_vertex;
                vertex.set(value.x(),value.y(),0.0f);
            }
        }

        virtual void apply(osg::Vec3Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollapse::Point;
                _pointList[i]->_index = i;

                _pointList[i]->_vertex = array[i];
            }
        }

        virtual void apply(osg::Vec4Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollapse::Point;
                _pointList[i]->_index = i;

                osg::Vec4& value = array[i];
                osg::Vec3& vertex = _pointList[i]->_vertex;
                vertex.set(value.x()/value.w(),value.y()/value.w(),value.z()/value.w());
            }
        }

        EdgeCollapse::PointList& _pointList;

    protected:

        CopyVertexArrayToPointsVisitor& operator = (const CopyVertexArrayToPointsVisitor&) { return *this; }

};

void EdgeCollapse::setGeometry(osg::Geometry* geometry, const Simplifier::IndexList& protectedPoints)
{
    _geometry = geometry;

    // check to see if vertex attributes indices exists, if so expand them to remove them
    if (_geometry->containsSharedArrays())
    {
        // removing coord indices
        OSG_INFO<<"EdgeCollapse::setGeometry(..): Duplicate shared arrays"<<std::endl;
        _geometry->duplicateSharedArrays();
    }

    unsigned int numVertices = geometry->getVertexArray()->getNumElements();

    _originalPointList.resize(numVertices);

    // copy vertices across to local point list
    CopyVertexArrayToPointsVisitor copyVertexArrayToPoints(_originalPointList);
    _geometry->getVertexArray()->accept(copyVertexArrayToPoints);

    // copy other per vertex attributes across to local point list.
    CopyArrayToPointsVisitor        copyArrayToPoints(_originalPointList);

    for(unsigned int ti=0;ti<_geometry->getNumTexCoordArrays();++ti)
    {
        if (_geometry->getTexCoordArray(ti))
            geometry->getTexCoordArray(ti)->accept(copyArrayToPoints);
    }

    if (_geometry->getNormalArray() && _geometry->getNormalArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
        geometry->getNormalArray()->accept(copyArrayToPoints);

    if (_geometry->getColorArray() && _geometry->getColorArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
        geometry->getColorArray()->accept(copyArrayToPoints);

    if (_geometry->getSecondaryColorArray() && _geometry->getSecondaryColorArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
        geometry->getSecondaryColorArray()->accept(copyArrayToPoints);

    if (_geometry->getFogCoordArray() && _geometry->getFogCoordArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
        geometry->getFogCoordArray()->accept(copyArrayToPoints);

    for(unsigned int vi=0;vi<_geometry->getNumVertexAttribArrays();++vi)
    {
        if (_geometry->getVertexAttribArray(vi) &&  _geometry->getVertexAttribArray(vi)->getBinding()==osg::Array::BIND_PER_VERTEX)
            geometry->getVertexAttribArray(vi)->accept(copyArrayToPoints);
    }

    // now set the protected points up.
    for(Simplifier::IndexList::const_iterator pitr=protectedPoints.begin();
        pitr!=protectedPoints.end();
        ++pitr)
    {
        _originalPointList[*pitr]->_protected = true;
    }


    CollectTriangleIndexFunctor collectTriangles;
    collectTriangles.setEdgeCollapse(this);

    _geometry->accept(collectTriangles);

}



class CopyPointsToArrayVisitor : public osg::ArrayVisitor
{
    public:
        CopyPointsToArrayVisitor(EdgeCollapse::PointList& pointList):
            _pointList(pointList),
            _index(0) {}


        template<typename T,typename R>
        void copy(T& array, R /*dummy*/)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                if (_index<_pointList[i]->_attributes.size())
                {
                    float val = (_pointList[i]->_attributes[_index]);
                    array[i] = R (val);
                }
            }

            ++_index;
        }

        // use local typedefs if usinged char,short and int to get round gcc 3.3.1 problem with defining unsigned short()
        typedef unsigned char dummy_uchar;
        typedef unsigned short dummy_ushort;
        typedef unsigned int dummy_uint;

        virtual void apply(osg::Array&) {}
        virtual void apply(osg::ByteArray& array) { copy(array, char());}
        virtual void apply(osg::ShortArray& array) { copy(array, short()); }
        virtual void apply(osg::IntArray& array) { copy(array, int()); }
        virtual void apply(osg::UByteArray& array) { copy(array, dummy_uchar()); }
        virtual void apply(osg::UShortArray& array) { copy(array,dummy_ushort()); }
        virtual void apply(osg::UIntArray& array) { copy(array, dummy_uint()); }
        virtual void apply(osg::FloatArray& array) { copy(array, float()); }

        virtual void apply(osg::Vec4ubArray& array)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                array[i].set((unsigned char)attributes[_index],
                             (unsigned char)attributes[_index+1],
                             (unsigned char)attributes[_index+2],
                             (unsigned char)attributes[_index+3]);
            }
            _index += 4;
        }

        virtual void apply(osg::Vec2Array& array)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                if (_index+1<attributes.size()) array[i].set(attributes[_index],attributes[_index+1]);
            }
            _index += 2;
        }

        virtual void apply(osg::Vec3Array& array)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                if (_index+2<attributes.size()) array[i].set(attributes[_index],attributes[_index+1],attributes[_index+2]);
            }
            _index += 3;
        }

        virtual void apply(osg::Vec4Array& array)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                if (_index+3<attributes.size()) array[i].set(attributes[_index],attributes[_index+1],attributes[_index+2],attributes[_index+3]);
            }
            _index += 4;
        }

        EdgeCollapse::PointList& _pointList;
        unsigned int _index;

    protected:

        CopyPointsToArrayVisitor& operator = (CopyPointsToArrayVisitor&) { return *this; }
};

class NormalizeArrayVisitor : public osg::ArrayVisitor
{
    public:
        NormalizeArrayVisitor() {}

        template<typename Itr>
        void normalize(Itr begin, Itr end)
        {
            for(Itr itr = begin;
                itr != end;
                ++itr)
            {
                itr->normalize();
            }
        }

        virtual void apply(osg::Vec2Array& array) { normalize(array.begin(),array.end()); }
        virtual void apply(osg::Vec3Array& array) { normalize(array.begin(),array.end()); }
        virtual void apply(osg::Vec4Array& array) { normalize(array.begin(),array.end()); }

};

class CopyPointsToVertexArrayVisitor : public osg::ArrayVisitor
{
    public:
        CopyPointsToVertexArrayVisitor(EdgeCollapse::PointList& pointList):
            _pointList(pointList) {}

        virtual void apply(osg::Vec2Array& array)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i]->_index = i;
                osg::Vec3& vertex = _pointList[i]->_vertex;
                array[i].set(vertex.x(),vertex.y());
            }
        }

        virtual void apply(osg::Vec3Array& array)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i]->_index = i;
                array[i] = _pointList[i]->_vertex;
            }
        }

        virtual void apply(osg::Vec4Array& array)
        {
            array.resize(_pointList.size());

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i]->_index = i;
                osg::Vec3& vertex = _pointList[i]->_vertex;
                array[i].set(vertex.x(),vertex.y(),vertex.z(),1.0f);
            }
        }

        EdgeCollapse::PointList& _pointList;

    protected:

        CopyPointsToVertexArrayVisitor& operator = (const CopyPointsToVertexArrayVisitor&) { return *this; }
};


void EdgeCollapse::copyBackToGeometry()
{

    // rebuild the _pointList from the _pointSet
    _originalPointList.clear();
    std::copy(_pointSet.begin(),_pointSet.end(),std::back_inserter(_originalPointList));

    // copy vertices across to local point list
    CopyPointsToVertexArrayVisitor copyVertexArrayToPoints(_originalPointList);
    _geometry->getVertexArray()->accept(copyVertexArrayToPoints);

    // copy other per vertex attributes across to local point list.
    CopyPointsToArrayVisitor  copyArrayToPoints(_originalPointList);

    for(unsigned int ti=0;ti<_geometry->getNumTexCoordArrays();++ti)
    {
        if (_geometry->getTexCoordArray(ti))
            _geometry->getTexCoordArray(ti)->accept(copyArrayToPoints);
    }

    if (_geometry->getNormalArray() && _geometry->getNormalArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
    {
        _geometry->getNormalArray()->accept(copyArrayToPoints);

        // now normalize the normals.
        NormalizeArrayVisitor nav;
        _geometry->getNormalArray()->accept(nav);
    }

    if (_geometry->getColorArray() && _geometry->getColorArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
        _geometry->getColorArray()->accept(copyArrayToPoints);

    if (_geometry->getSecondaryColorArray() && _geometry->getSecondaryColorArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
        _geometry->getSecondaryColorArray()->accept(copyArrayToPoints);

    if (_geometry->getFogCoordArray() && _geometry->getFogCoordArray()->getBinding()==osg::Array::BIND_PER_VERTEX)
        _geometry->getFogCoordArray()->accept(copyArrayToPoints);

    for(unsigned int vi=0;vi<_geometry->getNumVertexAttribArrays();++vi)
    {
        if (_geometry->getVertexAttribArray(vi) &&  _geometry->getVertexAttribArray(vi)->getBinding()==osg::Array::BIND_PER_VERTEX)
            _geometry->getVertexAttribArray(vi)->accept(copyArrayToPoints);
    }

    typedef std::set< osg::ref_ptr<Triangle>, dereference_less >    TrianglesSorted;
    TrianglesSorted trianglesSorted;
    for(TriangleSet::iterator itr = _triangleSet.begin();
        itr != _triangleSet.end();
        ++itr)
    {
        trianglesSorted.insert(*itr);
    }

    osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt(GL_TRIANGLES,trianglesSorted.size()*3);
    unsigned int pos = 0;
    for(TrianglesSorted::iterator titr=trianglesSorted.begin();
        titr!=trianglesSorted.end();
        ++titr)
    {
        const Triangle* triangle = (*titr).get();
        (*primitives)[pos++] = triangle->_p1->_index;
        (*primitives)[pos++] = triangle->_p2->_index;
        (*primitives)[pos++] = triangle->_p3->_index;
    }

    _geometry->getPrimitiveSetList().clear();
    _geometry->addPrimitiveSet(primitives);

}


Simplifier::Simplifier(double sampleRatio, double maximumError, double maximumLength):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _sampleRatio(sampleRatio),
            _maximumError(maximumError),
            _maximumLength(maximumLength),
            _triStrip(true),
            _smoothing(true)

{
}

void Simplifier::simplify(osg::Geometry& geometry)
{
    // pass an empty list of indices to simply(Geometry,IndexList)
    // so that this one method handle both cases of non protected indices
    // and specified indices.
    IndexList emptyList;
    simplify(geometry,emptyList);
}

void Simplifier::simplify(osg::Geometry& geometry, const IndexList& protectedPoints)
{
    OSG_INFO<<"++++++++++++++simplifier************"<<std::endl;

    EdgeCollapse ec;
    ec.setComputeErrorMetricUsingLength(getSampleRatio()>=1.0);
    ec.setGeometry(&geometry, protectedPoints);
    ec.updateErrorMetricForAllEdges();

    unsigned int numOriginalPrimitives = ec._triangleSet.size();

    if (getSampleRatio()<1.0)
    {
        while (!ec._edgeSet.empty() &&
               continueSimplification((*ec._edgeSet.begin())->getErrorMetric() , numOriginalPrimitives, ec._triangleSet.size()) &&
               ec.collapseMinimumErrorEdge())
        {
           //OSG_INFO<<"   Collapsed edge ec._triangleSet.size()="<<ec._triangleSet.size()<<" error="<<(*ec._edgeSet.begin())->getErrorMetric()<<" vs "<<getMaximumError()<<std::endl;
        }

        OSG_INFO<<"******* AFTER EDGE COLLAPSE *********"<<ec._triangleSet.size()<<std::endl;
    }
    else
    {

        // up sampling...
        while (!ec._edgeSet.empty() &&
               continueSimplification((*ec._edgeSet.rbegin())->getErrorMetric() , numOriginalPrimitives, ec._triangleSet.size()) &&
//               ec._triangleSet.size() < targetNumTriangles  &&
               ec.divideLongestEdge())
        {
           //OSG_INFO<<"   Edge divided ec._triangleSet.size()="<<ec._triangleSet.size()<<" error="<<(*ec._edgeSet.rbegin())->getErrorMetric()<<" vs "<<getMaximumError()<<std::endl;
        }
        OSG_INFO<<"******* AFTER EDGE DIVIDE *********"<<ec._triangleSet.size()<<std::endl;
    }

    OSG_INFO<<"Number of triangle errors after edge collapse= "<<ec.testAllTriangles()<<std::endl;
    OSG_INFO<<"Number of edge errors before edge collapse= "<<ec.testAllEdges()<<std::endl;
    OSG_INFO<<"Number of point errors after edge collapse= "<<ec.testAllPoints()<<std::endl;
    OSG_INFO<<"Number of triangles= "<<ec._triangleSet.size()<<std::endl;
    OSG_INFO<<"Number of points= "<<ec._pointSet.size()<<std::endl;
    OSG_INFO<<"Number of edges= "<<ec._edgeSet.size()<<std::endl;
    OSG_INFO<<"Number of boundary edges= "<<ec.computeNumBoundaryEdges()<<std::endl;

    if (!ec._edgeSet.empty())
    {
        OSG_INFO<<std::endl<<"Simplifier, in = "<<numOriginalPrimitives<<"\tout = "<<ec._triangleSet.size()<<"\terror="<<(*ec._edgeSet.begin())->getErrorMetric()<<"\tvs "<<getMaximumError()<<std::endl<<std::endl;
        OSG_INFO<<           "        !ec._edgeSet.empty()  = "<<!ec._edgeSet.empty()<<std::endl;
        OSG_INFO<<           "        continueSimplification(,,)  = "<<continueSimplification((*ec._edgeSet.begin())->getErrorMetric() , numOriginalPrimitives, ec._triangleSet.size())<<std::endl;
    }

    ec.copyBackToGeometry();

    if (_smoothing)
    {
        osgUtil::SmoothingVisitor::smooth(geometry);
    }

    if (_triStrip)
    {
        osgUtil::TriStripVisitor stripper;
        stripper.stripify(geometry);
    }

}
