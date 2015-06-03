/* OpenSceneGraph example, osgminimalglsl.
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

/* file:        examples/osgminimalglsl.cpp
 *
 * A minimal demo of the OpenGL Shading Language shaders using core OSG.
 *
 * blocky shader taken from osgshaders sample (Author: Mike Weiblein)
*/

#include <osgViewer/Viewer>

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Vec3>

#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

using namespace osg;

///////////////////////////////////////////////////////////////////////////
// in-line GLSL source code

static const char *blockyVertSource = {
    "// blocky.vert - an GLSL vertex shader with animation\n"
    "// the App updates uniforms \"slowly\" (eg once per frame) for animation.\n"
    "uniform float Sine;\n"
    "const vec3 LightPosition = vec3(0.0, 0.0, 4.0);\n"
    "const float BlockScale = 0.30;\n"
    "// varyings are written by vert shader, interpolated, and read by frag shader.\n"
    "varying float LightIntensity;\n"
    "varying vec2  BlockPosition;\n"
    "void main(void)\n"
    "{\n"
    "    // per-vertex diffuse lighting\n"
    "    vec4 ecPosition    = gl_ModelViewMatrix * gl_Vertex;\n"
    "    vec3 tnorm         = normalize(gl_NormalMatrix * gl_Normal);\n"
    "    vec3 lightVec      = normalize(LightPosition - vec3 (ecPosition));\n"
    "    LightIntensity     = max(dot(lightVec, tnorm), 0.0);\n"
    "    // blocks will be determined by fragment's position on the XZ plane.\n"
    "    BlockPosition  = gl_Vertex.xz / BlockScale;\n"
    "    // scale the geometry based on an animation variable.\n"
    "    vec4 vertex    = gl_Vertex;\n"
    "    vertex.w       = 1.0 + 0.4 * (Sine + 1.0);\n"
    "    gl_Position    = gl_ModelViewProjectionMatrix * vertex;\n"
    "}\n"
};

static const char *blockyFragSource = {
    "// blocky.frag - an GLSL fragment shader with animation\n"
    "// the App updates uniforms \"slowly\" (eg once per frame) for animation.\n"
    "uniform float Sine;\n"
    "const vec3 Color1 = vec3(1.0, 1.0, 1.0);\n"
    "const vec3 Color2 = vec3(0.0, 0.0, 0.0);\n"
    "// varyings are written by vert shader, interpolated, and read by frag shader.\n"
    "varying vec2  BlockPosition;\n"
    "varying float LightIntensity;\n"
    "void main(void)\n"
    "{\n"
    "    vec3 color;\n"
    "    float ss, tt, w, h;\n"
    "    ss = BlockPosition.x;\n"
    "    tt = BlockPosition.y;\n"
    "    if (fract(tt * 0.5) > 0.5)\n"
    "        ss += 0.5;\n"
    "    ss = fract(ss);\n"
    "    tt = fract(tt);\n"
    "    // animate the proportion of block to mortar\n"
    "    float blockFract = (Sine + 1.1) * 0.4;\n"
    "    w = step(ss, blockFract);\n"
    "    h = step(tt, blockFract);\n"
    "    color = mix(Color2, Color1, w * h) * LightIntensity;\n"
    "    gl_FragColor = vec4 (color, 1.0);\n"
    "}\n"
};

///////////////////////////////////////////////////////////////////////////
// callback for animating various Uniforms (currently only the SIN uniform)

class AnimateCallback: public osg::UniformCallback
{
    public:
        enum Operation { SIN };
        AnimateCallback(Operation op) : _operation(op) {}
        virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv )
        {
            float angle = 2.0 * nv->getFrameStamp()->getSimulationTime();
            float sine = sinf( angle );            // -1 -> 1
            switch(_operation) {
                case SIN : uniform->set( sine ); break;
            }
        }
    private:
        Operation _operation;
};

int main(int, char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // use a geode with a Box ShapeDrawable
    osg::Geode* basicModel = new osg::Geode();
    basicModel->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f,0.0f,0.0f),1.0f)));

    // create the "blocky" shader, a simple animation test
    osg::StateSet *ss = basicModel->getOrCreateStateSet();
    osg::Program* program = new osg::Program;
    program->setName( "blocky" );
    program->addShader( new osg::Shader( osg::Shader::VERTEX, blockyVertSource ) );
    program->addShader( new osg::Shader( osg::Shader::FRAGMENT, blockyFragSource ) );
    ss->setAttributeAndModes(program, osg::StateAttribute::ON);

    // attach some animated Uniform variable to the state set
    osg::Uniform* SineUniform   = new osg::Uniform( "Sine", 0.0f );
    ss->addUniform( SineUniform );
    SineUniform->setUpdateCallback(new AnimateCallback(AnimateCallback::SIN));

    // run the osg::Viewer using our model
    viewer.setSceneData( basicModel );
    return viewer.run();
}

/*EOF*/
