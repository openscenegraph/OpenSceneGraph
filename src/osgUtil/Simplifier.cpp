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


    struct Point : public osg::Referenced
    {
        Point() {}
        
        unsigned int _index;
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


    struct LessPtr
    {
        inline bool operator() (const osg::ref_ptr<Edge>& lhs,const osg::ref_ptr<Edge>& rhs) const
        {
            return lhs->getErrorMetric()<rhs->getErrorMetric();
        }
    };

    typedef std::set<osg::ref_ptr<Edge>,LessPtr> EdgeSet;


    void addTriangle(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        std::cout<<"addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;
    }

protected:

    osg::Geometry*                  _geometry;
    unsigned int                    _targetNumTriangles;
    EdgeSet                         _edgeSet;
    
};

struct MyTriangleOperator
{

    MyTriangleOperator():_ec(0) {}

    void setEdgeCollapse(EdgeCollapse* ec) { _ec = ec; }
    
    EdgeCollapse* _ec;    

    // for use  in the triangle functor.
    inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        _ec->addTriangle(p1,p2,p3);
    }

};

typedef osg::TriangleIndexFunctor<MyTriangleOperator> MyTriangleIndexFunctor;

void EdgeCollapse::setGeometry(osg::Geometry* geometry)
{
    _geometry = geometry;

    MyTriangleIndexFunctor collectTriangles;
    collectTriangles.setEdgeCollapse(this);
}
 
Simplifier::Simplifier()
{
}

void Simplifier::simplify(osg::Geometry& geometry, float sampleRatio)
{
    EdgeCollapse ec;
    ec.setGeometry(&geometry);

    ec.setTargetNumOfTriangles((unsigned int)(sampleRatio*(float)ec.getNumOfTriangles()));

    while (ec.collapseMinimumErrorEdge()) {}

    ec.copyBackToGeometry();
}

void Simplifier::simplify(osg::Geometry& geometry, unsigned int targetNumberOfTriangles)
{
    EdgeCollapse ec;
    ec.setGeometry(&geometry);

    ec.setTargetNumOfTriangles(targetNumberOfTriangles);

    while (ec.collapseMinimumErrorEdge()) {}

    ec.copyBackToGeometry();
}
