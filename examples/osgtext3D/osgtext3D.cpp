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


float computeAngle(osg::Vec3& v1, osg::Vec3& v2, osg::Vec3& v3)
{
    osg::Vec3 v12(v1-v2);
    osg::Vec3 v32(v3-v2);
    v12.normalize();
    v32.normalize();
    float dot = v12*v32;
    float angle = acosf(dot);
    OSG_NOTICE<<"    v1="<<v1<<", v2="<<v2<<", v3="<<v3<<", dot_angle="<<osg::RadiansToDegrees(angle)<<std::endl;
    return angle;
}

void computeBoundaryAngles(osg::Vec3Array& vertices, unsigned int start, unsigned int count)
{
    OSG_NOTICE<<"computeBoundaryAngles("<<vertices.size()<<", "<<start<<", "<<count<<")"<<std::endl;
    if (vertices[start+count-1]==vertices[start])
    {
        OSG_NOTICE<<"is a line loop"<<std::endl;
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
    v21.normalize();
    v32.normalize();
    osg::Vec3 cross = v21^v32;

    float t = 5.0;

    osg::Vec3 bisector(v32-v21);

    if (bisector.length()<0.5)
    {
        // angle wider than 90 degrees so use side vectors as guide for angle to project along.
        osg::Vec3 s21 = v21 ^ osg::Vec3(0.0f,0.0f,1.0f);
        s21.normalize();

        osg::Vec3 s32 = v32 ^ osg::Vec3(0.0f,0.0f,1.0f);
        s32.normalize();

        osg::Vec3 bisector(s21+s32);
        bisector.normalize();

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

        osg::Vec3 new_vertex = v2 + bisector * l;
        new_vertex.z() += 0.5f;
        return new_vertex;
    }
}

osg::PrimitiveSet* computeBevelEdge(osg::Vec3Array& orig_vertices, unsigned int start, unsigned int count, osg::Vec3Array& new_vertices)
{
    OSG_NOTICE<<"computeBoundaryAngles("<<orig_vertices.size()<<", "<<start<<", "<<count<<")"<<std::endl;
    if (orig_vertices[start+count-1]==orig_vertices[start])
    {
        OSG_NOTICE<<"is a line loop"<<std::endl;
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

osg::Geometry* computeBevelEdge(osg::Geometry* orig_geometry)
{
    OSG_NOTICE<<"computeBoundaryAngles("<<orig_geometry<<")"<<std::endl;
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
            new_primitives.push_back(computeBevelEdge(*orig_vertices, drawArray->getFirst(), drawArray->getCount(), *new_vertices));
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
    while(arguments.read("-w",word)) {}

    osg::ref_ptr<osgText::Font3D> font = osgText::readFont3DFile(fontFile);
    if (!font) return 1;
    OSG_NOTICE<<"Read font "<<fontFile<<" font="<<font.get()<<std::endl;


    bool useTessellator = false;
    while(arguments.read("-t") || arguments.read("--tessellate")) { useTessellator = true; }

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

        computeBoundaryAngles(geometry);

        osg::Geometry* bevel = computeBevelEdge(geometry);
        geode->addDrawable(bevel);

        if (useTessellator)
        {
            osgUtil::Tessellator ts;
            ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
            ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
            ts.retessellatePolygons(*geometry);

            ts.retessellatePolygons(*bevel);

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