/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/* OpenSceneGraph example, osgstereomatch.
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

#include "StereoPass.h"
#include <osgDB/FileUtils>
#include <iostream>

StereoPass::StereoPass(osg::TextureRectangle *left_tex, 
					   osg::TextureRectangle *right_tex,
					   int width, int height,
					   int min_disparity, int max_disparity, int window_size):
    _TextureWidth(width),
    _TextureHeight(height),
    _MinDisparity(min_disparity),
    _MaxDisparity(max_disparity),
    _WindowSize(window_size)
{
    _RootGroup = new osg::Group;
    
	_InTextureLeft = left_tex;
    _InTextureRight = right_tex;
   
    createOutputTextures();

    _Camera = new osg::Camera;
    setupCamera();
    _Camera->addChild(createTexturedQuad().get());

    _RootGroup->addChild(_Camera.get());

    setShader("shaders/stereomatch_stereopass.frag");
}

StereoPass::~StereoPass()
{
}

osg::ref_ptr<osg::Group> StereoPass::createTexturedQuad()
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

    quad_geom->setVertexArray(quad_coords.get());
    quad_geom->setTexCoordArray(0, quad_tcoords.get());
    quad_geom->addPrimitiveSet(quad_da.get());
    
    _StateSet = quad_geom->getOrCreateStateSet();
    _StateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    _StateSet->setTextureAttributeAndModes(0, _InTextureLeft.get(), osg::StateAttribute::ON);
    _StateSet->setTextureAttributeAndModes(1, _InTextureRight.get(), osg::StateAttribute::ON);

    _StateSet->addUniform(new osg::Uniform("textureID0", 0));
    _StateSet->addUniform(new osg::Uniform("textureID1", 1));
    _StateSet->addUniform(new osg::Uniform("min_disparity", _MinDisparity));
    _StateSet->addUniform(new osg::Uniform("max_disparity", _MaxDisparity));
    _StateSet->addUniform(new osg::Uniform("window_size", _WindowSize));

    quad_geode->addDrawable(quad_geom.get());
    
    top_group->addChild(quad_geode.get());

    return top_group;
}

void StereoPass::setupCamera()
{
    // clearing
    _Camera->setClearColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    _Camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // projection and view
    _Camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
    _Camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _Camera->setViewMatrix(osg::Matrix::identity());

    // viewport
    _Camera->setViewport(0, 0, _TextureWidth, _TextureHeight);

    _Camera->setRenderOrder(osg::Camera::PRE_RENDER);
    _Camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	// attach the output texture and use it as the color buffer.
	_Camera->attach(osg::Camera::COLOR_BUFFER, _OutTexture.get());
}

void StereoPass::createOutputTextures()
{
    _OutTexture = new osg::TextureRectangle;
    
    _OutTexture->setTextureSize(_TextureWidth, _TextureHeight);
    _OutTexture->setInternalFormat(GL_RGBA);
    _OutTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _OutTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
}

void StereoPass::setShader(std::string filename)
{
    osg::ref_ptr<osg::Shader> fshader = new osg::Shader( osg::Shader::FRAGMENT ); 
    fshader->loadShaderSourceFromFile(osgDB::findDataFile(filename));

    _FragmentProgram = 0;
    _FragmentProgram = new osg::Program;

    _FragmentProgram->addShader(fshader.get());

    _StateSet->setAttributeAndModes(_FragmentProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
}
