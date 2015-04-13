/* OpenSceneGraph example, osgshadercomposition.
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
#include <osg/PositionAttitudeTransform>

// pull in the old shader composition implementation function
extern osg::Node* createOldShaderCompositionScene(osg::ArgumentParser& arguments);

osg::Node* createNewShaderCompositionScene(osg::ArgumentParser& arguments)
{
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFiles(arguments);
    if (!node) return 0;

    osg::ref_ptr<osg::Group> group = new osg::Group;

    osg::ref_ptr<osg::StateSet> stateset = group->getOrCreateStateSet();
    osg::ref_ptr<osg::Program> program = new osg::Program;
    stateset->setAttribute(program.get());

    osg::ref_ptr<osg::Shader> lighting_shader = osgDB::readShaderFile("shaders/lighting.vert");
    if (lighting_shader.valid())
    {
        program->addShader(lighting_shader.get());
        OSG_NOTICE<<"Adding lighting shader"<<std::endl;
    }

    osg::ref_ptr<osg::Shader> vertex_shader = osgDB::readShaderFile("shaders/osgshadercomposition.vert");
    if (vertex_shader.valid())
    {
        program->addShader(vertex_shader.get());
        OSG_NOTICE<<"Adding vertex shader"<<std::endl;
    }

    osg::ref_ptr<osg::Shader> fragment_shader = osgDB::readShaderFile("shaders/osgshadercomposition.frag");
    if (fragment_shader.valid())
    {
        program->addShader(fragment_shader.get());
        OSG_NOTICE<<"Adding fragment shader"<<std::endl;
    }

    stateset->addUniform(new osg::Uniform("texture0", 0));


    double spacing = node->getBound().radius() * 2.0;

    osg::Vec3d position(0.0,0.0,0.0);

    {
        // first subgraph, one the left, just inherit all the defaults
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->addChild(node.get());

        position.x() += spacing;

        group->addChild(pat);
    }

    {
        // second subgraph, enable lighting by passing a GL_LIGHTING defines to the shaders
        // As the lighting.vert shader has a #pragma requires(GL_LIGHTING) in the shader it
        // instructs the osg::Prorgam to link in this shader only when the GL_LIGHTING define
        // is provided.  The osgshadercomposition.vert also has a #pragma import_defines(GL_LIGHTING ..) so
        // when the GL_LIGHTING is provided it'll enable the lighting paths in the osgshadercomposition.vert
        // shader calling the lighting function per vertex
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->getOrCreateStateSet()->setDefine("LIGHTING");

        pat->addChild(node.get());

        position.x() += spacing;

        group->addChild(pat);
    }

    {
        // third subgraph, enable texturing by passing the GL_TEXTURE_2D define to the shaders.
        // Both the osgshadercomposition.vert and osgshadercomposition.frag shaders have a
        // #pragma import_defines(GL_TEXTURE_2D) so that can use this define to enable the
        // passing of texture coordinates between the vertex and framgment shaders and for
        // the fragment shader to read the texture of unit 0 (provided by the "texture0" uniform above.
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->getOrCreateStateSet()->setDefine("TEXTURE_2D");

        pat->addChild(node.get());

        position.x() += spacing;

        group->addChild(pat);
    }


    {
        // fourth subgraph, enable texturing and lighting
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->getOrCreateStateSet()->setDefine("LIGHTING");
        pat->getOrCreateStateSet()->setDefine("TEXTURE_2D");

        pat->addChild(node.get());

        position.x() += spacing;

        group->addChild(pat);
    }

    {
        // fourth subgraph, enable texturing and lighting
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->getOrCreateStateSet()->setDefine("LIGHTING");
        pat->getOrCreateStateSet()->setDefine("TEXTURE_2D");
        pat->getOrCreateStateSet()->setDefine("VERTEX_FUNC(v)", "vec4(v.x, v.y, v.z * sin(osg_SimulationTime), v.w)");

        pat->addChild(node.get());

        position.x() += spacing;

        group->addChild(pat);
    }

    group->addChild(node.get());

    return group.release();
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    if (arguments.read("--old"))
    {
        // use deprecated shader composition infrastructure
        osg::ref_ptr<osg::Node> scenegraph = createOldShaderCompositionScene(arguments);
        if (!scenegraph) return 1;

        viewer.setSceneData(scenegraph.get());

        viewer.realize();

        // enable shader composition
        osgViewer::Viewer::Windows windows;
        viewer.getWindows(windows);
        for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
            itr != windows.end();
            ++itr)
        {
            (*itr)->getState()->setShaderCompositionEnabled(true);
        }
    }
    else
    {
        // use new #pragma(tic) shader composition.
        osg::ref_ptr<osg::Node> scenegraph = createNewShaderCompositionScene(arguments);
        if (!scenegraph) return 1;

        viewer.setSceneData(scenegraph.get());
    }

    return viewer.run();
}
