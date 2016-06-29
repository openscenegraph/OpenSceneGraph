/* OpenSceneGraph example, osgshadermultiviewport.
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

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/ViewportIndexed>

void createShaderCompositionStateSet(osg::StateSet *stateSet) {

    stateSet->setDefine("LIGHTING");
    stateSet->setDefine("TEXTURE_2D");
    //add a global (dummy) texture, to fill in for missing textures.
    osg::Image* image = new osg::Image;
    unsigned char* imageData = new unsigned char[4];
    *((int *)(imageData)) = 0xFFFF0000;//blue
    image->setImage(1, 1, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, imageData, osg::Image::USE_NEW_DELETE, 1);
    osg::Texture2D* texture = new osg::Texture2D(image);
    stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    //handle the alpha values
    stateSet->setDefine("ALPHA_FUNC(a)", "if (a < 0.5) discard;");

    osg::ref_ptr<osg::Program> program = new osg::Program;
    stateSet->setAttribute(program.get());

    stateSet->addUniform(new osg::Uniform("texture0", 0));

    //////////////////////////
    osg::ref_ptr<osg::Shader> ts =
        new osg::Shader(osg::Shader::GEOMETRY,
            "#version 330 core\n"
            "#extension GL_ARB_gpu_shader5 : enable\n"
            "#extension GL_NV_viewport_array : enable\n"
            "layout (triangles, invocations = 2) in;\n"
            "layout (triangle_strip, max_vertices = 3) out;\n"
            "uniform mat4 transform_block[2];\n"
            "in  vec4 vbasecolor[];\n"
            "in  vec2 vtexcoord[];\n"
            "out vec4 basecolor;\n"
            "out vec2 texcoord;\n"
            "out int gl_Layer;\n"
            "void main(void) {\n"
            "    for (int i = 0; i < gl_in.length(); i++)\n"
            "    {"
            "        basecolor = vec4(1,1,1,1);"
            "        texcoord = vtexcoord[i];\n"
            "        gl_ViewportIndex = gl_InvocationID;\n"
            "        gl_Position = transform_block[gl_InvocationID] * gl_in[i].gl_Position;\n"
            "        EmitVertex();\n"
            "    }\n"
            "    EndPrimitive();\n"
            "}\n"
        );
    program->addShader(ts.get());

    osg::ref_ptr<osg::Shader> vertex_shader =
        new osg::Shader(osg::Shader::VERTEX,
            "#version 120\n"
            "#pragma import_defines ( LIGHTING )\n"
            "#ifdef LIGHTING\n"
            "void directionalLight( int lightNum, vec3 normal, inout vec4 color )"
            "{"
            "    vec3 n = normalize(gl_NormalMatrix * normal);"
            "    float NdotL = dot( n, normalize(gl_LightSource[lightNum].position.xyz) );"
            "    NdotL = max( 0.0, NdotL );"
            "    float NdotHV = dot( n, gl_LightSource[lightNum].halfVector.xyz );"
            "    NdotHV = max( 0.0, NdotHV );"
            "    color *= gl_LightSource[lightNum].ambient +"
            "             gl_LightSource[lightNum].diffuse * NdotL;"
            "}\n"
            "#endif\n"
            "varying vec2 vtexcoord;"
            "varying vec4 vbasecolor;"
            "void main(void)"
            "{"
            "    vbasecolor = gl_Color;\n"
            "#ifdef LIGHTING\n"
            "    directionalLight( 0, gl_Normal.xyz, vbasecolor);\n"
            "#endif\n"
            "    vtexcoord = gl_MultiTexCoord0.xy;"
#if 1
            "    gl_Position   = gl_ModelViewMatrix * gl_Vertex;"
#else
            "    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;"
#endif
            "}");
    program->addShader(vertex_shader.get());

    osg::ref_ptr<osg::Shader> fragment_shader =
        new osg::Shader(osg::Shader::FRAGMENT,
            "#pragma import_defines ( TEXTURE_2D )\n"
            "#pragma import_defines ( ALPHA_FUNC )\n"
            "#ifdef TEXTURE_2D\n"
            "uniform sampler2D texture0;\n"
            "varying vec2 texcoord;\n"
            "#endif\n"
            "varying vec4 basecolor;\n"
            "void main(void)\n"
            "{\n"
            "#ifdef TEXTURE_2D\n"
            "    gl_FragColor = texture2D( texture0, texcoord) * basecolor;\n"
            "#ifdef ALPHA_FUNC\n"
            "    ALPHA_FUNC(gl_FragColor.a)\n"
            "#endif    \n"
            "#else\n"
            "    gl_FragColor = basecolor;\n"
            "#endif\n"
            "}\n");
        program->addShader(fragment_shader.get());
}


osg::Node* createNewShaderCompositionScene(osg::ArgumentParser& arguments)
{
    osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFiles(arguments);
    if (!node) return 0;

    osg::ref_ptr<osg::Group> group = new osg::Group;
    osg::ref_ptr<osg::StateSet> stateset = group->getOrCreateStateSet();
    createShaderCompositionStateSet(stateset.get());
    group->addChild(node.get());
    return group.release();
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);


    // use new #pragma(tic) shader composition.
    osg::ref_ptr<osg::Node> scenegraph = createNewShaderCompositionScene(arguments);
    if (!scenegraph) return 1;

    osg::ref_ptr<osg::StateSet> stateSet = scenegraph->getOrCreateStateSet();

    osg::Uniform* LRuniform = stateSet->getUniform("transform_block");
    if (!LRuniform) {
        LRuniform = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "transform_block", 2);
        stateSet->addUniform(LRuniform);
    }

    viewer.setSceneData(scenegraph.get());
    viewer.realize();


    double width = viewer.getCamera()->getViewport()->width();
    double height = viewer.getCamera()->getViewport()->height();

    stateSet->setAttribute(new osg::ViewportIndexed(0, 0.0, 0.0, width * 0.5, height)); // left
    stateSet->setAttribute(new osg::ViewportIndexed(1, width * 0.5, 0.0, width * 0.5, height)); // right

    osg::Camera *cam = viewer.getCamera();
    osg::Matrix matrixL = cam->getProjectionMatrix();
    LRuniform->setElement(0, matrixL);
    double l, r, b, t, n, f;
    matrixL.getFrustum(l, r, b, t, n, f);
    osg::Matrix matrixR;
    matrixR.makeFrustum(l * 2.0, r* 2.0, b* 2.0, t* 2.0, n, f);
    LRuniform->setElement(1, matrixR);

    return viewer.run();
}
