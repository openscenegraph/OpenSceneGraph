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
    geom.setNormalArray( normals, osg::Array::BIND_PER_VERTEX);

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
        if (p1==p2 || p2==p3 || p1==p3)
        {
            return;
        }

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
        _maxDeviationDotProduct(0.0f),
        _currentPrimitiveSetIndex(0)
    {
    }

    struct Triangle : public osg::Referenced
    {
        Triangle(unsigned int primitiveSetIndex, unsigned int p1, unsigned int p2, unsigned int p3):
            _primitiveSetIndex(primitiveSetIndex), _p1(p1), _p2(p2), _p3(p3) {}

        Triangle(const Triangle& tri):
            osg::Referenced(true),
            _primitiveSetIndex(tri._primitiveSetIndex), _p1(tri._p1), _p2(tri._p2), _p3(tri._p3) {}

        Triangle& operator = (const Triangle& tri)
        {
            _primitiveSetIndex = tri._primitiveSetIndex;
            _p1 = tri._p1;
            _p2 = tri._p2;
            _p3 = tri._p3;
            return *this;
        }

        unsigned int _primitiveSetIndex;
        unsigned int _p1;
        unsigned int _p2;
        unsigned int _p3;
    };

    typedef std::list< osg::ref_ptr<Triangle> > Triangles;

    struct ProblemVertex : public osg::Referenced
    {
        ProblemVertex(unsigned int p):
            _point(p) {}

        unsigned int _point;
        Triangles _triangles;
    };

    typedef std::vector< osg::ref_ptr<ProblemVertex> > ProblemVertexVector;
    typedef std::list< osg::ref_ptr<ProblemVertex> > ProblemVertexList;
    typedef std::list< osg::ref_ptr<osg::Array> > ArrayList;

    bool set(osg::Geometry* geom, float creaseAngle)
    {
        _geometry = geom;
        _creaseAngle = creaseAngle;

        if (!_geometry)
        {
            OSG_NOTICE<<"Warning: SmoothTriangleIndexFunctor::set(..) requires a geometry."<<std::endl;
            return false;
        }

        _vertices = dynamic_cast<osg::Vec3Array*>(_geometry->getVertexArray());
        _normals = dynamic_cast<osg::Vec3Array*>(_geometry->getNormalArray());
        _maxDeviationDotProduct = cos(_creaseAngle*0.5);

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

        _problemVertexVector.resize(_vertices->size());

        addArray(geom->getVertexArray());
        addArray(geom->getNormalArray());
        addArray(geom->getColorArray());
        addArray(geom->getSecondaryColorArray());
        addArray(geom->getFogCoordArray());

        for(unsigned int i=0; i<geom->getNumTexCoordArrays(); ++i)
        {
            addArray(geom->getTexCoordArray(i));
        }

        return true;
    }

    void addArray(osg::Array* array)
    {
        if (array && array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            _arrays.push_back(array);
        }
    }

    void operator() (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        osg::Vec3 normal( computeNormal(p1, p2, p3) );

        if (p1==p2 || p2==p3 || p1==p3)
        {
            // OSG_NOTICE<<"NULL triangle ("<<p1<<", "<<p2<<", "<<p3<<")"<<std::endl;
            return;
        }

        Triangle* tri = new Triangle(_currentPrimitiveSetIndex, p1, p2, p3);
        _triangles.push_back(tri);

        if (checkDeviation(p1, normal)) markProblemVertex(p1);
        if (checkDeviation(p2, normal)) markProblemVertex(p2);
        if (checkDeviation(p3, normal)) markProblemVertex(p3);
    }

    void markProblemVertex(unsigned int p)
    {
        if (!_problemVertexVector[p])
        {
            _problemVertexVector[p] = new ProblemVertex(p);
            _problemVertexList.push_back(_problemVertexVector[p]);
        }
    }

    void checkTrianglesForProblemVertices()
    {
        for(Triangles::iterator itr = _triangles.begin();
            itr != _triangles.end();
            ++itr)
        {
            Triangle* tri = itr->get();
            insertTriangleIfProblemVertex(tri->_p1, tri);
            insertTriangleIfProblemVertex(tri->_p2, tri);
            insertTriangleIfProblemVertex(tri->_p3, tri);
        }
    }

    void insertTriangleIfProblemVertex(unsigned int p, Triangle* tri)
    {
        if (_problemVertexVector[p]) _problemVertexVector[p]->_triangles.push_back(tri);
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
            for(Triangles::iterator titr = pv->_triangles.begin();
                titr != pv->_triangles.end();
                ++titr)
            {
                Triangle* tri = titr->get();
                OSG_NOTICE<<"    triangle("<<tri->_p1<<", "<<tri->_p2<<", "<<tri->_p3<<")"<<std::endl;
                osg::Vec3 normal( computeNormal(tri->_p1, tri->_p2, tri->_p3) );
                float deviation = normal * (*_normals)[pv->_point];
                OSG_NOTICE<<"    deviation "<<deviation<<std::endl;
            }

        }
    }

    class DuplicateVertex : public osg::ArrayVisitor
    {
        public:

            unsigned int _i;
            unsigned int _end;

            DuplicateVertex(unsigned int i):
                                _i(i),
                                _end(i) {}

            template <class ARRAY>
            void apply_imp(ARRAY& array)
            {
                _end = array.size();
                array.push_back(array[_i]);
            }

            virtual void apply(osg::ByteArray& ba) { apply_imp(ba); }
            virtual void apply(osg::ShortArray& ba) { apply_imp(ba); }
            virtual void apply(osg::IntArray& ba) { apply_imp(ba); }
            virtual void apply(osg::UByteArray& ba) { apply_imp(ba); }
            virtual void apply(osg::UShortArray& ba) { apply_imp(ba); }
            virtual void apply(osg::UIntArray& ba) { apply_imp(ba); }
            virtual void apply(osg::Vec4ubArray& ba) { apply_imp(ba); }
            virtual void apply(osg::FloatArray& ba) { apply_imp(ba); }
            virtual void apply(osg::Vec2Array& ba) { apply_imp(ba); }
            virtual void apply(osg::Vec3Array& ba) { apply_imp(ba); }
            virtual void apply(osg::Vec4Array& ba) { apply_imp(ba); }

    };

    unsigned int duplicateVertex(unsigned int i)
    {
        DuplicateVertex duplicate(i);
        for(ArrayList::iterator aItr = _arrays.begin();
            aItr != _arrays.end();
            ++aItr)
        {
            (*aItr)->accept(duplicate);
        }
        return duplicate._end;
    }

    void duplicateProblemVertexAll(ProblemVertex* pv)
    {
        unsigned int p = pv->_point;
        Triangles::iterator titr = pv->_triangles.begin();
        ++titr;
        for(;
            titr != pv->_triangles.end();
            ++titr)
        {
            Triangle* tri = titr->get();
            unsigned int duplicated_p = duplicateVertex(p);
            if (tri->_p1==p) tri->_p1 = duplicated_p;
            if (tri->_p2==p) tri->_p2 = duplicated_p;
            if (tri->_p3==p) tri->_p3 = duplicated_p;
        }
    }

    void duplicateProblemVertex(ProblemVertex* pv)
    {
        if (pv->_triangles.size()<=2)
        {
            duplicateProblemVertexAll(pv);
        }
        else
        {
            // implement a form of greedy association based on similar orientation
            // rather than iterating through all the various permutation of triangles that might
            // provide the best fit.
            unsigned int p = pv->_point;
            Triangles::iterator titr = pv->_triangles.begin();
            while(titr != pv->_triangles.end())
            {
                Triangle* tri = titr->get();
                osg::Vec3 normal = computeNormal(tri->_p1, tri->_p2, tri->_p3);

                Triangles associatedTriangles;
                associatedTriangles.push_back(tri);

                // remove triangle for list
                pv->_triangles.erase(titr);

                // reset iterator
                titr = pv->_triangles.begin();
                while(titr != pv->_triangles.end())
                {
                    Triangle* tri2 = titr->get();
                    osg::Vec3 normal2 = computeNormal(tri2->_p1, tri2->_p2, tri2->_p3);
                    float deviation = normal * normal2;
                    if (deviation >= _maxDeviationDotProduct)
                    {
                        // Tri and tri2 are close enough together to associate.
                        associatedTriangles.push_back(tri2);

                        Triangles::iterator titr_to_erase = titr;

                        ++titr;
                        pv->_triangles.erase(titr_to_erase);
                    }
                    else
                    {
                        ++titr;
                    }
                }

                // create duplicate vertex to set of associated triangles
                unsigned int duplicated_p = duplicateVertex(p);

                // now rest the index on th triangles of the point that was duplicated
                for(Triangles::iterator aitr = associatedTriangles.begin();
                    aitr != associatedTriangles.end();
                    ++aitr)
                {
                    Triangle* tri = aitr->get();
                    if (tri->_p1==p) tri->_p1 = duplicated_p;
                    if (tri->_p2==p) tri->_p2 = duplicated_p;
                    if (tri->_p3==p) tri->_p3 = duplicated_p;
                }

                // reset iterator to beginning
                titr = pv->_triangles.begin();
            }

        }
    }

    void duplicateProblemVertices()
    {
        checkTrianglesForProblemVertices();

        for(ProblemVertexList::iterator itr = _problemVertexList.begin();
            itr != _problemVertexList.end();
            ++itr)
        {
            ProblemVertex* pv = itr->get();
            if (pv->_triangles.size()>1)
            {
                duplicateProblemVertex(itr->get());
            }
        }
    }

    osg::PrimitiveSet* createPrimitiveSet(Triangles& triangles)
    {
        osg::ref_ptr<osg::DrawElements> elements = (_vertices->size()<16384) ?
            static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_TRIANGLES)) :
            static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_TRIANGLES));

        elements->reserveElements(triangles.size()*3);

        for(Triangles::iterator itr = triangles.begin();
            itr != triangles.end();
            ++itr)
        {
            Triangle* tri = itr->get();
            elements->addElement(tri->_p1);
            elements->addElement(tri->_p2);
            elements->addElement(tri->_p3);
        }

        return elements.release();
    }

    void updateGeometry()
    {
        duplicateProblemVertices();


        typedef std::map<unsigned int, Triangles> TriangleMap;
        TriangleMap triangleMap;
        for(Triangles::iterator itr = _triangles.begin();
            itr != _triangles.end();
            ++itr)
        {
            Triangle* tri = itr->get();
            triangleMap[tri->_primitiveSetIndex].push_back(tri);
        }

        for(TriangleMap::iterator itr = triangleMap.begin();
            itr != triangleMap.end();
            ++itr)
        {
            osg::PrimitiveSet* originalPrimitiveSet = _geometry->getPrimitiveSet(itr->first);
            osg::PrimitiveSet* newPrimitiveSet = createPrimitiveSet(itr->second);
            newPrimitiveSet->setName(originalPrimitiveSet->getName());
            _geometry->setPrimitiveSet(itr->first, newPrimitiveSet);
        }
    }


    osg::Geometry*      _geometry;
    osg::Vec3Array*     _vertices;
    osg::Vec3Array*     _normals;
    ArrayList           _arrays;
    float               _creaseAngle;
    float               _maxDeviationDotProduct;
    ProblemVertexVector _problemVertexVector;
    ProblemVertexList   _problemVertexList;
    Triangles           _triangles;
    unsigned int        _currentPrimitiveSetIndex;
};


static void smooth_new(osg::Geometry& geom, double creaseAngle)
{
    OSG_INFO<<"smooth_new("<<&geom<<", "<<osg::RadiansToDegrees(creaseAngle)<<")"<<std::endl;

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
    if (!vertices) return;

    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geom.getNormalArray());
    if (!normals || (normals && normals->size() != vertices->size()))
    {
        normals = new osg::Vec3Array(vertices->size());
        geom.setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
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
    if (fsef.set(&geom, creaseAngle))
    {
        // look for normals that deviate too far

        fsef.setVertexArray(vertices->getNumElements(), static_cast<const Vec3*>(vertices->getDataPointer()));
        for(unsigned int i = 0; i < geom.getNumPrimitiveSets(); ++i)
        {
            fsef._currentPrimitiveSetIndex = i;
            geom.getPrimitiveSet(i)->accept(fsef);
        }

        // fsef.listProblemVertices();
        fsef.updateGeometry();

        osg::TriangleIndexFunctor<SmoothTriangleIndexFunctor> stif2;
        if (stif2.set(vertices, normals))
        {
            // accumulate all the normals
            geom.accept(stif);

            // normalize the normals
            stif.normalize();
        }

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
