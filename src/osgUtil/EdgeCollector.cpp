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

#include <osgUtil/EdgeCollector>
#include <osgUtil/ConvertVec>

#include <osg/TriangleIndexFunctor>

namespace osgUtil
{

bool EdgeCollector::Point::isBoundaryPoint() const
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

void EdgeCollector::Edge::clear()
{
    _p1 = 0;
    _p2 = 0;
    _op1 = 0;
    _op2 = 0;
    _triangles.clear();
}



bool EdgeCollector::Edge::operator < ( const Edge& rhs) const
{
    if (dereference_check_less(_p1,rhs._p1)) return true;
    if (dereference_check_less(rhs._p1,_p1)) return false;

    return dereference_check_less(_p2,rhs._p2);
}

bool EdgeCollector::Edge::operator == ( const Edge& rhs) const
{
    if (&rhs==this) return true;
    if (*this<rhs) return false;
    if (rhs<*this) return false;
    return true;
}

bool EdgeCollector::Edge::operator != ( const Edge& rhs) const
{
    if (&rhs==this) return false;
    if (*this<rhs) return true;
    if (rhs<*this) return true;
    return false;
}

void EdgeCollector::Edge::setOrderedPoints(Point* p1, Point* p2)
{
    if (dereference_check_less(p1, p2))
    {
        _p1 = _op1 = p1;
        _p2 = _op2 = p2;
    }
    else
    {
        _p1 = _op2 = p2;
        _p2 = _op1 = p1;
    }
}

void EdgeCollector::Triangle::clear()
{
    _p1 = 0;
    _p2 = 0;
    _p3 = 0;

    _op1 = 0;
    _op2 = 0;
    _op3 = 0;

    _e1 = 0;
    _e2 = 0;
    _e3 = 0;
}

bool EdgeCollector::Triangle::operator < (const Triangle& rhs) const
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


void EdgeCollector::Triangle::setOrderedPoints(Point* p1, Point* p2, Point* p3)
{
    Point* points[3];

    _op1 = points[0] = p1;
    _op2 = points[1] = p2;
    _op3 = points[2] = p3;

    // find the lowest value point in the list.
    unsigned int lowest = 0;
    if (dereference_check_less(points[1],points[lowest])) lowest = 1;
    if (dereference_check_less(points[2],points[lowest])) lowest = 2;

    _p1 = points[lowest];
    _p2 = points[(lowest+1)%3];
    _p3 = points[(lowest+2)%3];

    _plane.set(_op1->_vertex,_op2->_vertex,_op3->_vertex);
}




osg::UIntArray * EdgeCollector::Edgeloop::toIndexArray() const
{
    osg::UIntArray * indexArray = new osg::UIntArray;

    EdgeList::const_iterator it = _edgeList.begin(), end = _edgeList.end();

    for (;it != end; ++it)
        indexArray->push_back((*it)->_op1->_index);

    return indexArray;
}

EdgeCollector::Triangle* EdgeCollector::addTriangle(unsigned int p1, unsigned int p2, unsigned int p3)
{
    //OSG_NOTICE<<"addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;

    // detect if triangle is degenerate.
    if (p1==p2 || p2==p3 || p1==p3) return 0;
    if ((_originalPointList[p1]->_vertex == _originalPointList[p2]->_vertex) ||
        (_originalPointList[p2]->_vertex == _originalPointList[p3]->_vertex) ||
        (_originalPointList[p3]->_vertex == _originalPointList[p1]->_vertex)) return 0;

    Triangle* triangle = new Triangle;

    triangle->setOrderedPoints(addPoint(triangle, p1), addPoint(triangle, p2), addPoint(triangle, p3));

    triangle->_e1 = addEdge(triangle, triangle->_op1.get(), triangle->_op2.get());
    triangle->_e2 = addEdge(triangle, triangle->_op2.get(), triangle->_op3.get());
    triangle->_e3 = addEdge(triangle, triangle->_op3.get(), triangle->_op1.get());

    _triangleSet.insert(triangle);

    return triangle;
}

EdgeCollector::Triangle* EdgeCollector::addTriangle(Point* p1, Point* p2, Point* p3)
{
    // OSG_NOTICE<<"      addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;

    // detect if triangle is degenerate.
    if (p1==p2 || p2==p3 || p1==p3) return 0;
    if ((p1->_vertex == p2->_vertex) ||
        (p2->_vertex == p3->_vertex) ||
        (p3->_vertex == p1->_vertex)) return 0;

        Triangle* triangle = new Triangle;

        triangle->setOrderedPoints(addPoint(triangle, p1), addPoint(triangle, p2), addPoint(triangle, p3));

        triangle->_e1 = addEdge(triangle, triangle->_op1.get(), triangle->_op2.get());
        triangle->_e2 = addEdge(triangle, triangle->_op2.get(), triangle->_op3.get());
        triangle->_e3 = addEdge(triangle, triangle->_op3.get(), triangle->_op1.get());

        _triangleSet.insert(triangle);

        return triangle;
    }


EdgeCollector::Edge* EdgeCollector::addEdge(Triangle* triangle, Point* p1, Point* p2)
{
        // OSG_NOTICE<<"        addEdge("<<p1<<","<<p2<<")"<<std::endl;
    osg::ref_ptr<Edge> edge = new Edge;
    edge->setOrderedPoints(p1,p2);

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




EdgeCollector::Point* EdgeCollector::addPoint(Triangle* triangle, Point* point)
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

void EdgeCollector::getBoundaryEdgeList(EdgeList & el)
{
    for (EdgeSet::iterator it = _edgeSet.begin(), end = _edgeSet.end(); it != end; ++it)
    {
        if ((*it)->isBoundaryEdge()) el.push_back(*it);
    }
}

bool EdgeCollector::extractBoundaryEdgeloop(EdgeList & el, Edgeloop & edgeloop)
{
    if (el.empty()) return false;


    osg::ref_ptr<Edge> current = el.back();
    el.pop_back();

    // ** init the Edgeloop
    edgeloop._edgeList.push_back(current.get());



    bool done = false;
    while (!done)
    {
        bool found = false;
        EdgeList::iterator it = el.begin(), end = el.end();
        while (it != end && !found)
        {
            if (current->endConnected(*(it->get())))
            {
                found = true;
            }
            else
            {
                ++it;
            }
        }

        if (!found)
        {
            OSG_WARN << "extractBoundaryEdgeloop : unable to close edge loop" << std::endl;
            return false;
        }
        else
        {
            edgeloop._edgeList.push_back(it->get());
            current = it->get();
            el.erase(it);

            if (edgeloop.isClosed()) done = true;
        }
    }
    return true;
}

bool EdgeCollector::extractBoundaryEdgeloopList(EdgeList & el, EdgeloopList & edgeloopList)
{
    while (!el.empty())
    {
        osg::ref_ptr<Edgeloop> edgeloop(new Edgeloop);

        if (extractBoundaryEdgeloop(el, *edgeloop))
            edgeloopList.push_back(edgeloop);
        else
            return false;
    }
    return true;
}







struct CollectTriangleOperator
{

    CollectTriangleOperator():_ec(0) {}

    void setEdgeCollector(EdgeCollector* ec) { _ec = ec; }

    EdgeCollector* _ec;

    // for use  in the triangle functor.
    inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        _ec->addTriangle(p1,p2,p3);
    }

};


typedef osg::TriangleIndexFunctor<CollectTriangleOperator> CollectTriangleIndexFunctor;

class CopyVertexArrayToPointsVisitor : public osg::ArrayVisitor
{
    public:
        CopyVertexArrayToPointsVisitor(EdgeCollector::PointList& pointList):
            _pointList(pointList) {}

        virtual void apply(osg::Vec2Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollector::Point;
                _pointList[i]->_index = i;

                osgUtil::ConvertVec<osg::Vec2, osg::Vec3d>::convert(array[i], _pointList[i]->_vertex);
            }
        }

        virtual void apply(osg::Vec3Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollector::Point;
                _pointList[i]->_index = i;

                _pointList[i]->_vertex = array[i];
            }
        }

        virtual void apply(osg::Vec4Array& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollector::Point;
                _pointList[i]->_index = i;

                osgUtil::ConvertVec<osg::Vec4, osg::Vec3d>::convert(array[i], _pointList[i]->_vertex);
            }
        }

        virtual void apply(osg::Vec2dArray& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollector::Point;
                _pointList[i]->_index = i;

                osgUtil::ConvertVec<osg::Vec2d, osg::Vec3d>::convert(array[i], _pointList[i]->_vertex);
            }
        }

        virtual void apply(osg::Vec3dArray& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollector::Point;
                _pointList[i]->_index = i;

                _pointList[i]->_vertex = array[i];
            }
        }

        virtual void apply(osg::Vec4dArray& array)
        {
            if (_pointList.size()!=array.size()) return;

            for(unsigned int i=0;i<_pointList.size();++i)
            {
                _pointList[i] = new EdgeCollector::Point;
                _pointList[i]->_index = i;

                osgUtil::ConvertVec<osg::Vec4d, osg::Vec3d>::convert(array[i], _pointList[i]->_vertex);
            }
        }

        EdgeCollector::PointList& _pointList;

    protected:

        CopyVertexArrayToPointsVisitor& operator = (const CopyVertexArrayToPointsVisitor&) { return *this; }
};

EdgeCollector::~EdgeCollector()
{
    std::for_each(_edgeSet.begin(),_edgeSet.end(),dereference_clear());

    std::for_each(_triangleSet.begin(),_triangleSet.end(),dereference_clear());
    std::for_each(_pointSet.begin(),_pointSet.end(),dereference_clear());
    std::for_each(_originalPointList.begin(),_originalPointList.end(),dereference_clear());
}


void EdgeCollector::setGeometry(osg::Geometry* geometry)
{
    _geometry = geometry;

    unsigned int numVertices = geometry->getVertexArray()->getNumElements();

    _originalPointList.resize(numVertices);

    // copy vertices across to local point list
    CopyVertexArrayToPointsVisitor copyVertexArrayToPoints(_originalPointList);
    _geometry->getVertexArray()->accept(copyVertexArrayToPoints);

    CollectTriangleIndexFunctor collectTriangles;
    collectTriangles.setEdgeCollector(this);

    _geometry->accept(collectTriangles);
}

// ** search BoundaryEdgeloop in the geometry, extrude this loop
// **  and create primitiveSet to link original loop and extruded loop
void EdgeCollector::getEdgeloopIndexList(IndexArrayList & ial)
{
    // ** collect Boundary Edge
    EdgeList edgeList;
    getBoundaryEdgeList(edgeList);

    // ** collect Edgeloop
    EdgeloopList edgeloopList;
    if (extractBoundaryEdgeloopList(edgeList, edgeloopList) == false)
    {
        OSG_WARN << "EdgeCollector: fail to collect Edgeloop.\n\n\n" << std::endl;
        return;
    }

    // ** get IndexArray of each Edgeloop
    EdgeloopList::iterator elIt, elEnd = edgeloopList.end();
    for (elIt = edgeloopList.begin(); elIt != elEnd; ++elIt)
    {
        ial.push_back((*elIt)->toIndexArray());
    }
}

} // end of osgUtil namespace
