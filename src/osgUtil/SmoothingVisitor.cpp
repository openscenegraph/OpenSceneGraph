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
#include <osg/TriangleFunctor>
#include <osg/TriangleIndexFunctor>
#include <osg/io_utils>

#include <osgUtil/SmoothingVisitor>

#include <stdio.h>
#include <list>
#include <set>


using namespace osg;
using namespace osgUtil;

namespace Smoother
{

struct LessPtr
{
    inline bool operator() (const osg::Vec3* lhs,const osg::Vec3* rhs) const
    {
        return *lhs<*rhs;
    }
};

// triangle functor.
struct SmoothTriangleFunctor
{

    osg::Vec3* _coordBase;
    osg::Vec3* _normalBase;

    typedef std::multiset<const osg::Vec3*,LessPtr> CoordinateSet;
    CoordinateSet _coordSet;

    SmoothTriangleFunctor():
         _coordBase(0),
         _normalBase(0) {}
    
    void set(osg::Vec3 *cb,int noVertices, osg::Vec3 *nb)
    {
        _coordBase=cb;
        _normalBase=nb;

        osg::Vec3* vptr = cb;
        for(int i=0;i<noVertices;++i)
        {
            _coordSet.insert(vptr++);
        }
    }

    inline void updateNormal(const osg::Vec3& normal,const osg::Vec3* vptr)
    {
        std::pair<CoordinateSet::iterator,CoordinateSet::iterator> p =
            _coordSet.equal_range(vptr);

        for(CoordinateSet::iterator itr=p.first;
            itr!=p.second;
            ++itr)
        {
            osg::Vec3* nptr = _normalBase + (*itr-_coordBase);
            (*nptr) += normal;
        }
    }

    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool treatVertexDataAsTemporary )
    {
        if (!treatVertexDataAsTemporary)
        {
            // calc orientation of triangle.
            osg::Vec3 normal = (v2-v1)^(v3-v1);
            // normal.normalize();

            updateNormal(normal,&v1);
            updateNormal(normal,&v2);
            updateNormal(normal,&v3);
        }

    }
};

static void smooth_old(osg::Geometry& geom)
{
    OSG_INFO<<"smooth_old("<<&geom<<")"<<std::endl;
    Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    Geometry::PrimitiveSetList::iterator itr;
    unsigned int numSurfacePrimitives=0;
    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        switch((*itr)->getMode())
        {
            case(PrimitiveSet::TRIANGLES):
            case(PrimitiveSet::TRIANGLE_STRIP):
            case(PrimitiveSet::TRIANGLE_FAN):
            case(PrimitiveSet::QUADS):
            case(PrimitiveSet::QUAD_STRIP):
            case(PrimitiveSet::POLYGON):
                ++numSurfacePrimitives;
                break;
            default:
                break;
        }
    }

    if (!numSurfacePrimitives) return;

    osg::Vec3Array *coords = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
    if (!coords || !coords->size()) return;

    osg::Vec3Array *normals = new osg::Vec3Array(coords->size());

    osg::Vec3Array::iterator nitr;
    for(nitr = normals->begin();
        nitr!=normals->end();
        ++nitr)
    {
        nitr->set(0.0f,0.0f,0.0f);
    }

    TriangleFunctor<SmoothTriangleFunctor> stf;
    stf.set(&(coords->front()),coords->size(),&(normals->front()));

    geom.accept(stf);

    for(nitr= normals->begin();
        nitr!=normals->end();
        ++nitr)
    {
        nitr->normalize();
    }
    geom.setNormalArray( normals );
    geom.setNormalIndices( geom.getVertexIndices() );
    geom.setNormalBinding(osg::Geometry::BIND_PER_VERTEX);


    geom.dirtyDisplayList();
}


struct SmoothTriangleIndexFunctor
{
    SmoothTriangleIndexFunctor():
        _vertices(0),
        _normals(0)
    {
    }

    bool set(osg::Vec3Array* vertices, osg::Vec3Array* normals)
    {
        _vertices = vertices;
        _normals = normals;

        if (!_vertices)
        {
            OSG_NOTICE<<"Warning: SmoothTriangleIndexFunctor::set(..) requires a valid vertex arrays."<<std::endl;
            return false;
        }

        if (!_normals)
        {
            OSG_NOTICE<<"Warning: SmoothTriangleIndexFunctor::set(..) requires a valid normal arrays."<<std::endl;
            return false;
        }

        for(osg::Vec3Array::iterator itr = _normals->begin();
            itr != _normals->end();
            ++itr)
        {
            (*itr).set(0.0f,0.0f,0.0f);
        }

        return true;
    }

    void normalize()
    {
        if (!_normals) return;

        for(osg::Vec3Array::iterator itr = _normals->begin();
            itr != _normals->end();
            ++itr)
        {
            (*itr).normalize();
        }
    }

    void operator() (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        const osg::Vec3& v1 = (*_vertices)[p1];
        const osg::Vec3& v2 = (*_vertices)[p2];
        const osg::Vec3& v3 = (*_vertices)[p3];
        osg::Vec3 normal( (v2-v1)^(v3-v1) );

        normal.normalize();

        (*_normals)[p1] += normal;
        (*_normals)[p2] += normal;
        (*_normals)[p3] += normal;
    }

    osg::Vec3Array*     _vertices;
    osg::Vec3Array*     _normals;
};



struct FindSharpEdgesFunctor
{
    FindSharpEdgesFunctor():
        _vertices(0),
        _normals(0),
        _maxDeviationDotProduct(0.0f)
    {
    }

    bool set(osg::Vec3Array* vertices, osg::Vec3Array* normals, float maxDeviationDotProduct)
    {
        _vertices = vertices;
        _normals = normals;
        _maxDeviationDotProduct = maxDeviationDotProduct;

        if (!_vertices)
        {
            OSG_NOTICE<<"Warning: SmoothTriangleIndexFunctor::set(..) requires a valid vertex arrays."<<std::endl;
            return false;
        }

        if (!_normals)
        {
            OSG_NOTICE<<"Warning: SmoothTriangleIndexFunctor::set(..) requires a valid normal arrays."<<std::endl;
            return false;
        }

        OSG_NOTICE<<" _maxDeviationDotProduct = "<<_maxDeviationDotProduct<<std::endl;

        _problemVertexVector.resize(_vertices->size());

        return true;
    }

    void operator() (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        osg::Vec3 normal( computeNormal(p1, p2, p3) );

        if (checkDeviation(p1, normal)) insertTriangle(p1, p1, p2, p3);
        if (checkDeviation(p2, normal)) insertTriangle(p2, p1, p2, p3);
        if (checkDeviation(p3, normal)) insertTriangle(p3, p1, p2, p3);
    }

    void insertTriangle(unsigned int p, unsigned int p1, unsigned int p2, unsigned int p3)
    {
        Triangle tri(p1,p2,p3);
        if (!_problemVertexVector[p])
        {
            _problemVertexVector[p] = new ProblemVertex(p);
            _problemVertexList.push_back(_problemVertexVector[p]);
        }

        _problemVertexVector[p]->_triangles.push_back(tri);
    }

    bool checkDeviation(unsigned int p, osg::Vec3& normal)
    {
        float deviation = normal * (*_normals)[p];
        return (deviation < _maxDeviationDotProduct);
    }

    osg::Vec3 computeNormal(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        const osg::Vec3& v1 = (*_vertices)[p1];
        const osg::Vec3& v2 = (*_vertices)[p2];
        const osg::Vec3& v3 = (*_vertices)[p3];
        osg::Vec3 normal( (v2-v1)^(v3-v1) );
        normal.normalize();
        return normal;
    }

    void listProblemVertices()
    {
        OSG_NOTICE<<"listProblemVertices() "<<_problemVertexList.size()<<std::endl;
        for(ProblemVertexList::iterator itr = _problemVertexList.begin();
            itr != _problemVertexList.end();
            ++itr)
        {
            ProblemVertex* pv = itr->get();
            OSG_NOTICE<<"  pv._point = "<<pv->_point<<" triangles "<<pv->_triangles.size()<<std::endl;
            for(ProblemVertex::Triangles::iterator titr = pv->_triangles.begin();
                titr != pv->_triangles.end();
                ++titr)
            {
                OSG_NOTICE<<"    triangle("<<titr->_p1<<", "<<titr->_p2<<", "<<titr->_p3<<")"<<std::endl;
                osg::Vec3 normal( computeNormal(titr->_p1, titr->_p2, titr->_p3) );
                float deviation = normal * (*_normals)[pv->_point];
                OSG_NOTICE<<"    deviation "<<deviation<<std::endl;
            }

        }
    }

    struct Triangle
    {
        Triangle(unsigned int p1, unsigned int p2, unsigned int p3):
            _p1(p1), _p2(p2), _p3(p3) {}

        Triangle(const Triangle& tri):
            _p1(tri._p1), _p2(tri._p2), _p3(tri._p3) {}

        Triangle& operator = (const Triangle& tri)
        {
            _p1 = tri._p1;
            _p2 = tri._p2;
            _p3 = tri._p3;
            return *this;
        }

        unsigned int _p1;
        unsigned int _p2;
        unsigned int _p3;
    };

    struct ProblemVertex : public osg::Referenced
    {
        ProblemVertex(unsigned int p):
            _point(p) {}

        typedef std::list<Triangle> Triangles;
        unsigned int _point;
        Triangles _triangles;
    };

    typedef std::vector< osg::ref_ptr<ProblemVertex> > ProblemVertexVector;
    typedef std::list< osg::ref_ptr<ProblemVertex> > ProblemVertexList;

    osg::Vec3Array*     _vertices;
    osg::Vec3Array*     _normals;
    float               _maxDeviationDotProduct;
    ProblemVertexVector _problemVertexVector;
    ProblemVertexList   _problemVertexList;
};


static void smooth_new(osg::Geometry& geom, double creaseAngle)
{
    OSG_NOTICE<<"smooth_new("<<&geom<<", "<<osg::RadiansToDegrees(creaseAngle)<<")"<<std::endl;

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
    if (!vertices) return;

    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geom.getNormalArray());
    if (!normals)
    {
        normals = new osg::Vec3Array(vertices->size());
        geom.setNormalArray(normals);
        geom.setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }

    osg::TriangleIndexFunctor<SmoothTriangleIndexFunctor> stif;
    if (stif.set(vertices, normals))
    {
        // accumulate all the normals
        geom.accept(stif);

        // normalize the normals
        stif.normalize();
    }

    osg::TriangleIndexFunctor<FindSharpEdgesFunctor> fsef;
    if (fsef.set(vertices, normals, cos(creaseAngle*0.5)))
    {
        // look normals that deviate too far
        geom.accept(fsef);
        fsef.listProblemVertices();
    }
}

}


SmoothingVisitor::SmoothingVisitor():
    _creaseAngle(osg::PI)
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
}

SmoothingVisitor::~SmoothingVisitor()
{
}

void SmoothingVisitor::smooth(osg::Geometry& geom, double creaseAngle)
{
    if (creaseAngle==osg::PI)
    {
        Smoother::smooth_old(geom);
    }
    else
    {
        Smoother::smooth_new(geom, creaseAngle);
    }
}


void SmoothingVisitor::apply(osg::Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) smooth(*geom, _creaseAngle);
    }
}
