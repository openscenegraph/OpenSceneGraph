/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <set>
#include <list>
#include <iostream>

using namespace osgUtil;


class EdgeCollapse
{
public:

    EdgeCollapse():
        _targetNumTriangles(0) {}
    ~EdgeCollapse() {}

    void setGeometry(osg::Geometry* geometry);
    osg::Geometry* getGeometry() { return _geometry; }

    void setTargetNumOfTriangles(unsigned int num) { _targetNumTriangles = num; }

    unsigned int getNumOfTriangles() { return 0; }

    bool collapseMinimumErrorEdge() { return false; }

    void copyBackToGeometry() {}

    class Triangle;
    class Edge;

    typedef std::vector<float> FloatList;

    struct Point : public osg::Referenced
    {
        Point() {}
        
        unsigned int _index;

        osg::Vec3 _vertex;
        FloatList _attributes;        
    };

    struct Edge : public osg::Referenced
    {
        Edge() {}
        
        osg::ref_ptr<Point> _p1;
        osg::ref_ptr<Point> _p2;
        
        osg::ref_ptr<Triangle> _t1;
        osg::ref_ptr<Triangle> _t2;
        
        void setErrorMetric(float errorMetric) { _errorMetric = errorMetric; }
        float getErrorMetric() const { return _errorMetric; }
        
        float _errorMetric;
    };

    struct Triangle : public osg::Referenced
    {
        Triangle() {}
        
        osg::ref_ptr<Point> _p1;
        osg::ref_ptr<Point> _p2;
        osg::ref_ptr<Point> _p3;
        
        osg::ref_ptr<Edge> _e1;
        osg::ref_ptr<Edge> _e2;
        osg::ref_ptr<Edge> _e3;
    };



    
    struct LessErrorMetricFunctor
    {
        inline bool operator() (const osg::ref_ptr<Edge>& lhs,const osg::ref_ptr<Edge>& rhs) const
        {
            return lhs->getErrorMetric()<rhs->getErrorMetric();
        }
    };

    struct LessTriangleFunctor
    {
        inline bool operator() (const osg::ref_ptr<Triangle>& lhs,const osg::ref_ptr<Triangle>& rhs) const
        {
            if (lhs->_p1 < rhs->_p1) return true;
            if (lhs->_p1 > rhs->_p1) return false;

            if (lhs->_p2 < rhs->_p2) return true;
            if (lhs->_p2 > rhs->_p2) return false;

            if (lhs->_p1 < rhs->_p3) return true;
            return false;
        }
    };

    struct LessPointFunctor
    {
        inline bool operator() (const osg::ref_ptr<Point>& lhs,const osg::ref_ptr<Point>& rhs) const
        {
            return lhs->_index<rhs->_index;
        }
    };

    typedef std::set<osg::ref_ptr<Edge>,LessErrorMetricFunctor> EdgeSet;
    typedef std::set< osg::ref_ptr<Point>, LessPointFunctor> PointSet;
    typedef std::list< osg::ref_ptr<Triangle> > TriangleList;

    Triangle* addTriangle(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        std::cout<<"addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;
        abort();
#if 0
        // detect if triangle is degenerate.
        if (p1==p2 || p2==p3 || p1==p3) return 0;
        
        Triangle* triangle = new Triangle;

        Point* points[3];
        points[0] = addPoint(triangle, p1);
        points[1] = addPoint(triangle, p2);
        points[2] = addPoint(triangle, p3);
        
        // find the lowest value point in the list.
        unsigned int lowest = 0;        
        if (points[1]<points[lowest]) lowest = 1;
        if (points[2]<points[lowest]) lowest = 2;
        

        triangle->_p1 = points[lowest];
        triangle->_p2 = points[(lowest+1)%3];
        triangle->_p3 = points[(lowest+2)%3];

        triangle->_e1 = addEdge(triangle, triangle->_p1, triangle->_p2);
        triangle->_e2 = addEdge(triangle, triangle->_p2, triangle->_p3);
        triangle->_e3 = addEdge(triangle, triangle->_p3, triangle->_p1);
        
        _triangleList.insert(triangle);
        
        return triangle;
#endif        
    }
    
    Edge* addEdge(Triangle* triangle, Point* p1, Point* p2)
    {
        std::cout<<"addEdge("<<p1<<","<<p2<<")"<<std::endl;
        
        
    }

    Point* addPoint(Triangle* triangle, Point* p1)
    {
        std::cout<<"addPoint("<<p1<<")"<<std::endl;
    }

protected:

    osg::Geometry*                  _geometry;
    unsigned int                    _targetNumTriangles;
    EdgeSet                         _edgeSet;
    TriangleList                    _triangleList;
    PointSet                        _pointSet;
    
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

typedef osg::TriangleIndexFunctor<CollectTriangleOperator> CollectTriangleIndexFunctor;

void EdgeCollapse::setGeometry(osg::Geometry* geometry)
{
    _geometry = geometry;

    CollectTriangleIndexFunctor collectTriangles;
    collectTriangles.setEdgeCollapse(this);
    
    _geometry->accept(collectTriangles);
}
 
Simplifier::Simplifier()
{
}

void Simplifier::simplify(osg::Geometry& geometry, float sampleRatio)
{
    std::cout<<"++++++++++++++simplifier************"<<std::endl;

    EdgeCollapse ec;
    ec.setGeometry(&geometry);

    ec.setTargetNumOfTriangles((unsigned int)(sampleRatio*(float)ec.getNumOfTriangles()));

    while (ec.collapseMinimumErrorEdge()) {}

    ec.copyBackToGeometry();

}

void Simplifier::simplify(osg::Geometry& geometry, unsigned int targetNumberOfTriangles)
{
    std::cout<<"------------simplifier************"<<std::endl;

    EdgeCollapse ec;
    ec.setGeometry(&geometry);

    ec.setTargetNumOfTriangles(targetNumberOfTriangles);

    while (ec.collapseMinimumErrorEdge()) {}

    ec.copyBackToGeometry();
}
