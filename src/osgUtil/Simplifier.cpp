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
    typedef std::set< osg::ref_ptr<Point>,dereference_less> PointSet;
    typedef std::vector< osg::ref_ptr<Point> >              PointList;
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
        
        osg::ref_ptr<Point> point = _originalPointList[p1];

#if 0
        point->_index = p1;

        if (_vertexList.valid() && p1<_vertexList->size())
        {
            point->_vertex = (*_vertexList)[p1];
        }
#endif

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
    ArrayList                       _arrayList;
    
    unsigned int                    _targetNumTriangles;
    EdgeSet                         _edgeSet;
    TriangleList                    _triangleList;
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

        virtual void apply(osg::UByte4Array& array)
        {
            if (_pointList.size()!=array.size()) return;
        
            for(unsigned int i=0;i<_pointList.size();++i) 
            {
                osg::UByte4& value = array[i];
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
};

void EdgeCollapse::setGeometry(osg::Geometry* geometry)
{
    _geometry = geometry;
    
    // check to see if vertex attributes indices exists, if so expand them to remove them
    if (_geometry->suitableForOptimization())
    {
        // removing coord indices
        osg::notify(osg::INFO)<<"EdgeCollapse::setGeometry(..): Removing attribute indices"<<std::endl;
        _geometry->copyToAndOptimize(*_geometry);
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

    if (_geometry->getNormalArray() && _geometry->getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)
        geometry->getNormalArray()->accept(copyArrayToPoints);
        
    if (_geometry->getColorArray() && _geometry->getColorBinding()==osg::Geometry::BIND_PER_VERTEX)
        geometry->getColorArray()->accept(copyArrayToPoints);
        
    if (_geometry->getSecondaryColorArray() && _geometry->getSecondaryColorBinding()==osg::Geometry::BIND_PER_VERTEX)
        geometry->getSecondaryColorArray()->accept(copyArrayToPoints);

    if (_geometry->getFogCoordArray() && _geometry->getFogCoordBinding()==osg::Geometry::BIND_PER_VERTEX)
        geometry->getFogCoordArray()->accept(copyArrayToPoints);

    for(unsigned int vi=0;vi<_geometry->getNumVertexAttribArrays();++vi)
    {
        if (_geometry->getVertexAttribArray(vi) &&  _geometry->getVertexAttribBinding(vi)==osg::Geometry::BIND_PER_VERTEX)
            geometry->getVertexAttribArray(vi)->accept(copyArrayToPoints);
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
        void copy(T& array)
        {
            array.resize(_pointList.size());
        
            for(unsigned int i=0;i<_pointList.size();++i) 
            {
                float val = (_pointList[i]->_attributes[_index]);
                array[i] = R (val);
            }
                
            ++_index;
        }
        
        virtual void apply(osg::Array&) {}
        virtual void apply(osg::ByteArray& array) { copy<osg::ByteArray,char>(array); }
        virtual void apply(osg::ShortArray& array) { copy<osg::ShortArray,short>(array); }
        virtual void apply(osg::IntArray& array) { copy<osg::IntArray,int>(array); }
        virtual void apply(osg::UByteArray& array) { copy<osg::UByteArray,unsigned char>(array); }
        virtual void apply(osg::UShortArray& array) { copy<osg::UShortArray,unsigned short>(array); }
        virtual void apply(osg::UIntArray& array) { copy<osg::UIntArray,unsigned int>(array); }
        virtual void apply(osg::FloatArray& array) { copy<osg::FloatArray,float>(array); }

        virtual void apply(osg::UByte4Array& array)
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
                array[i].set(attributes[_index],attributes[_index+1]);
            }
            _index += 2;
        }

        virtual void apply(osg::Vec3Array& array)
        {
            array.resize(_pointList.size());
        
            for(unsigned int i=0;i<_pointList.size();++i) 
            {
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                array[i].set(attributes[_index],attributes[_index+1],attributes[_index+2]);
            }
            _index += 3;
        }
        
        virtual void apply(osg::Vec4Array& array)
        {
            array.resize(_pointList.size());
        
            for(unsigned int i=0;i<_pointList.size();++i) 
            {
                EdgeCollapse::FloatList& attributes = _pointList[i]->_attributes;
                array[i].set(attributes[_index],attributes[_index+1],attributes[_index+2],attributes[_index+3]);
            }
            _index += 4;
        }
        
        EdgeCollapse::PointList& _pointList;
        unsigned int _index;
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

    if (_geometry->getNormalArray() && _geometry->getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)
        _geometry->getNormalArray()->accept(copyArrayToPoints);
        
    if (_geometry->getColorArray() && _geometry->getColorBinding()==osg::Geometry::BIND_PER_VERTEX)
        _geometry->getColorArray()->accept(copyArrayToPoints);
        
    if (_geometry->getSecondaryColorArray() && _geometry->getSecondaryColorBinding()==osg::Geometry::BIND_PER_VERTEX)
        _geometry->getSecondaryColorArray()->accept(copyArrayToPoints);

    if (_geometry->getFogCoordArray() && _geometry->getFogCoordBinding()==osg::Geometry::BIND_PER_VERTEX)
        _geometry->getFogCoordArray()->accept(copyArrayToPoints);

    for(unsigned int vi=0;vi<_geometry->getNumVertexAttribArrays();++vi)
    {
        if (_geometry->getVertexAttribArray(vi) &&  _geometry->getVertexAttribBinding(vi)==osg::Geometry::BIND_PER_VERTEX)
            _geometry->getVertexAttribArray(vi)->accept(copyArrayToPoints);
    }

    osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt(GL_TRIANGLES,_triangleList.size()*3);
    unsigned int pos = 0;
    for(TriangleList::iterator titr=_triangleList.begin();
        titr!=_triangleList.end();
        ++titr)
    {
        Triangle* triangle = (*titr).get();
        (*primitives)[pos++] = triangle->_p1->_index;
        (*primitives)[pos++] = triangle->_p2->_index;
        (*primitives)[pos++] = triangle->_p3->_index;
    }
    
    _geometry->getPrimitiveSetList().clear();
    _geometry->addPrimitiveSet(primitives);

#if 0    
    osgUtil::SmoothingVisitor::smooth(*_geometry);
    
    osgUtil::TriStripVisitor stripper;
    stripper.stripify(*_geometry);
#endif  
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
