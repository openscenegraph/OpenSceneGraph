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
#include <osg/PositionAttitudeTransform>
#include <osgText/Font3D>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgUtil/Tessellator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/io_utils>

extern int main_orig(int, char**);
extern int main_test(int, char**);


osg::Vec3 computeRayIntersectionPoint(const osg::Vec3& a, const osg::Vec3& an, const osg::Vec3& c, const osg::Vec3& cn)
{
    float denominator = ( cn.x() * an.y() - cn.y() * an.x());
    if (denominator==0.0)
    {
        //OSG_NOTICE<<"computeRayIntersectionPoint()<<denominator==0.0"<<std::endl;
        // line segments must be parallel.
        return (a+c)*0.5;
    }

    float t = ((a.x()-c.x())*an.y() - (a.y()-c.y())*an.x()) / denominator;
    return c + cn*t;
}

osg::Vec3 computeIntersectionPoint(const osg::Vec3& a, const osg::Vec3& b, const osg::Vec3& c, const osg::Vec3& d)
{
#if 0
    float ba_x = b.x()-a.x();
    float ba_y = b.y()-a.y();

    float dc_x = d.x()-c.x();
    float dc_y = d.y()-c.y();

    float denominator = (dc_x * ba_y - dc_y * ba_x);
    if (denominator==0.0)
    {
        // line segments must be parallel.
        return (b+c)*0.5;
    }

    float t = ((a.x()-c.x())*ba_y + (a.y()-c.y())*ba_x) / denominator;

    return c + (d-c)*t;
#endif
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

class Boundary
{
public:

    typedef std::pair<unsigned int, unsigned int> Segment;
    typedef std::vector<Segment>  Segments;
    osg::ref_ptr<osg::Vec3Array> _vertices;
    Segments _segments;

    Boundary(osg::Vec3Array* vertices, unsigned int start, unsigned int count)
    {
        _vertices = vertices;

        if ((*_vertices)[start]==(*_vertices)[start+count-1])
        {
            // OSG_NOTICE<<"Boundary is a line loop"<<std::endl;
        }
        else
        {
            OSG_NOTICE<<"Boundary is not a line loop"<<std::endl;
        }

        _segments.reserve(count-1);
        for(unsigned int i=start; i<start+count-1; ++i)
        {
            _segments.push_back(Segment(i,i+1));
        }

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

        new_vertex.z() += 0.5f;
        return new_vertex;
    }

    void addBoundaryToGeometry(osg::Geometry* geometry, float targetThickness)
    {
        if (_segments.empty()) return;

        if (geometry->getVertexArray()==0) geometry->setVertexArray(new osg::Vec3Array);
        osg::Vec3Array* new_vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());

        unsigned int first = new_vertices->size();
        unsigned int count = 0;

        // reserve enough space in the vertex array to accomodate the vertices associated with the segments
        // new_vertices->reserve(new_vertices->size()+_segments.size()+1);

        // create vertices
        unsigned int previous_second = _segments[0].second;
        osg::Vec3 newPoint = computeBisectorPoint(0, targetThickness);
        new_vertices->push_back(newPoint);
        ++count;

        for(unsigned int i=1; i<_segments.size(); ++i)
        {
            previous_second = _segments[i].second;
            newPoint = computeBisectorPoint(i, targetThickness);
            new_vertices->push_back(newPoint);
            ++count;
        }

        // repeat the first point to make it a full closed loop
        new_vertices->push_back((*new_vertices)[first]);
        ++count;

        // add DrawArrays primitive set for polygon
        if (count!=0) geometry->addPrimitiveSet(new osg::DrawArrays(GL_POLYGON, first, count));
    }

};


float computeAngle(osg::Vec3& v1, osg::Vec3& v2, osg::Vec3& v3)
{
    osg::Vec3 v12(v1-v2);
    osg::Vec3 v32(v3-v2);
    v12.normalize();
    v32.normalize();
    float dot = v12*v32;
    float angle = acosf(dot);
    return angle;
}

void computeBoundaryAngles(osg::Vec3Array& vertices, unsigned int start, unsigned int count)
{
    //OSG_NOTICE<<"computeBoundaryAngles("<<vertices.size()<<", "<<start<<", "<<count<<")"<<std::endl;
    if (vertices[start+count-1]==vertices[start])
    {
        // OSG_NOTICE<<"is a line loop"<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"is not line loop, ("<<vertices[start+count-1]<<"), ("<<vertices[start]<<")"<<std::endl;
        return;
    }

    computeAngle(vertices[start+count-2],vertices[start],vertices[start+1]);
    for(unsigned int i=start+1; i<start+count-1; ++i)
    {
        computeAngle(vertices[i-1],vertices[i],vertices[i+1]);
    }
    computeAngle(vertices[start+count-2],vertices[start],vertices[start+1]);
}

void computeBoundaryAngles(osg::Geometry* geometry)
{
    OSG_NOTICE<<"computeBoundaryAngles("<<geometry<<")"<<std::endl;
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    osg::Geometry::PrimitiveSetList& primitives = geometry->getPrimitiveSetList();
    for(osg::Geometry::PrimitiveSetList::iterator itr = primitives.begin();
        itr != primitives.end();
        ++itr)
    {
        osg::DrawArrays* drawArray = dynamic_cast<osg::DrawArrays*>(itr->get());
        if (drawArray && drawArray->getMode()==GL_POLYGON)
        {
            computeBoundaryAngles(*vertices, drawArray->getFirst(), drawArray->getCount());
        }
    }
}

osg::Vec3 computeNewVertexPosition(osg::Vec3& v1, osg::Vec3& v2, osg::Vec3& v3)
{
    double angle = computeAngle(v1,v2,v3);
    osg::Vec3 v21(v2-v1);
    osg::Vec3 v32(v3-v2);
    float length_21 = v21.normalize();
    float length_32 = v32.normalize();

    float t = 5.0;
    if (length_21==0.0)
    {
        OSG_NOTICE<<"length_21=="<<length_21<<", length_32="<<length_32<<std::endl;
        osg::Vec3 bisector = v32 ^ osg::Vec3(0.0f,0.0f,1.0f);
        bisector.normalize();
        osg::Vec3 new_vertex = v2 + bisector * t;
        return new_vertex;
    }
    else if (length_32==0.0)
    {
        OSG_NOTICE<<"length_21=="<<length_21<<", length_32="<<length_32<<std::endl;
        osg::Vec3 bisector = v21 ^ osg::Vec3(0.0f,0.0f,1.0f);
        bisector.normalize();
        osg::Vec3 new_vertex = v2 + bisector * t;
        return new_vertex;
    }

    osg::Vec3 cross = v21^v32;
    osg::Vec3 bisector(v32-v21);

#if 0
    OSG_NOTICE<<"v1=["<<v1<<"], v2=["<<v2<<"], v3=["<<v3<<"], dot_angle="<<osg::RadiansToDegrees(angle)<<std::endl;
    OSG_NOTICE<<"     computeIntersectionPoint() point "<<computeIntersectionPoint(v1,v2,v2,v3)<<std::endl;
#endif
    // OSG_NOTICE<<"     computeBisectorNormal() normal "<<computeBisectorNormal(v1,v2,v2,v3)<<std::endl;
    
    if (bisector.length()<0.5)
    {
        // angle wider than 90 degrees so use side vectors as guide for angle to project along.
        osg::Vec3 s21 = v21 ^ osg::Vec3(0.0f,0.0f,1.0f);
        s21.normalize();

        osg::Vec3 s32 = v32 ^ osg::Vec3(0.0f,0.0f,1.0f);
        s32.normalize();

        osg::Vec3 bisector(s21+s32);
        bisector.normalize();

        
        if ((computeBisectorNormal(v1,v2,v2,v3)-bisector).length2()>0.001)
        {
            OSG_NOTICE<<"     WARNING 1 bisector disagree "<<bisector<<", s21=["<<s21<<"], s32=["<<s32<<"]"<<std::endl;
        }
        else
        {
#if 0
            OSG_NOTICE<<"     bisector normal "<<bisector<<std::endl;
#endif
        }

        float l = t / sin(angle*0.5);

        osg::Vec3 new_vertex = v2 + bisector * l;
        new_vertex.z() += 0.5f;

        return new_vertex;
    }
    else
    {
        float l = t / sin(angle*0.5);

        bisector.normalize();
        if (cross.z()>0.0) bisector = -bisector;

        if ((computeBisectorNormal(v1,v2,v2,v3)-bisector).length2()>0.001)
        {
            OSG_NOTICE<<"     WARNING 2 bisector disagree "<<bisector<<std::endl;
        }
        else
        {
#if 0
            OSG_NOTICE<<"     bisector normal "<<bisector<<std::endl;
#endif
        }

        osg::Vec3 new_vertex = v2 + bisector * l;
        new_vertex.z() += 0.5f;
        return new_vertex;
    }
}

osg::DrawArrays* computeBevelEdge(osg::Vec3Array& orig_vertices, unsigned int start, unsigned int count, osg::Vec3Array& new_vertices)
{

    // OSG_NOTICE<<"computeBevelEdge("<<orig_vertices.size()<<", "<<start<<", "<<count<<")"<<std::endl;
    if (orig_vertices[start+count-1]==orig_vertices[start])
    {
        // OSG_NOTICE<<"is a line loop"<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"is not line loop, ("<<orig_vertices[start+count-1]<<"), ("<<orig_vertices[start]<<")"<<std::endl;
        return new osg::DrawArrays(GL_POLYGON, start, count);
    }

    new_vertices[start] = computeNewVertexPosition(orig_vertices[start+count-2],orig_vertices[start],orig_vertices[start+1]);
    for(unsigned int i=start+1; i<start+count-1; ++i)
    {
        new_vertices[i] = computeNewVertexPosition(orig_vertices[i-1],orig_vertices[i],orig_vertices[i+1]);
    }
    new_vertices[start+count-1] = computeNewVertexPosition(orig_vertices[start+count-2],orig_vertices[start],orig_vertices[start+1]);

    return new osg::DrawArrays(GL_POLYGON, start, count);
}

void removeLoops(osg::Vec3Array& orig_vertices, unsigned int start, unsigned int count)
{
}

osg::Geometry* computeBevelEdge(osg::Geometry* orig_geometry)
{
    // OSG_NOTICE<<"computeBoundaryAngles("<<orig_geometry<<")"<<std::endl;
    osg::Vec3Array* orig_vertices = dynamic_cast<osg::Vec3Array*>(orig_geometry->getVertexArray());
    osg::Geometry::PrimitiveSetList& orig_primitives = orig_geometry->getPrimitiveSetList();

    osg::Geometry* new_geometry = new osg::Geometry;
    osg::Vec3Array* new_vertices = new osg::Vec3Array(*orig_vertices);
    osg::Geometry::PrimitiveSetList& new_primitives = new_geometry->getPrimitiveSetList();
    new_geometry->setVertexArray(new_vertices);
    osg::Vec4Array* new_colours = new osg::Vec4Array;
    new_colours->push_back(osg::Vec4(1.0,0.0,0.0,1.0));
    new_geometry->setColorArray(new_colours);
    new_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    for(osg::Geometry::PrimitiveSetList::iterator itr = orig_primitives.begin();
        itr != orig_primitives.end();
        ++itr)
    {
        osg::DrawArrays* drawArray = dynamic_cast<osg::DrawArrays*>(itr->get());
        if (drawArray && drawArray->getMode()==GL_POLYGON)
        {
            osg::DrawArrays* new_drawArray = computeBevelEdge(*orig_vertices, drawArray->getFirst(), drawArray->getCount(), *new_vertices);
            removeLoops(*new_vertices, new_drawArray->getFirst(), new_drawArray->getCount());
            new_primitives.push_back(new_drawArray);
        }
    }
    return new_geometry;
}

osg::Geometry* computeThickness(osg::Geometry* orig_geometry, float thickness)
{
    // OSG_NOTICE<<"computeThickness("<<orig_geometry<<")"<<std::endl;
    osg::Vec3Array* orig_vertices = dynamic_cast<osg::Vec3Array*>(orig_geometry->getVertexArray());
    osg::Geometry::PrimitiveSetList& orig_primitives = orig_geometry->getPrimitiveSetList();

    osg::Geometry* new_geometry = new osg::Geometry;

    osg::Vec4Array* new_colours = new osg::Vec4Array;
    new_colours->push_back(osg::Vec4(1.0,1.0,0.0,1.0));
    new_geometry->setColorArray(new_colours);
    new_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    for(osg::Geometry::PrimitiveSetList::iterator itr = orig_primitives.begin();
        itr != orig_primitives.end();
        ++itr)
    {
        osg::DrawArrays* drawArray = dynamic_cast<osg::DrawArrays*>(itr->get());
        if (drawArray && drawArray->getMode()==GL_POLYGON)
        {
            Boundary boundary(orig_vertices, drawArray->getFirst(), drawArray->getCount());
            boundary.computeAllThickness();

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

    bool useOldBoundaryCalc = false;
    while(arguments.read("--old")) useOldBoundaryCalc = true;

    bool useTessellator = false;
    while(arguments.read("-t") || arguments.read("--tessellate")) { useTessellator = true; }

    float thickness = 5.0;
    while(arguments.read("--thickness",thickness)) {}

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

        osg::Geometry* bevel = 0;
        if (useOldBoundaryCalc)
        {
            bevel = computeBevelEdge(geometry);
       }
        else
        {
            bevel = computeThickness(geometry, thickness);
        }

        if (bevel) geode->addDrawable(bevel);

        if (useTessellator)
        {
            if (geometry)
            {
                osgUtil::Tessellator ts;
                ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
                ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
                ts.retessellatePolygons(*geometry);
            }

            if (bevel)
            {
                osgUtil::Tessellator ts;
                ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
                ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
                ts.retessellatePolygons(*bevel);
            }

        }

        geode->addDrawable(geometry.get());

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