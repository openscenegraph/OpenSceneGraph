/* OpenSceneGraph example, osggameoflife.
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

#include "GameOfLifePass.h"
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <iostream>

ProcessPass::ProcessPass(osg::TextureRectangle *in_tex,
                         osg::TextureRectangle *out_tex,
                         int width, int height):
    _TextureWidth(width),
    _TextureHeight(height)
{
    _RootGroup = new osg::Group;

    _InTexture = in_tex;
    _OutTexture = out_tex;

    _Camera = new osg::Camera;
    setupCamera();
    _Camera->addChild(createTexturedQuad().get());

    _RootGroup->addChild(_Camera.get());

    setShader("shaders/gameoflife.frag");
}

ProcessPass::~ProcessPass()
{
}

osg::ref_ptr<osg::Group> ProcessPass::createTexturedQuad()
{
    osg::ref_ptr<osg::Group> top_group = new osg::Group;

    osg::ref_ptr<osg::Geode> quad_geode = new osg::Geode;

    osg::ref_ptr<osg::Vec3Array> quad_coords = new osg::Vec3Array; // vertex coords
    // counter-clockwise
    quad_coords->push_back(osg::Vec3d(0, 0, -1));
    quad_coords->push_back(osg::Vec3d(1, 0, -1));
    quad_coords->push_back(osg::Vec3d(1, 1, -1));
    quad_coords->push_back(osg::Vec3d(0, 1, -1));

    osg::ref_ptr<osg::Vec2Array> quad_tcoords = new osg::Vec2Array; // texture coords
    quad_tcoords->push_back(osg::Vec2(0, 0));
    quad_tcoords->push_back(osg::Vec2(_TextureWidth, 0));
    quad_tcoords->push_back(osg::Vec2(_TextureWidth, _TextureHeight));
    quad_tcoords->push_back(osg::Vec2(0, _TextureHeight));

    osg::ref_ptr<osg::Geometry> quad_geom = new osg::Geometry;
    osg::ref_ptr<osg::DrawArrays> quad_da = new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4);

    osg::ref_ptr<osg::Vec4Array> quad_colors = new osg::Vec4Array;
    quad_colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

    quad_geom->setVertexArray(quad_coords.get());
    quad_geom->setTexCoordArray(0, quad_tcoords.get());
    quad_geom->addPrimitiveSet(quad_da.get());
    quad_geom->setColorArray(quad_colors.get(), osg::Array::BIND_OVERALL);

    _StateSet = quad_geom->getOrCreateStateSet();
    _StateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    _StateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

    _StateSet->setTextureAttributeAndModes(0, _InTexture.get(), osg::StateAttribute::ON);

    _StateSet->addUniform(new osg::Uniform("textureIn", 0));

    quad_geode->addDrawable(quad_geom.get());

    top_group->addChild(quad_geode.get());

    return top_group;
}

void ProcessPass::setupCamera()
{
    // clearing
    _Camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // projection and view
    _Camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
    _Camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _Camera->setViewMatrix(osg::Matrix::identity());

    // viewport
    _Camera->setViewport(0, 0, _TextureWidth, _TextureHeight);

    _Camera->setRenderOrder(osg::Camera::PRE_RENDER);
    _Camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

    _Camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER), _OutTexture.get());
}

void ProcessPass::setShader(std::string filename)
{
    osg::ref_ptr<osg::Shader> fshader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, filename);
    if (!fshader)
    {
        osg::notify(osg::NOTICE)<<"Could not file shader file: "<<filename<<std::endl;
        return;
    }

    _FragmentProgram = 0;
    _FragmentProgram = new osg::Program;

    _FragmentProgram->addShader(fshader.get());

    _StateSet->setAttributeAndModes(_FragmentProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
}

////////////////////////////////////////////////////////////////////////

GameOfLifePass::GameOfLifePass(osg::Image *in_image)
{
    _TextureWidth = in_image->s();
    _TextureHeight = in_image->t();

    _RootGroup = new osg::Group;

    _BranchSwith[0] = new osg::Switch;
    _BranchSwith[1] = new osg::Switch;

    _RootGroup->addChild(_BranchSwith[0].get());
    _RootGroup->addChild(_BranchSwith[1].get());

    _ActiveBranch = 0;
    activateBranch();

    createOutputTextures();
    _InOutTextureLife[0]->setImage(in_image);

    _ProcessPass[0] = new ProcessPass(_InOutTextureLife[0].get(),
                                      _InOutTextureLife[1].get(),
                                      _TextureWidth, _TextureHeight);

    // For the other pass, the input/output textures are flipped
    _ProcessPass[1] = new ProcessPass(_InOutTextureLife[1].get(),
                                      _InOutTextureLife[0].get(),
                                      _TextureWidth, _TextureHeight);

    _BranchSwith[0]->addChild(_ProcessPass[0]->getRoot().get());
    _BranchSwith[1]->addChild(_ProcessPass[1]->getRoot().get());
}

GameOfLifePass::~GameOfLifePass()
{
    delete _ProcessPass[0];
    delete _ProcessPass[1];
}

osg::ref_ptr<osg::TextureRectangle> GameOfLifePass::getOutputTexture()
{
    int out_tex = (_ActiveBranch == 0) ? 1 : 0;
    return _ProcessPass[out_tex]->getOutputTexture();
}

void GameOfLifePass::activateBranch()
{
    int onb = _ActiveBranch;
    int offb = (onb == 1) ? 0 : 1;

    _BranchSwith[onb]->setAllChildrenOn();
    _BranchSwith[offb]->setAllChildrenOff();
}

void GameOfLifePass::flip()
{
    _ActiveBranch = (_ActiveBranch == 1) ? 0 : 1;
    activateBranch();
}

void GameOfLifePass::createOutputTextures()
{
    for (int i=0; i<2; i++) {
        _InOutTextureLife[i] = new osg::TextureRectangle;

        _InOutTextureLife[i]->setTextureSize(_TextureWidth, _TextureHeight);
        _InOutTextureLife[i]->setInternalFormat(GL_RGBA);
        _InOutTextureLife[i]->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
        _InOutTextureLife[i]->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
    }
}

