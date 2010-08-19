/* OpenSceneGraph example, osgtext.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/ArgumentParser>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/TriangleIndexFunctor>
#include <osg/PositionAttitudeTransform>
#include <osgUtil/SmoothingVisitor>
#include <osgText/Font3D>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgUtil/Tessellator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/io_utils>

extern int main_orig(int, char**);
extern int main_test(int, char**);


class BevelProfile
{
    public:

        typedef std::vector<osg::Vec2> Vertices;

        BevelProfile()
        {
            flatBevel();
        }

        void flatBevel(float width=0.25f)
        {
            _vertices.clear();

            if (width>0.5f) width = 0.5f;

            _vertices.push_back(osg::Vec2(0.0f,0.0f));

            _vertices.push_back(osg::Vec2(width,1.0f));

            if (width<0.5f) _vertices.push_back(osg::Vec2(1-width,1.0f));

            _vertices.push_back(osg::Vec2(1.0f,0.0f));
        }

        void roundedBevel(float width=0.5f, unsigned int numSteps=10)
        {
            _vertices.clear();

            if (width>0.5f) width = 0.5f;

            unsigned int i = 0;
            for(; i<=numSteps; ++i)
            {
                float angle = float(osg::PI)*0.5f*(float(i)/float(numSteps));
                _vertices.push_back( osg::Vec2((1.0f-cosf(angle))*width, sinf(angle)) );
            }

            // start the second half one into the curve if the width is half way across
            i = width<0.5f ? 0 : 1;
            for(; i<=numSteps; ++i)
            {
                float angle = float(osg::PI)*0.5f*(float(numSteps-i)/float(numSteps));
                _vertices.push_back( osg::Vec2(1.0-(1.0f-cosf(angle))*width, sin(angle)) );
            }
        }

        void print(std::ostream& fout)
        {
            OSG_NOTICE<<"print bevel"<<std::endl;
            for(Vertices::iterator itr = _vertices.begin();
                itr != _vertices.end();
                ++itr)
            {
                OSG_NOTICE<<"  "<<*itr<<std::endl;
            }
        }

        Vertices& getVertices() { return _vertices; }

    protected:

        Vertices _vertices;
};

class Boundary
{
public:

    typedef std::pair<unsigned int, unsigned int> Segment;
    typedef std::vector<Segment>  Segments;
    osg::ref_ptr<osg::Vec3Array> _vertices;
    unsigned int _start;
    unsigned int _count;
    Segments _segments;

    Boundary(osg::Vec3Array* vertices, unsigned int start, unsigned int count)
    {
        _vertices = vertices;
        _start = start;
        _count = count;

        if ((*_vertices)[start]==(*_vertices)[start+count-1])
        {
            // OSG_NOTICE<<"Boundary is a line loop"<<std::endl;
        }
        else
        {
            OSG_NOTICE<<"Boundary is not a line loop"<<std::endl;
        }

        _segments.reserve(count-1);
        for(unsigned int i=start; i<start+count-2; ++i)
        {
            _segments.push_back(Segment(i,i+1));
        }
        _segments.push_back(Segment(start+count-2,start));

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
            OSG_NOTICE<<"   computeBisectorNormal(a=["<<a<<"], b=["<<b<<"], c=["<<c<<"], d=["<<d<<"]), nx="<<nx<<", ny="<<ny<<", denominator="<<denominator<<" need to swap!!!"<<std::endl;
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

    osg::Vec3 computeBisectorPoint(unsigned int i, float targetThickness)
    {
        Segment& seg_before = _segments[ (i+_segments.size()-1) % _segments.size() ];
        Segment& seg_target = _segments[ (i) % _segments.size() ];
        osg::Vec3& a = (*_vertices)[seg_before.first];
        osg::Vec3& b = (*_vertices)[seg_before.second];
        osg::Vec3& c = (*_vertices)[seg_target.first];
        osg::Vec3& d = (*_vertices)[seg_target.second];
        osg::Vec3 intersection_abcd = computeIntersectionPoint(a,b,c,d);
        osg::Vec3 bisector_abcd = computeBisectorNormal(a,b,c,d);
        osg::Vec3 ab_sidevector(b.y()-a.y(), a.x()-b.x(), 0.0);
        ab_sidevector.normalize();
        float scale_factor = 1.0/ (bisector_abcd*ab_sidevector);
        osg::Vec3 new_vertex = intersection_abcd + bisector_abcd*(scale_factor*targetThickness);

        // OSG_NOTICE<<"bisector_abcd = "<<bisector_abcd<<", ab_sidevector="<<ab_sidevector<<", b-a="<<b-a<<", scale_factor="<<scale_factor<<std::endl;
        return new_vertex;
    }

    void addBoundaryToGeometry(osg::Geometry* geometry, float targetThickness)
    {
        if (_segments.empty()) return;

        if (geometry->getVertexArray()==0) geometry->setVertexArray(new osg::Vec3Array);
        osg::Vec3Array* new_vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());

        // allocate the primitive set to store the face geometry
        osg::DrawElementsUShort* face = new osg::DrawElementsUShort(GL_POLYGON);
        face->setName("face");

        // reserve enough space in the vertex array to accomodate the vertices associated with the segments
        new_vertices->reserve(new_vertices->size() + _segments.size()+1 + _count);

        // create vertices
        unsigned int previous_second = _segments[0].second;
        osg::Vec3 newPoint = computeBisectorPoint(0, targetThickness);
        unsigned int first = new_vertices->size();
        new_vertices->push_back(newPoint);

        if (_segments[0].first != _start)
        {
            //OSG_NOTICE<<"We have pruned from the start"<<std::endl;
            for(unsigned int j=_start; j<=_segments[0].first;++j)
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
        while(face->size() < _count)
        {
            face->push_back(first);
        }

        // add face primitive set for polygon
        geometry->addPrimitiveSet(face);

        osg::DrawElementsUShort* bevel = new osg::DrawElementsUShort(GL_QUAD_STRIP);
        bevel->setName("bevel");
        bevel->reserve(_count*2);
        for(unsigned int i=0; i<_count; ++i)
        {
            unsigned int vi = new_vertices->size();
            new_vertices->push_back((*_vertices)[_start+i]);
            bevel->push_back(vi);
            bevel->push_back((*face)[i]);
        }
        geometry->addPrimitiveSet(bevel);
    }

};

osg::Geometry* getGeometryComponent(osg::Geometry* geometry, bool bevel)
{
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return 0;

    osg::Geometry* new_geometry = new osg::Geometry;
    osg::Vec3Array* new_vertices = new osg::Vec3Array(*vertices);
    new_geometry->setVertexArray(new_vertices);

    for(unsigned int i=0; i<geometry->getNumPrimitiveSets(); ++i)
    {
        osg::PrimitiveSet* primitiveSet = geometry->getPrimitiveSet(i);
        if (primitiveSet->getName()=="bevel")
        {
            if (bevel) new_geometry->addPrimitiveSet(primitiveSet);
        }
        else
        {
            if (!bevel) new_geometry->addPrimitiveSet(primitiveSet);
        }
    }

    osg::Vec4Array* new_colours = new osg::Vec4Array;
    new_colours->push_back(bevel ? osg::Vec4(1.0,1.0,0.0,1.0) : osg::Vec4(1.0,0.0,0.0,1.0));
    new_geometry->setColorArray(new_colours);
    new_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    if (!bevel)
    {
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0,0.0,1.0));
        new_geometry->setNormalArray(normals);
        new_geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
    }

    return new_geometry;
}

osg::Geometry* computeBevelGeometry(osg::Geometry* geometry, BevelProfile& profile, float width)
{
    if (!geometry) return 0;

    osg::Vec3Array* orig_vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!orig_vertices) return 0;

    if (geometry->getNumPrimitiveSets()==0) return 0;

    osg::Geometry* new_geometry = new osg::Geometry;

    osg::Vec3Array* new_vertices = new osg::Vec3Array;
    new_geometry->setVertexArray(new_vertices);

    osg::Vec3 forward(0.0f, 0.0f, -width);

    for(unsigned int prim_i=0; prim_i<geometry->getNumPrimitiveSets(); ++prim_i)
    {
        osg::DrawElementsUShort* bevel = dynamic_cast<osg::DrawElementsUShort*>(geometry->getPrimitiveSet(prim_i));
        if (!bevel) continue;

        unsigned int no_vertices_on_boundary = bevel->size()/2;

        BevelProfile::Vertices& profileVertices = profile.getVertices();
        unsigned int no_vertices_on_bevel = profileVertices.size();

        unsigned int start = new_vertices->size();

        // populate vertices
        for(unsigned int i=0; i<bevel->size()-1;)
        {
            osg::Vec3& top_vertex = (*orig_vertices)[ (*bevel)[i++] ];
            osg::Vec3& base_vertex = (*orig_vertices)[ (*bevel)[i++] ];
            osg::Vec3 up = top_vertex-base_vertex;
            for(unsigned int j=0; j<no_vertices_on_bevel; ++j)
            {
                osg::Vec2& pv = profileVertices[j];
                osg::Vec3 pos( base_vertex + (forward * pv.x()) + (up * pv.y()) );
                new_vertices->push_back(pos);
            }
        }

        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(GL_TRIANGLES);
        for(unsigned int i = 0; i< no_vertices_on_boundary-1; ++i)
        {
            for(unsigned int j=0; j<no_vertices_on_bevel-1; ++j)
            {
                unsigned int base = start + i*no_vertices_on_bevel + j;
                unsigned int next = base + no_vertices_on_bevel;

                elements->push_back(base);
                elements->push_back(next);
                elements->push_back(base+1);

                elements->push_back(base+1);
                elements->push_back(next);
                elements->push_back(next+1);
            }
        }

        new_geometry->addPrimitiveSet(elements);
    }

    return new_geometry;
}

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

osg::Geometry* computeFrontAndBackGeometry(osg::Geometry* geometry, float width)
{
    if (!geometry) return 0;

    osg::Vec3Array* orig_vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!orig_vertices) return 0;

    if (geometry->getNumPrimitiveSets()==0) return 0;

    osg::TriangleIndexFunctor<CollectTriangleIndicesFunctor> ctif;
    geometry->accept(ctif);
    CollectTriangleIndicesFunctor::Indices& face = ctif._indices;

    osg::Geometry* new_geometry = new osg::Geometry;

    osg::Vec3Array* new_vertices = new osg::Vec3Array;
    new_geometry->setVertexArray(new_vertices);

    osg::Vec3 forward(0.0f, 0.0f, -width);

    // front face
    osg::DrawElementsUShort* front_face = new osg::DrawElementsUShort(GL_TRIANGLES);
    new_geometry->addPrimitiveSet(front_face);
    for(unsigned int i=0; i<face.size();++i)
    {
        osg::Vec3& vertex = (*orig_vertices)[ face[i] ];
        front_face->push_back(new_vertices->size());
        new_vertices->push_back(vertex);
    }

    // back face
    osg::DrawElementsUShort* back_face = new osg::DrawElementsUShort(GL_TRIANGLES);
    new_geometry->addPrimitiveSet(back_face);
    for(unsigned int i=0; i<face.size();++i)
    {
        osg::Vec3 vertex = (*orig_vertices)[ face[face.size()-i-1] ] + forward;
        back_face->push_back(new_vertices->size());
        new_vertices->push_back(vertex);
    }

    return new_geometry;
}

osg::Geometry* computeThickness(osg::Geometry* orig_geometry, float thickness)
{
    // OSG_NOTICE<<"computeThickness("<<orig_geometry<<")"<<std::endl;
    osg::Vec3Array* orig_vertices = dynamic_cast<osg::Vec3Array*>(orig_geometry->getVertexArray());
    osg::Geometry::PrimitiveSetList& orig_primitives = orig_geometry->getPrimitiveSetList();

    osg::Geometry* new_geometry = new osg::Geometry;

    for(osg::Geometry::PrimitiveSetList::iterator itr = orig_primitives.begin();
        itr != orig_primitives.end();
        ++itr)
    {
        osg::DrawArrays* drawArray = dynamic_cast<osg::DrawArrays*>(itr->get());
        if (drawArray && drawArray->getMode()==GL_POLYGON)
        {
            Boundary boundary(orig_vertices, drawArray->getFirst(), drawArray->getCount());
            boundary.removeAllSegmentsBelowThickness(thickness);
            boundary.addBoundaryToGeometry(new_geometry, thickness);
        }
    }
    return new_geometry;
}

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    if (arguments.read("--test"))
    {
        return main_test(argc,argv);
    }
    else if (arguments.read("--original") || arguments.read("--orig"))
    {
        return main_orig(argc,argv);
    }

    std::string fontFile("arial.ttf");
    while(arguments.read("-f",fontFile)) {}

    std::string word("This is a simple test");

    while(arguments.read("--ascii"))
    {
        word.clear();
        for(unsigned int c=' '; c<=127;++c)
        {
            word.push_back(c);
        }
    }

    while(arguments.read("-w",word)) {}

    osg::ref_ptr<osgText::Font3D> font = osgText::readFont3DFile(fontFile);
    if (!font) return 1;
    OSG_NOTICE<<"Read font "<<fontFile<<" font="<<font.get()<<std::endl;

    bool useTessellator = true;
    while(arguments.read("-t") || arguments.read("--tessellate")) { useTessellator = true; }
    while(arguments.read("--no-tessellate")) { useTessellator = false; }

    float thickness = 5.0;
    while(arguments.read("--thickness",thickness)) {}

    float width = 20.0;
    while(arguments.read("--width",width)) {}

    float creaseAngle = 10.0f;
    while(arguments.read("--crease-angle",creaseAngle)) {}

    OSG_NOTICE<<"creaseAngle="<<creaseAngle<<std::endl;

    BevelProfile profile;
    float ratio = 0.5;
    while(arguments.read("--rounded",ratio)) { profile.roundedBevel(ratio); }
    while(arguments.read("--flat",ratio)) { profile.flatBevel(ratio); }

    profile.print(std::cout);


    osg::ref_ptr<osg::Group> group = new osg::Group;
    osg::Vec3 position;



    for(unsigned int i=0; i<word.size(); ++i)
    {
        osg::ref_ptr<osgText::Font3D::Glyph3D> glyph = font->getGlyph(word[i]);
        if (!glyph) return 1;

        osg::ref_ptr<osg::PositionAttitudeTransform> transform = new osg::PositionAttitudeTransform;
        transform->setPosition(position);
        transform->setAttitude(osg::Quat(osg::inDegrees(90.0),osg::Vec3d(1.0,0.0,0.0)));

        position.x() += glyph->getHorizontalWidth();

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;

        osg::Vec3Array* vertices = glyph->getRawVertexArray();
        osg::Geometry::PrimitiveSetList& primitives = glyph->getRawFacePrimitiveSetList();

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(vertices);
        geometry->setPrimitiveSetList(primitives);
        osg::Vec4Array* colours = new osg::Vec4Array;
        colours->push_back(osg::Vec4(1.0,1.0,1.0,1.0));
        geometry->setColorArray(colours);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::ref_ptr<osg::Geometry> face_and_bevel = computeThickness(geometry, thickness);

        osg::ref_ptr<osg::Geometry> face = getGeometryComponent(face_and_bevel, false);

        if (face.valid())
        {
            osgUtil::Tessellator ts;
            ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
            ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
            ts.retessellatePolygons(*face);

            osg::ref_ptr<osg::Geometry> faces = computeFrontAndBackGeometry(face.get(), width);
            if (faces.valid()) geode->addDrawable(faces.get());
        }

        osg::ref_ptr<osg::Geometry> bevel_strip = getGeometryComponent(face_and_bevel, true);
        osg::ref_ptr<osg::Geometry> bevel = computeBevelGeometry(bevel_strip, profile, width);

        if (bevel.valid())
        {
            geode->addDrawable(bevel.get());
        }

        osgUtil::SmoothingVisitor smoother;
        smoother.setCreaseAngle(osg::DegreesToRadians(creaseAngle));
        geode->accept(smoother);

        transform->addChild(geode.get());

        group->addChild(transform.get());
    }

    std::string filename;
    if (arguments.read("-o", filename)) osgDB::writeNodeFile(*group, filename);

    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData(group.get());
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    return viewer.run();
}
