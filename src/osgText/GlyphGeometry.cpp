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

#include "GlyphGeometry.h"

#include <osg/io_utils>
#include <osg/TriangleIndexFunctor>
#include <osg/LineWidth>
#include <osgUtil/Tessellator>
#include <osg/CullFace>

#include <limits.h>

namespace osgText
{

/////////////////////////////////////////////////////////////////////////////////////////
//
// Boundary
//
class Boundary
{
public:

    typedef std::pair<unsigned int, unsigned int> Segment;
    typedef std::vector<Segment>  Segments;
    osg::ref_ptr<const osg::Vec3Array> _vertices;
    osg::ref_ptr<const osg::DrawElementsUShort> _elements;
    Segments _segments;

    Boundary(const osg::Vec3Array* vertices, const osg::PrimitiveSet* primitiveSet)
    {
        const osg::DrawArrays* drawArrays = dynamic_cast<const osg::DrawArrays*>(primitiveSet);
        if (drawArrays)
        {
            set(vertices, drawArrays->getFirst(), drawArrays->getCount());
        }
        else
        {
            const osg::DrawElementsUShort* elements = dynamic_cast<const osg::DrawElementsUShort*>(primitiveSet);
            if (elements) set(vertices, elements);
        }
    }

    void set(const osg::Vec3Array* vertices, unsigned int start, unsigned int count)
    {
        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::POLYGON);
        for(unsigned int i=start; i<start+count; ++i)
        {
            elements->push_back(i);
        }

        set(vertices, elements);
    }

    void set(const osg::Vec3Array* vertices, const osg::DrawElementsUShort* elements)
    {
        _vertices = vertices;
        _elements = elements;

        _segments.clear();

        if (elements->empty()) return;

        _segments.reserve(elements->size()-1);
        for(unsigned int i=0; i<elements->size()-1; ++i)
        {
            _segments.push_back(Segment((*elements)[i],(*elements)[i+1]));
        }
    }

    osg::Vec3 computeRayIntersectionPoint(const osg::Vec3& a, const osg::Vec3& an, const osg::Vec3& c, const osg::Vec3& cn)
    {
        float denominator = ( cn.x() * an.y() - cn.y() * an.x());
        if (denominator==0.0f)
        {
            //OSG_NOTICE<<"computeRayIntersectionPoint()<<denominator==0.0"<<std::endl;
            // line segments must be parallel.
            return (a+c)*0.5f;
        }

        float t = ((a.x()-c.x())*an.y() - (a.y()-c.y())*an.x()) / denominator;
        return c + cn*t;
    }

    osg::Vec3 computeIntersectionPoint(const osg::Vec3& a, const osg::Vec3& b, const osg::Vec3& c, const osg::Vec3& d)
    {
        return computeRayIntersectionPoint(a, b-a, c, d-c);
    }

    osg::Vec3 computeBisectorNormal(const osg::Vec3& a, const osg::Vec3& b, const osg::Vec3& c, const osg::Vec3& d)
    {
        osg::Vec2 ab(a.x()-b.x(), a.y()-b.y());
        osg::Vec2 dc(d.x()-c.x(), d.y()-c.y());
        /*float length_ab =*/ ab.normalize();
        /*float length_dc =*/ dc.normalize();

        float e = dc.y() - ab.y();
        float f = ab.x() - dc.x();
        float denominator = sqrtf(e*e + f*f);
        float nx = e / denominator;
        float ny = f / denominator;
        if (( ab.x()*ny - ab.y()*nx) > 0.0f)
        {
            // OSG_NOTICE<<"   computeBisectorNormal(a=["<<a<<"], b=["<<b<<"], c=["<<c<<"], d=["<<d<<"]), nx="<<nx<<", ny="<<ny<<", denominator="<<denominator<<" no need to swap"<<std::endl;
            return osg::Vec3(nx,ny,0.0f);
        }
        else
        {
            OSG_INFO<<"   computeBisectorNormal(a=["<<a<<"], b=["<<b<<"], c=["<<c<<"], d=["<<d<<"]), nx="<<nx<<", ny="<<ny<<", denominator="<<denominator<<" need to swap!!!"<<std::endl;
            return osg::Vec3(-nx,-ny,0.0f);
        }
    }

    float computeBisectorIntersectorThickness(const osg::Vec3& a, const osg::Vec3& b, const osg::Vec3& c, const osg::Vec3& d, const osg::Vec3& e, const osg::Vec3& f)
    {
        osg::Vec3 intersection_abcd = computeIntersectionPoint(a,b,c,d);
        osg::Vec3 bisector_abcd = computeBisectorNormal(a,b,c,d);
        osg::Vec3 intersection_cdef = computeIntersectionPoint(c,d,e,f);
        osg::Vec3 bisector_cdef = computeBisectorNormal(c,d,e,f);
        if (bisector_abcd==bisector_cdef)
        {
            //OSG_NOTICE<<"computeBisectorIntersector(["<<a<<"], ["<<b<<"], ["<<c<<"], ["<<d<<"], ["<<e<<"], ["<<f<<"[)"<<std::endl;
            //OSG_NOTICE<<"   bisectors parallel, thickness = "<<FLT_MAX<<std::endl;
            return FLT_MAX;
        }

        osg::Vec3 bisector_intersection = computeRayIntersectionPoint(intersection_abcd,bisector_abcd, intersection_cdef, bisector_cdef);
        osg::Vec3 normal(d.y()-c.y(), c.x()-d.x(), 0.0);
        float cd_length = normal.normalize();
        if (cd_length==0)
        {
            //OSG_NOTICE<<"computeBisectorIntersector(["<<a<<"], ["<<b<<"], ["<<c<<"], ["<<d<<"], ["<<e<<"], ["<<f<<"[)"<<std::endl;
            //OSG_NOTICE<<"   segment length==0, thickness = "<<FLT_MAX<<std::endl;
            return FLT_MAX;
        }

        float thickness = (bisector_intersection - c) * normal;
    #if 0
        OSG_NOTICE<<"computeBisectorIntersector(["<<a<<"], ["<<b<<"], ["<<c<<"], ["<<d<<"], ["<<e<<"], ["<<f<<"[)"<<std::endl;
        OSG_NOTICE<<"   bisector_abcd = "<<bisector_abcd<<", bisector_cdef="<<bisector_cdef<<std::endl;
        OSG_NOTICE<<"   bisector_intersection = "<<bisector_intersection<<", thickness = "<<thickness<<std::endl;
    #endif
        return thickness;
    }


    float computeThickness(unsigned int i)
    {
        Segment& seg_before = _segments[ (i+_segments.size()-1) % _segments.size() ];
        Segment& seg_target = _segments[ (i) % _segments.size() ];
        Segment& seg_after =  _segments[ (i+1) % _segments.size() ];
        return computeBisectorIntersectorThickness(
            (*_vertices)[seg_before.first], (*_vertices)[seg_before.second],
            (*_vertices)[seg_target.first], (*_vertices)[seg_target.second],
            (*_vertices)[seg_after.first], (*_vertices)[seg_after.second]);
    }

    void computeAllThickness()
    {
        for(unsigned int i=0; i<_segments.size(); ++i)
        {
            computeThickness(i);
        }
    }


    bool findMinThickness(unsigned int& minThickness_i, float& minThickness)
    {
        minThickness_i = _segments.size();
        for(unsigned int i=0; i<_segments.size(); ++i)
        {
            float thickness = computeThickness(i);
            if (thickness>0.0 && thickness <  minThickness)
            {
                minThickness = thickness;
                minThickness_i = i;
            }
        }

        return minThickness_i != _segments.size();
    }

    void removeAllSegmentsBelowThickness(float targetThickness)
    {
        // OSG_NOTICE<<"removeAllSegmentsBelowThickness("<<targetThickness<<")"<<std::endl;
        for(;;)
        {
            unsigned int minThickness_i = _segments.size();
            float minThickness = targetThickness;
            if (!findMinThickness(minThickness_i,minThickness)) break;

            // OSG_NOTICE<<"  removing segment _segments["<<minThickness_i<<"] ("<<_segments[minThickness_i].first<<", "<<_segments[minThickness_i].second<<" with thickness="<<minThickness<<" "<<std::endl;
            _segments.erase(_segments.begin()+minThickness_i);
        }
    }

    bool findMaxThickness(unsigned int& maxThickness_i, float& maxThickness)
    {
        maxThickness_i = _segments.size();
        for(unsigned int i=0; i<_segments.size(); ++i)
        {
            float thickness = computeThickness(i);
            if (thickness<0.0 && thickness >  maxThickness)
            {
                maxThickness = thickness;
                maxThickness_i = i;
            }
        }

        return maxThickness_i != _segments.size();
    }


    void removeAllSegmentsAboveThickness(float targetThickness)
    {
        // OSG_NOTICE<<"removeAllSegmentsBelowThickness("<<targetThickness<<")"<<std::endl;
        for(;;)
        {
            unsigned int maxThickness_i = _segments.size();
            float maxThickness = targetThickness;
            if (!findMaxThickness(maxThickness_i,maxThickness)) break;

            // OSG_NOTICE<<"  removing segment _segments["<<minThickness_i<<"] ("<<_segments[minThickness_i].first<<", "<<_segments[minThickness_i].second<<" with thickness="<<minThickness<<" "<<std::endl;
            _segments.erase(_segments.begin()+maxThickness_i);
        }
    }

    osg::Vec3 computeBisectorPoint(unsigned int i, float targetThickness)
    {
        Segment& seg_before = _segments[ (i+_segments.size()-1) % _segments.size() ];
        Segment& seg_target = _segments[ (i) % _segments.size() ];
        const osg::Vec3& a = (*_vertices)[seg_before.first];
        const osg::Vec3& b = (*_vertices)[seg_before.second];
        const osg::Vec3& c = (*_vertices)[seg_target.first];
        const osg::Vec3& d = (*_vertices)[seg_target.second];
        osg::Vec3 intersection_abcd = computeIntersectionPoint(a,b,c,d);
        osg::Vec3 bisector_abcd = computeBisectorNormal(a,b,c,d);
        osg::Vec3 ab_sidevector(b.y()-a.y(), a.x()-b.x(), 0.0);
        ab_sidevector.normalize();
        float scale_factor = 1.0/ (bisector_abcd*ab_sidevector);
        osg::Vec3 new_vertex = intersection_abcd + bisector_abcd*(scale_factor*targetThickness);

        // OSG_NOTICE<<"bisector_abcd = "<<bisector_abcd<<", ab_sidevector="<<ab_sidevector<<", b-a="<<b-a<<", scale_factor="<<scale_factor<<std::endl;
        return new_vertex;
    }

    void addBoundaryToGeometry(osg::Geometry* geometry, float targetThickness, bool requireFace)
    {
        if (_segments.empty()) return;

        if (geometry->getVertexArray()==0) geometry->setVertexArray(new osg::Vec3Array);
        osg::Vec3Array* new_vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());

        // allocate the primitive set to store the face geometry
        osg::ref_ptr<osg::DrawElementsUShort> face = new osg::DrawElementsUShort(GL_POLYGON);
        face->setName("face");

        // reserve enough space in the vertex array to accomodate the vertices associated with the segments
        new_vertices->reserve(new_vertices->size() + _segments.size()+1 + _elements->size());

        // create vertices
        unsigned int previous_second = _segments[0].second;
        osg::Vec3 newPoint = computeBisectorPoint(0, targetThickness);
        unsigned int first = new_vertices->size();
        new_vertices->push_back(newPoint);

        unsigned int start = (*_elements)[0];
        unsigned int count = _elements->size();

        if (_segments[0].first != start)
        {
            //OSG_NOTICE<<"We have pruned from the start"<<std::endl;
            for(unsigned int j=start; j<=_segments[0].first;++j)
            {
                face->push_back(first);
            }
        }
        else
        {
            face->push_back(first);
        }


        for(unsigned int i=1; i<_segments.size(); ++i)
        {
            newPoint = computeBisectorPoint(i, targetThickness);
            unsigned int vi = new_vertices->size();
            new_vertices->push_back(newPoint);

            if (previous_second != _segments[i].first)
            {
                //OSG_NOTICE<<"Gap in boundary"<<previous_second<<" to "<<_segments[i].first<<std::endl;
                for(unsigned int j=previous_second; j<=_segments[i].first;++j)
                {
                    face->push_back(vi);
                }
            }
            else
            {
                face->push_back(vi);
            }

            previous_second = _segments[i].second;
        }

        // fill the end of the polygon with repititions of the first index in the polygon to ensure
        // that the orignal and new boundary polygons have the same number and pairing of indices.
        // This ensures that the bevel can be created coherently.
        while(face->size() < count)
        {
            face->push_back(first);
        }


        if (requireFace)
        {
            // add face primitive set for polygon
            geometry->addPrimitiveSet(face.get());
        }


        osg::DrawElementsUShort* bevel = new osg::DrawElementsUShort(GL_QUAD_STRIP);
        bevel->setName("bevel");
        bevel->reserve(count*2);
        for(unsigned int i=0; i<count; ++i)
        {
            unsigned int vi = new_vertices->size();
            new_vertices->push_back((*_vertices)[(*_elements)[i]]);
            bevel->push_back(vi);
            bevel->push_back((*face)[i]);
        }
        geometry->addPrimitiveSet(bevel);
    }

    void newAddBoundaryToGeometry(osg::Geometry* geometry, float targetThickness, const std::string& faceName, const std::string& bevelName)
    {
        if (_segments.empty()) return;

        unsigned int start = (*_elements)[0];
        unsigned int count = _elements->size();

        if (geometry->getVertexArray()==0) geometry->setVertexArray(new osg::Vec3Array(*_vertices));
        osg::Vec3Array* new_vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());

        // allocate the primitive set to store the face geometry
        osg::ref_ptr<osg::DrawElementsUShort> face = new osg::DrawElementsUShort(GL_POLYGON);
        face->setName(faceName);

        // reserve enough space in the vertex array to accomodate the vertices associated with the segments
        new_vertices->reserve(new_vertices->size() + _segments.size()+1 + count);

        // create vertices
        unsigned int previous_second = _segments[0].second;
        osg::Vec3 newPoint = computeBisectorPoint(0, targetThickness);
        unsigned int first = new_vertices->size();
        new_vertices->push_back(newPoint);

        if (_segments[0].first != start)
        {
            //OSG_NOTICE<<"We have pruned from the start"<<std::endl;
            for(unsigned int j=start; j<=_segments[0].first;++j)
            {
                face->push_back(first);
            }
        }
        else
        {
            face->push_back(first);
        }


        for(unsigned int i=1; i<_segments.size(); ++i)
        {
            newPoint = computeBisectorPoint(i, targetThickness);
            unsigned int vi = new_vertices->size();
            new_vertices->push_back(newPoint);

            if (previous_second != _segments[i].first)
            {
                //OSG_NOTICE<<"Gap in boundary"<<previous_second<<" to "<<_segments[i].first<<std::endl;
                for(unsigned int j=previous_second; j<=_segments[i].first;++j)
                {
                    face->push_back(vi);
                }
            }
            else
            {
                face->push_back(vi);
            }

            previous_second = _segments[i].second;
        }

        // fill the end of the polygon with repititions of the first index in the polygon to ensure
        // that the orignal and new boundary polygons have the same number and pairing of indices.
        // This ensures that the bevel can be created coherently.
        while(face->size() < count)
        {
            face->push_back(first);
        }

        if (!faceName.empty())
        {
            // add face primitive set for polygon
            geometry->addPrimitiveSet(face.get());
        }

        osg::DrawElementsUShort* bevel = new osg::DrawElementsUShort(GL_QUAD_STRIP);
        bevel->setName(bevelName);
        bevel->reserve(count*2);
        for(unsigned int i=0; i<count; ++i)
        {
            bevel->push_back((*_elements)[i]);
            bevel->push_back((*face)[i]);
        }
        geometry->addPrimitiveSet(bevel);
    }
};


/////////////////////////////////////////////////////////////////////////////////////////
//
// computeGlyphGeometry
//
struct CollectTriangleIndicesFunctor
{
    CollectTriangleIndicesFunctor() {}

    typedef std::vector<unsigned int> Indices;
    Indices _indices;

    void operator() (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        if (p1==p2 || p2==p3 || p1==p3)
        {
            return;
        }

        _indices.push_back(p1);
        _indices.push_back(p3);
        _indices.push_back(p2);

    }
};


OSGTEXT_EXPORT osg::Geometry* computeGlyphGeometry(const osgText::Glyph3D* glyph, float bevelThickness, float shellThickness)
{
    const osg::Vec3Array* orig_vertices = glyph->getRawVertexArray();
    const osg::Geometry::PrimitiveSetList& orig_primitives = glyph->getRawFacePrimitiveSetList();

    osg::ref_ptr<osg::Geometry> new_geometry = new osg::Geometry;

    for(osg::Geometry::PrimitiveSetList::const_iterator itr = orig_primitives.begin();
        itr != orig_primitives.end();
        ++itr)
    {
        if ((*itr)->getMode()==GL_POLYGON)
        {
            Boundary boundaryInner(orig_vertices, itr->get());
            boundaryInner.removeAllSegmentsBelowThickness(bevelThickness);
            boundaryInner.newAddBoundaryToGeometry(new_geometry.get(), bevelThickness, "face", "bevel");

            Boundary boundaryOuter(orig_vertices, itr->get());
            boundaryOuter.removeAllSegmentsAboveThickness(-shellThickness);
            boundaryOuter.newAddBoundaryToGeometry(new_geometry.get(), -shellThickness, "", "shell");
        }

    }

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(new_geometry->getVertexArray());

    // need to tessellate the inner boundary
    {
        osg::ref_ptr<osg::Geometry> face_geometry = new osg::Geometry;
        face_geometry->setVertexArray(vertices);

        osg::CopyOp copyop(osg::CopyOp::DEEP_COPY_ALL);

        osg::Geometry::PrimitiveSetList primitiveSets;

        for(osg::Geometry::PrimitiveSetList::iterator itr = new_geometry->getPrimitiveSetList().begin();
            itr != new_geometry->getPrimitiveSetList().end();
            ++itr)
        {
            osg::PrimitiveSet* prim = itr->get();
            if (prim->getName()=="face")  face_geometry->addPrimitiveSet(copyop(itr->get()));
            else primitiveSets.push_back(prim);
        }

        osgUtil::Tessellator ts;
        ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
        ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        ts.retessellatePolygons(*face_geometry);

        osg::TriangleIndexFunctor<CollectTriangleIndicesFunctor> ctif;
        face_geometry->accept(ctif);
        CollectTriangleIndicesFunctor::Indices& indices = ctif._indices;

        // remove the previous primitive sets
        new_geometry->getPrimitiveSetList().clear();

        // create a front face using triangle indices
        osg::DrawElementsUShort* front_face = new osg::DrawElementsUShort(GL_TRIANGLES);
        front_face->setName("face");
        new_geometry->addPrimitiveSet(front_face);
        for(unsigned int i=0; i<indices.size();++i)
        {
            front_face->push_back(indices[i]);
        }

        for(osg::Geometry::PrimitiveSetList::iterator itr = primitiveSets.begin();
            itr != primitiveSets.end();
            ++itr)
        {
            osg::PrimitiveSet* prim = itr->get();
            if (prim->getName()!="face")  new_geometry->addPrimitiveSet(prim);
        }
    }

    return new_geometry.release();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// computeTextGeometry
//
OSGTEXT_EXPORT osg::Geometry* computeTextGeometry(const osgText::Glyph3D* glyph, float width)
{
    const osg::Vec3Array* orig_vertices = glyph->getRawVertexArray();
    const osg::Geometry::PrimitiveSetList& orig_primitives = glyph->getRawFacePrimitiveSetList();

    osg::ref_ptr<osg::Geometry> text_geometry = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array((*orig_vertices));

    text_geometry->setVertexArray(vertices.get());
    text_geometry->setPrimitiveSetList(orig_primitives);

    osgUtil::Tessellator ts;
    ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
    ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    ts.retessellatePolygons(*text_geometry);

    osg::TriangleIndexFunctor<CollectTriangleIndicesFunctor> ctif;
    text_geometry->accept(ctif);
    CollectTriangleIndicesFunctor::Indices& indices = ctif._indices;

    // remove the previous primitive sets
    text_geometry->getPrimitiveSetList().clear();

    if (indices.empty()) return 0;


    // create a front face using triangle indices
    osg::DrawElementsUShort* frontFace = new osg::DrawElementsUShort(GL_TRIANGLES);
    frontFace->setName("front");
    text_geometry->addPrimitiveSet(frontFace);
    for(unsigned int i=0; i<indices.size();++i)
    {
        frontFace->push_back(indices[i]);
    }

    typedef std::vector<unsigned int> Indices;
    const unsigned int NULL_VALUE = UINT_MAX;
    Indices back_indices;
    back_indices.resize(vertices->size(), NULL_VALUE);
    osg::Vec3 forward(0,0,-width);

    // build up the vertices primitives for the back face, and record the indices
    // for later use, and to ensure sharing of vertices in the face primitive set
    // the order of the triangle indices are flipped to make sure that the triangles are back face
    osg::DrawElementsUShort* backFace = new osg::DrawElementsUShort(GL_TRIANGLES);
    backFace->setName("back");
    text_geometry->addPrimitiveSet(backFace);
    for(unsigned int i=0; i<indices.size()-2;)
    {
        unsigned int p1 = indices[i++];
        unsigned int p2 = indices[i++];
        unsigned int p3 = indices[i++];
        if (back_indices[p1]==NULL_VALUE)
        {
            back_indices[p1] = vertices->size();
            vertices->push_back((*vertices)[p1]+forward);
        }

        if (back_indices[p2]==NULL_VALUE)
        {
            back_indices[p2] = vertices->size();
            vertices->push_back((*vertices)[p2]+forward);
        }

        if (back_indices[p3]==NULL_VALUE)
        {
            back_indices[p3] = vertices->size();
            vertices->push_back((*vertices)[p3]+forward);
        }

        backFace->push_back(back_indices[p1]);
        backFace->push_back(back_indices[p3]);
        backFace->push_back(back_indices[p2]);
    }

    unsigned int orig_size = orig_vertices->size();
    Indices frontedge_indices, backedge_indices;
    frontedge_indices.resize(orig_size, NULL_VALUE);
    backedge_indices.resize(orig_size, NULL_VALUE);


    for(osg::Geometry::PrimitiveSetList::const_iterator itr = orig_primitives.begin();
        itr != orig_primitives.end();
        ++itr)
    {
        osg::DrawElementsUShort* edging = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
        edging->setName("wall");
        text_geometry->addPrimitiveSet(edging);

        osg::DrawElementsUShort* elements = dynamic_cast<osg::DrawElementsUShort*>(itr->get());
        if (elements)
        {
            for(unsigned int i=0; i<elements->size(); ++i)
            {
                unsigned int ei = (*elements)[i];
                if (frontedge_indices[ei]==NULL_VALUE)
                {
                    frontedge_indices[ei] = vertices->size();
                    vertices->push_back((*orig_vertices)[ei]);
                }
                if (backedge_indices[ei]==NULL_VALUE)
                {
                    backedge_indices[ei] = vertices->size();
                    vertices->push_back((*orig_vertices)[ei]+forward);
                }

                edging->push_back(backedge_indices[ei]);
                edging->push_back(frontedge_indices[ei]);
            }
        }
    }

    return text_geometry.release();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// computeTextGeometry
//
OSGTEXT_EXPORT osg::Geometry* computeTextGeometry(osg::Geometry* glyphGeometry, const osgText::Bevel& profile, float width)
{
    osg::Vec3Array* orig_vertices = dynamic_cast<osg::Vec3Array*>(glyphGeometry->getVertexArray());
    if (!orig_vertices)
    {
        OSG_INFO<<"computeTextGeometry(..): No vertices on glyphGeometry."<<std::endl;
        return 0;
    }

    osg::ref_ptr<osg::Geometry> text_geometry = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    text_geometry->setVertexArray(vertices.get());

    typedef std::vector<unsigned int> Indices;
    const unsigned int NULL_VALUE = UINT_MAX;
    Indices front_indices, back_indices;
    front_indices.resize(orig_vertices->size(), NULL_VALUE);
    back_indices.resize(orig_vertices->size(), NULL_VALUE);

    osg::DrawElementsUShort* face = 0;
    osg::Geometry::PrimitiveSetList bevelPrimitiveSets;
    osg::Vec3 forward(0,0,-width);

    // collect bevels and face primitive sets
    for(osg::Geometry::PrimitiveSetList::iterator itr = glyphGeometry->getPrimitiveSetList().begin();
        itr != glyphGeometry->getPrimitiveSetList().end();
        ++itr)
    {
        osg::PrimitiveSet* prim = itr->get();
        if (prim->getName()=="face") face = dynamic_cast<osg::DrawElementsUShort*>(prim);
        else if (prim->getName()=="bevel") bevelPrimitiveSets.push_back(prim);
    }

    // if we don't have a face we can't create any 3d text
    if (!face) return 0;

    // face doesn't have enough vertices on it to represent a polygon.
    if (face->size()<3)
    {
        OSG_NOTICE<<"Face does not have enough elements to be able to represent a polygon, face->size() = "<<face->size()<<std::endl;
        return 0;
    }

    // build up the vertices primitives for the front face, and record the indices
    // for later use, and to ensure sharing of vertices in the face primitive set
    osg::DrawElementsUShort* frontFace = new osg::DrawElementsUShort(GL_TRIANGLES);
    frontFace->setName("front");
    text_geometry->addPrimitiveSet(frontFace);
    for(unsigned int i=0; i<face->size();)
    {
        unsigned int pi = (*face)[i++];
        if (front_indices[pi]==NULL_VALUE)
        {
            front_indices[pi] = vertices->size();
            vertices->push_back((*orig_vertices)[pi]);
        }
        frontFace->push_back(front_indices[pi]);
    }


    // build up the vertices primitives for the back face, and record the indices
    // for later use, and to ensure sharing of vertices in the face primitive set
    // the order of the triangle indices are flipped to make sure that the triangles are back face
    osg::DrawElementsUShort* backFace = new osg::DrawElementsUShort(GL_TRIANGLES);
    backFace->setName("back");
    text_geometry->addPrimitiveSet(backFace);
    for(unsigned int i=0; i<face->size()-2;)
    {
        unsigned int p1 = (*face)[i++];
        unsigned int p2 = (*face)[i++];
        unsigned int p3 = (*face)[i++];
        if (back_indices[p1]==NULL_VALUE)
        {
            back_indices[p1] = vertices->size();
            vertices->push_back((*orig_vertices)[p1]+forward);
        }

        if (back_indices[p2]==NULL_VALUE)
        {
            back_indices[p2] = vertices->size();
            vertices->push_back((*orig_vertices)[p2]+forward);
        }

        if (back_indices[p3]==NULL_VALUE)
        {
            back_indices[p3] = vertices->size();
            vertices->push_back((*orig_vertices)[p3]+forward);
        }

        backFace->push_back(back_indices[p1]);
        backFace->push_back(back_indices[p3]);
        backFace->push_back(back_indices[p2]);
    }

    bool shareVerticesWithFaces = true;

    // now build up the bevel
    for(osg::Geometry::PrimitiveSetList::iterator itr = bevelPrimitiveSets.begin();
        itr != bevelPrimitiveSets.end();
        ++itr)
    {
        osg::DrawElementsUShort* bevel = dynamic_cast<osg::DrawElementsUShort*>(itr->get());
        if (!bevel) continue;

        unsigned int no_vertices_on_boundary = bevel->size()/2;

        const osgText::Bevel::Vertices& profileVertices = profile.getVertices();
        unsigned int no_vertices_on_bevel = profileVertices.size();

        Indices bevelIndices;
        bevelIndices.resize(no_vertices_on_boundary*no_vertices_on_bevel, NULL_VALUE);

        // populate vertices
        for(unsigned int i=0; i<no_vertices_on_boundary; ++i)
        {
            unsigned int topi = (*bevel)[i*2];
            unsigned int basei = (*bevel)[i*2+1];

            osg::Vec3& top_vertex = (*orig_vertices)[ topi ];
            osg::Vec3& base_vertex = (*orig_vertices)[ basei ];
            osg::Vec3 up = top_vertex-base_vertex;

            if (shareVerticesWithFaces)
            {
                if (front_indices[basei]==NULL_VALUE)
                {
                    front_indices[basei] = vertices->size();
                    vertices->push_back(base_vertex);
                }

                bevelIndices[i*no_vertices_on_bevel + 0] = front_indices[basei];

                for(unsigned int j=1; j<no_vertices_on_bevel-1; ++j)
                {
                    const osg::Vec2& pv = profileVertices[j];
                    osg::Vec3 pos( base_vertex + (forward * pv.x()) + (up * pv.y()) );
                    bevelIndices[i*no_vertices_on_bevel + j] = vertices->size();
                    vertices->push_back(pos);
                }

                if (back_indices[basei]==NULL_VALUE)
                {
                    back_indices[basei] = vertices->size();
                    vertices->push_back(base_vertex + forward);
                }

                bevelIndices[i*no_vertices_on_bevel + no_vertices_on_bevel-1] = back_indices[basei];
            }
            else
            {
                for(unsigned int j=0; j<no_vertices_on_bevel; ++j)
                {
                    const osg::Vec2& pv = profileVertices[j];
                    osg::Vec3 pos( base_vertex + (forward * pv.x()) + (up * pv.y()) );
                    bevelIndices[i*no_vertices_on_bevel + j] = vertices->size();
                    vertices->push_back(pos);
                }
            }
        }

        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(GL_TRIANGLES);
        elements->setName("wall");
        unsigned int base, next;
        for(unsigned int i = 0; i< no_vertices_on_boundary-1; ++i)
        {
            for(unsigned int j=0; j<no_vertices_on_bevel-1; ++j)
            {
                base = i*no_vertices_on_bevel + j;
                next = base + no_vertices_on_bevel;

                elements->push_back(bevelIndices[base]);
                elements->push_back(bevelIndices[next]);
                elements->push_back(bevelIndices[base+1]);

                elements->push_back(bevelIndices[base+1]);
                elements->push_back(bevelIndices[next]);
                elements->push_back(bevelIndices[next+1]);
            }
        }

        text_geometry->addPrimitiveSet(elements);
    }

    return text_geometry.release();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// computeShellGeometry
//
OSGTEXT_EXPORT osg::Geometry* computeShellGeometry(osg::Geometry* glyphGeometry, const osgText::Bevel& profile, float width)
{
    osg::Vec3Array* orig_vertices = dynamic_cast<osg::Vec3Array*>(glyphGeometry->getVertexArray());
    if (!orig_vertices)
    {
        OSG_NOTICE<<"computeTextGeometry(..): No vertices on glyphGeometry."<<std::endl;
        return 0;
    }

    osg::ref_ptr<osg::Geometry> text_geometry = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    text_geometry->setVertexArray(vertices.get());

    typedef std::vector<unsigned int> Indices;
    const unsigned int NULL_VALUE = UINT_MAX;
    Indices front_indices, back_indices;
    front_indices.resize(orig_vertices->size(), NULL_VALUE);
    back_indices.resize(orig_vertices->size(), NULL_VALUE);

    osg::DrawElementsUShort* face = 0;
    osg::Geometry::PrimitiveSetList bevelPrimitiveSets;
    osg::Geometry::PrimitiveSetList shellPrimitiveSets;
    osg::Vec3 frontOffset(0,0,width);
    osg::Vec3 backOffset(0,0,-2.0*width);
    osg::Vec3 forward(backOffset-frontOffset);

    // collect bevels and face primitive sets
    for(osg::Geometry::PrimitiveSetList::iterator itr = glyphGeometry->getPrimitiveSetList().begin();
        itr != glyphGeometry->getPrimitiveSetList().end();
        ++itr)
    {
        osg::PrimitiveSet* prim = itr->get();
        if (prim->getName()=="face") face = dynamic_cast<osg::DrawElementsUShort*>(prim);
        else if (prim->getName()=="bevel") bevelPrimitiveSets.push_back(prim);
        else if (prim->getName()=="shell") shellPrimitiveSets.push_back(prim);
    }

    // if we don't have a face we can't create any 3d text
    if (!face) return 0;

    // build up the vertices primitives for the front face, and record the indices
    // for later use, and to ensure sharing of vertices in the face primitive set
    // the order of the triangle indices are flipped to make sure that the triangles are back face
    osg::DrawElementsUShort* frontFace = new osg::DrawElementsUShort(GL_TRIANGLES);
    text_geometry->addPrimitiveSet(frontFace);
    for(unsigned int i=0; i<face->size()-2;)
    {
        unsigned int p1 = (*face)[i++];
        unsigned int p2 = (*face)[i++];
        unsigned int p3 = (*face)[i++];
        if (front_indices[p1]==NULL_VALUE)
        {
            front_indices[p1] = vertices->size();
            vertices->push_back((*orig_vertices)[p1]+frontOffset);
        }

        if (front_indices[p2]==NULL_VALUE)
        {
            front_indices[p2] = vertices->size();
            vertices->push_back((*orig_vertices)[p2]+frontOffset);
        }

        if (front_indices[p3]==NULL_VALUE)
        {
            front_indices[p3] = vertices->size();
            vertices->push_back((*orig_vertices)[p3]+frontOffset);
        }

        frontFace->push_back(front_indices[p1]);
        frontFace->push_back(front_indices[p3]);
        frontFace->push_back(front_indices[p2]);
    }


    // build up the vertices primitives for the back face, and record the indices
    // for later use, and to ensure sharing of vertices in the face primitive set
    osg::DrawElementsUShort* backFace = new osg::DrawElementsUShort(GL_TRIANGLES);
    text_geometry->addPrimitiveSet(backFace);
    for(unsigned int i=0; i<face->size();)
    {
        unsigned int pi = (*face)[i++];
        if (back_indices[pi]==NULL_VALUE)
        {
            back_indices[pi] = vertices->size();
            vertices->push_back((*orig_vertices)[pi]+backOffset);
        }
        backFace->push_back(back_indices[pi]);
    }

    for(osg::Geometry::PrimitiveSetList::iterator itr = bevelPrimitiveSets.begin();
        itr != bevelPrimitiveSets.end();
        ++itr)
    {
        osg::DrawElementsUShort* strip = dynamic_cast<osg::DrawElementsUShort*>(itr->get());
        if (!strip) continue;

        osg::CopyOp copyop(osg::CopyOp::DEEP_COPY_ALL);

        osg::DrawElementsUShort* front_strip = dynamic_cast<osg::DrawElementsUShort*>(copyop(strip));
        text_geometry->addPrimitiveSet(front_strip);
        for(unsigned int i=0; i<front_strip->size(); ++i)
        {
            unsigned short& pi  = (*front_strip)[i];
            if (front_indices[pi]==NULL_VALUE)
            {
                front_indices[pi] = vertices->size();
                vertices->push_back((*orig_vertices)[pi]+frontOffset);
            }
            pi = front_indices[pi];
        }

        for(unsigned int i=0; i<front_strip->size()-1;)
        {
            unsigned short& p1  = (*front_strip)[i++];
            unsigned short& p2  = (*front_strip)[i++];
            std::swap(p1,p2);
        }

        osg::DrawElementsUShort* back_strip = dynamic_cast<osg::DrawElementsUShort*>(copyop(strip));
        text_geometry->addPrimitiveSet(back_strip);
        for(unsigned int i=0; i<back_strip->size(); ++i)
        {
            unsigned short& pi  = (*back_strip)[i];
            if (back_indices[pi]==NULL_VALUE)
            {
                back_indices[pi] = vertices->size();
                vertices->push_back((*orig_vertices)[pi]+backOffset);
            }
            pi = back_indices[pi];
        }
    }


    // now build up the shell
    for(osg::Geometry::PrimitiveSetList::iterator itr = shellPrimitiveSets.begin();
        itr != shellPrimitiveSets.end();
        ++itr)
    {
        osg::DrawElementsUShort* bevel = dynamic_cast<osg::DrawElementsUShort*>(itr->get());
        if (!bevel) continue;

        unsigned int no_vertices_on_boundary = bevel->size()/2;

        const osgText::Bevel::Vertices& profileVertices = profile.getVertices();
        unsigned int no_vertices_on_bevel = profileVertices.size();

        Indices bevelIndices;
        bevelIndices.resize(no_vertices_on_boundary*no_vertices_on_bevel, NULL_VALUE);

        // populate vertices
        for(unsigned int i=0; i<no_vertices_on_boundary; ++i)
        {
            unsigned int topi = (*bevel)[i*2+1];
            unsigned int basei = (*bevel)[i*2];

            osg::Vec3 top_vertex = (*orig_vertices)[ topi ] + frontOffset;
            osg::Vec3 base_vertex = (*orig_vertices)[ basei ] + frontOffset;
            osg::Vec3 up = top_vertex-base_vertex;

            if (front_indices[basei]==NULL_VALUE)
            {
                front_indices[basei] = vertices->size();
                vertices->push_back(base_vertex);
            }

            bevelIndices[i*no_vertices_on_bevel + 0] = front_indices[basei];

            for(unsigned int j=1; j<no_vertices_on_bevel-1; ++j)
            {
                const osg::Vec2& pv = profileVertices[j];
                osg::Vec3 pos( base_vertex + (forward * pv.x()) + (up * pv.y()) );
                bevelIndices[i*no_vertices_on_bevel + j] = vertices->size();
                vertices->push_back(pos);
            }

            if (back_indices[basei]==NULL_VALUE)
            {
                back_indices[basei] = vertices->size();
                vertices->push_back(base_vertex + forward);
            }

            bevelIndices[i*no_vertices_on_bevel + no_vertices_on_bevel-1] = back_indices[basei];
        }

        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(GL_TRIANGLES);
        unsigned int base, next;
        for(unsigned int i = 0; i< no_vertices_on_boundary-1; ++i)
        {
            for(unsigned int j=0; j<no_vertices_on_bevel-1; ++j)
            {
                base = i*no_vertices_on_bevel + j;
                next = base + no_vertices_on_bevel;

                elements->push_back(bevelIndices[base]);
                elements->push_back(bevelIndices[base+1]);
                elements->push_back(bevelIndices[next]);

                elements->push_back(bevelIndices[base+1]);
                elements->push_back(bevelIndices[next+1]);
                elements->push_back(bevelIndices[next]);
            }
        }

        text_geometry->addPrimitiveSet(elements);
    }

#if 1
    osg::Vec4Array* new_colours = new osg::Vec4Array;
    new_colours->push_back(osg::Vec4(1.0,1.0,1.0,0.2));
    text_geometry->setColorArray(new_colours, osg::Array::BIND_OVERALL);


    osg::StateSet* stateset = text_geometry->getOrCreateStateSet();
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateset->setAttributeAndModes(new osg::CullFace, osg::StateAttribute::ON);
    //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateset->setRenderBinDetails(11, "SORT_FRONT_TO_BACK");
#endif
    return text_geometry.release();
}

}
