/* A demonstration of Tessellation Shaders in OpenScenegraph.
 *
 * Instructions:
 *   Press plus to increase tesselation and minus to decrease it.
 *   Press right arrow to increase inner tesselation and left arrow to decrease it.
 *   Press up arrow to increase outer tesselation and down arrow to decrease it.
 *
 * Original code by Philip Rideout
 * Adapted to OpenScenegraph by John Kaniarz
 * Additional work by Michael Mc Donnell
 */

#include <osg/Program>
#include <osg/PatchParameter>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

static const char* vertSource = {
"#version 400\n"
"in vec4 osg_Vertex;\n"
"out vec3 vPosition;\n"
"void main(){\n"
"    vPosition = osg_Vertex.xyz;\n"
"}\n"
};
static const char* tessControlSource = {
"#version 400\n"
"layout(vertices = 3) out;\n"
"in vec3 vPosition[];\n"
"out vec3 tcPosition[];\n"
"uniform float TessLevelInner;\n"
"uniform float TessLevelOuter;\n"
"#define ID gl_InvocationID\n"
"void main(){\n"
"    tcPosition[ID] = vPosition[ID];\n"
"    if (ID == 0) {\n"
"        gl_TessLevelInner[0] = TessLevelInner;\n"
"        gl_TessLevelOuter[0] = TessLevelOuter;\n"
"        gl_TessLevelOuter[1] = TessLevelOuter;\n"
"        gl_TessLevelOuter[2] = TessLevelOuter;\n"
"    }\n"
"}\n"
};
static const char* tessEvalSource = {
"#version 400\n"
"layout(triangles, equal_spacing, cw) in;\n"
"in vec3 tcPosition[];\n"
"out vec3 tePosition;\n"
"out vec3 tePatchDistance;\n"
"uniform mat4 osg_ProjectionMatrix;\n"
"uniform mat4 osg_ModelViewMatrix;\n"
"void main(){\n"
"    vec3 p0 = gl_TessCoord.x * tcPosition[0];\n"
"    vec3 p1 = gl_TessCoord.y * tcPosition[1];\n"
"    vec3 p2 = gl_TessCoord.z * tcPosition[2];\n"
"    tePatchDistance = gl_TessCoord;\n"
"    tePosition = normalize(p0 + p1 + p2);\n"
"    gl_Position = osg_ProjectionMatrix * osg_ModelViewMatrix * vec4(tePosition, 1);\n"
"}\n"
};
static const char* geomSource = {
"#version 400\n"
"uniform mat4 osg_ModelViewMatrix;\n"
"uniform mat3 osg_NormalMatrix;\n"
"layout(triangles) in;\n"
"layout(triangle_strip, max_vertices = 3) out;\n"
"in vec3 tePosition[3];\n"
"in vec3 tePatchDistance[3];\n"
"out vec3 gFacetNormal;\n"
"out vec3 gPatchDistance;\n"
"out vec3 gTriDistance;\n"
"out vec4 gColor;\n"
"void main(){\n"
"    vec3 A = tePosition[2] - tePosition[0];\n"
"    vec3 B = tePosition[1] - tePosition[0];\n"
"    gFacetNormal = osg_NormalMatrix * normalize(cross(A, B));\n"
"    gPatchDistance = tePatchDistance[0];\n"
"    gTriDistance = vec3(1, 0, 0);\n"
"    gColor = osg_ModelViewMatrix[0];\n"
"    gl_Position = gl_in[0].gl_Position; EmitVertex();\n"
"    gPatchDistance = tePatchDistance[1];\n"
"    gTriDistance = vec3(0, 1, 0);\n"
"    gColor = osg_ModelViewMatrix[1];\n"
"    gl_Position = gl_in[1].gl_Position; EmitVertex();\n"
"    gPatchDistance = tePatchDistance[2];\n"
"    gTriDistance = vec3(0, 0, 1);\n"
"    gColor = osg_ModelViewMatrix[2];\n"
"    gl_Position = gl_in[2].gl_Position; EmitVertex();\n"
"    EndPrimitive();\n"
"}\n"
};
static const char* fragSource = {
"#version 400\n"
"out vec4 FragColor;\n"
"in vec3 gFacetNormal;\n"
"in vec3 gTriDistance;\n"
"in vec3 gPatchDistance;\n"
"in vec4 gColor;\n"
"in float gPrimitive;\n"
"uniform vec3 LightPosition;\n"
"uniform vec3 DiffuseMaterial;\n"
"uniform vec3 AmbientMaterial;\n"
"float amplify(float d, float scale, float offset){\n"
"    d = scale * d + offset;\n"
"    d = clamp(d, 0, 1);\n"
"    d = 1 - exp2(-2*d*d);\n"
"    return d;\n"
"}\n"
"void main(){\n"
"    vec3 N = normalize(gFacetNormal);\n"
"    vec3 L = LightPosition;\n"
"    float df = abs(dot(N, L));\n"
"    vec3 color = AmbientMaterial + df * DiffuseMaterial;\n"
"    float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);\n"
"    float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);\n"
"    color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;\n"
"    FragColor = vec4(color, 1.0);\n"
"}\n"
};

osg::ref_ptr<osg::Geode> CreateIcosahedron(osg::Program* /*program*/)
{
    osg::Geode *geode=new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();
    const unsigned int Faces[] = {
        2, 1, 0,
        3, 2, 0,
        4, 3, 0,
        5, 4, 0,
        1, 5, 0,

        11, 6,  7,
        11, 7,  8,
        11, 8,  9,
        11, 9,  10,
        11, 10, 6,

        1, 2, 6,
        2, 3, 7,
        3, 4, 8,
        4, 5, 9,
        5, 1, 10,

        2,  7, 6,
        3,  8, 7,
        4,  9, 8,
        5, 10, 9,
        1, 6, 10 };
    int IndexCount = sizeof(Faces) / sizeof(Faces[0]);
    const float Verts[] = {
         0.000f,  0.000f,  1.000f,
         0.894f,  0.000f,  0.447f,
         0.276f,  0.851f,  0.447f,
        -0.724f,  0.526f,  0.447f,
        -0.724f, -0.526f,  0.447f,
         0.276f, -0.851f,  0.447f,
         0.724f,  0.526f, -0.447f,
        -0.276f,  0.851f, -0.447f,
        -0.894f,  0.000f, -0.447f,
        -0.276f, -0.851f, -0.447f,
         0.724f, -0.526f, -0.447f,
         0.000f,  0.000f, -1.000f };

    int VertexCount = sizeof(Verts)/sizeof(float);
    osg::Vec3Array* vertices = new osg::Vec3Array();
    for(int i=0;i<VertexCount;i+=3){
        vertices->push_back(osg::Vec3(Verts[i],Verts[i+1],Verts[i+2]));
    }
    geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::PATCHES,IndexCount,Faces));

    // Expand the bounding box, otherwise the geometry is clipped in front when tessellating.
    osg::BoundingBox bbox(osg::Vec3(-1.0f, -1.9f, -1.0f), osg::Vec3(1.0f, 1.0f, 1.0f));
    geometry->setInitialBound(bbox);

    geode->addDrawable(geometry);
    return geode;
}

osg::ref_ptr<osg::Program> createProgram()
{
    osg::Program *program = new osg::Program();
    program->addShader(new osg::Shader(osg::Shader::VERTEX,vertSource));
    program->addShader(new osg::Shader(osg::Shader::TESSCONTROL,tessControlSource));
    program->addShader(new osg::Shader(osg::Shader::TESSEVALUATION,tessEvalSource));
    program->addShader(new osg::Shader(osg::Shader::GEOMETRY,geomSource));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT,fragSource));
    program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    return program;
}

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    KeyboardEventHandler(osg::ref_ptr<osg::Uniform> tessInnerU, osg::ref_ptr<osg::Uniform> tessOuterU):
        _tessInnerU(tessInnerU),
        _tessOuterU(tessOuterU)
    {
        tessInnerU->get(_tessInner);
        tessOuterU->get(_tessOuter);
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& gaa)
    {
        if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN){
            switch (ea.getKey()){
                case osgGA::GUIEventAdapter::KEY_Up:
                    increaseOuterTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Down:
                    decreaseOuterTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Left:
                    decreaseInnerTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Right:
                    increaseInnerTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Plus:
                case osgGA::GUIEventAdapter::KEY_KP_Add:
                    increaseInnerTesselation();
                    increaseOuterTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Minus:
                case osgGA::GUIEventAdapter::KEY_KP_Subtract:
                    decreaseInnerTesselation();
                    decreaseOuterTesselation();
                    return true;
            }
        }
        return osgGA::GUIEventHandler::handle(ea, gaa);
    }

private:
    osg::ref_ptr<osg::Uniform> _tessInnerU;
    osg::ref_ptr<osg::Uniform> _tessOuterU;
    float _tessInner;
    float _tessOuter;

    void increaseInnerTesselation()
    {
        _tessInnerU->set(++_tessInner);
    }

    void decreaseInnerTesselation()
    {
        _tessInner = std::max(1.0f, _tessInner-1.0f);
        _tessInnerU->set(_tessInner);
    }

    void increaseOuterTesselation()
    {
        _tessOuterU->set(++_tessOuter);
    }

    void decreaseOuterTesselation()
    {
        _tessOuter = std::max(1.0f, _tessOuter-1.0f);
        _tessOuterU->set(_tessOuter);
    }
};

int main(int, char* [])
{
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100,100,800,600);
    osg::ref_ptr<osg::Program> program = createProgram();
    osg::ref_ptr<osg::Geode> geode = CreateIcosahedron(program.get());
    osg::ref_ptr<osg::Uniform> tessInnerU = new osg::Uniform("TessLevelInner", 1.0f);
    osg::ref_ptr<osg::Uniform> tessOuterU = new osg::Uniform("TessLevelOuter", 1.0f);

    osg::StateSet *state;
    state = geode->getOrCreateStateSet();
    state->addUniform(new osg::Uniform("AmbientMaterial",osg::Vec3(0.04f, 0.04f, 0.04f)));
    state->addUniform(new osg::Uniform("DiffuseMaterial",osg::Vec3(0.0f, 0.75f, 0.75f)));
    state->addUniform(new osg::Uniform("LightPosition",osg::Vec3(0.25f, 0.25f, 1.0f)));
    state->addUniform(tessInnerU.get());
    state->addUniform(tessOuterU.get());
    state->setAttribute(new osg::PatchParameter(3));
    state->setAttribute(program.get());

    // switch on the uniforms that track the modelview and projection matrices
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        osg::State *s=(*itr)->getState();
        s->setUseModelViewAndProjectionUniforms(true);
        s->setUseVertexAttributeAliasing(true);
    }

    viewer.addEventHandler(new KeyboardEventHandler(tessInnerU, tessOuterU));
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.setSceneData(geode.get());
    return viewer.run();
}

