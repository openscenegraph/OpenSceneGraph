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

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TriStripVisitor>

#include <set>
#include <list>
#include <iostream>

using namespace osgUtil;


struct dereference_less
{
    template<class T>
    inline bool operator() (const T& lhs,const T& rhs) const
    {
        return *lhs < *rhs;
    }
};

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

    void copyBackToGeometry();

    struct Triangle;
    struct Edge;
    struct Point;

    typedef std::vector<float>                              FloatList;
    typedef std::set<osg::ref_ptr<Edge>,dereference_less>   EdgeSet;
    typedef std::set< osg::ref_ptr<Point>,dereference_less>                 PointSet;
    typedef std::list< osg::ref_ptr<Triangle> >             TriangleList;

    struct Point : public osg::Referenced
    {
        Point():_index(0) {}
        
        unsigned int _index;

        osg::Vec3       _vertex;
        FloatList       _attributes;
        TriangleList    _triangles;

        bool operator < ( const Point& rhs) const
        {
            if (_vertex < rhs._vertex) return true;
            if (rhs._vertex < _vertex) return false;
            
            return _attributes < rhs._attributes;
        }


    };

    struct Edge : public osg::Referenced
    {
        Edge():_errorMetric(0.0f) {}
        
        osg::ref_ptr<Point> _p1;
        osg::ref_ptr<Point> _p2;
        
        osg::ref_ptr<Triangle> _t1;
        osg::ref_ptr<Triangle> _t2;
        
        void setErrorMetric(float errorMetric) { _errorMetric = errorMetric; }
        float getErrorMetric() const { return _errorMetric; }
        
        bool operator < ( const Edge& rhs) const
        {
            if (getErrorMetric()<rhs.getErrorMetric()) return true;
            else if (rhs.getErrorMetric()>getErrorMetric()) return false;
            
            if (_p1 < rhs._p1) return true;
            if (rhs._p1 < _p1) return false;

            return (_p2 < rhs._p2);
        }
        
        void addTriangle(Triangle* triangle)
        {
            if (!_t1)
            {
                _t1 = triangle;
            }
            else if (!_t2)
            {
                _t2 = triangle;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Warning too many traingles sharing edge"<<std::endl;
            }
        }
        
        float _errorMetric;
    };

    struct Triangle : public osg::Referenced
    {
        Triangle() {}
        
        inline bool operator < (const Triangle& rhs) const
        {
            if (_p1 < rhs._p1) return true;
            if (rhs._p1 < _p1) return false;

            if (_p2 < rhs._p2) return true;
            if (rhs._p2 < _p2) return false;

            return (_p3 < rhs._p3);
        }

        osg::ref_ptr<Point> _p1;
        osg::ref_ptr<Point> _p2;
        osg::ref_ptr<Point> _p3;
        
        osg::ref_ptr<Edge> _e1;
        osg::ref_ptr<Edge> _e2;
        osg::ref_ptr<Edge> _e3;
    };




    Triangle* addTriangle(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        //std::cout<<"addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;

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

        triangle->_e1 = addEdge(triangle, triangle->_p1.get(), triangle->_p2.get());
        triangle->_e2 = addEdge(triangle, triangle->_p2.get(), triangle->_p3.get());
        triangle->_e3 = addEdge(triangle, triangle->_p3.get(), triangle->_p1.get());
        
        _triangleList.push_back(triangle);
        
        return triangle;

    }
    
    Edge* addEdge(Triangle* triangle, Point* p1, Point* p2)
    {
        //std::cout<<"addEdge("<<p1<<","<<p2<<")"<<std::endl;
        osg::ref_ptr<Edge> edge = new Edge;
        if (p1<p2)
        {
            edge->_p1 = p1;
            edge->_p2 = p2;
        }
        else
        {
            edge->_p1 = p2;
            edge->_p2 = p1;
        }
        
        EdgeSet::iterator itr = _edgeSet.find(edge);
        if (itr==_edgeSet.end())
        {
            //std::cout<<"  addEdge("<<edge.get()<<")"<<std::endl;
            _edgeSet.insert(edge);
        }
        else
        {
            edge = *itr;
            //std::cout<<"  reuseEdge("<<edge.get()<<")"<<std::endl;
        }
        
        edge->addTriangle(triangle);
        
        return 0;
    }

    Point* addPoint(Triangle* triangle, unsigned int p1)
    {
        
        osg::ref_ptr<Point> point = new Point;
        point->_index = p1;

        if (_vertexList.valid() && p1<_vertexList->size())
        {
            point->_vertex = (*_vertexList)[p1];
        }


        PointSet::iterator itr = _pointSet.find(point);
        if (itr==_pointSet.end())
        {
            //std::cout<<"  addPoint("<<point.get()<<")"<<std::endl;
            _pointSet.insert(point);
        }
        else
        {
            point = *itr;
            //std::cout<<"  reusePoint("<<point.get()<<")"<<std::endl;
        }

        point->_triangles.push_back(triangle);
        
        return point.get();
    }

protected:

    typedef std::vector< osg::ref_ptr<osg::Array> > ArrayList;

    osg::Geometry*                  _geometry;
    osg::ref_ptr<osg::Vec3Array>    _vertexList;
    ArrayList                       _arrayList;
    
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
    _vertexList = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());

    CollectTriangleIndexFunctor collectTriangles;
    collectTriangles.setEdgeCollapse(this);
    
    _geometry->accept(collectTriangles);
}
 
void EdgeCollapse::copyBackToGeometry()
{
    osg::Vec3Array* vertices = new osg::Vec3Array(_pointSet.size());
    
    unsigned int pos = 0;
    for(PointSet::iterator pitr=_pointSet.begin();
        pitr!=_pointSet.end();
        ++pitr)
    {
        Point* point = const_cast<Point*>((*pitr).get());
        point->_index = pos;
        (*vertices)[pos++] = point->_vertex;
    }

    osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt(GL_TRIANGLES,_triangleList.size()*3);
    pos = 0;
    for(TriangleList::iterator titr=_triangleList.begin();
        titr!=_triangleList.end();
        ++titr)
    {
        Triangle* triangle = (*titr).get();
        (*primitives)[pos++] = triangle->_p1->_index;
        (*primitives)[pos++] = triangle->_p2->_index;
        (*primitives)[pos++] = triangle->_p3->_index;
    }
    
    _geometry->setNormalArray(0);
    //_geometry->setColorArray(0);
    _geometry->setTexCoordArray(0,0);
    _geometry->getPrimitiveSetList().clear();

    _geometry->setVertexArray(vertices);
    _geometry->addPrimitiveSet(primitives);
    
    osgUtil::SmoothingVisitor::smooth(*_geometry);
    
    osgUtil::TriStripVisitor stripper;
    stripper.stripify(*_geometry);
    
}


Simplifier::Simplifier(float sampleRatio):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _sampleRatio(sampleRatio)
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
