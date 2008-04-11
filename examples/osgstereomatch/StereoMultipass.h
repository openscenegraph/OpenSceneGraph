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

#ifndef STEREOMULTIPASS_H
#define STEREOMULTIPASS_H 1

#include <osg/ref_ptr>
#include <osg/Group>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

class SubtractPass {
public:
    SubtractPass(osg::TextureRectangle *left_tex, 
				 osg::TextureRectangle *right_tex,
				 int width, int height,
				 int start_disparity);
    ~SubtractPass();
    osg::ref_ptr<osg::Group> getRoot() { return _RootGroup; }
    osg::ref_ptr<osg::TextureRectangle> getOutputTexture(int i) { return _OutTexture[i]; }
    void setShader(std::string filename);

private:
    osg::ref_ptr<osg::Group> createTexturedQuad();
    void createOutputTextures();
    void setupCamera();

    osg::ref_ptr<osg::Group> _RootGroup;
    osg::ref_ptr<osg::Camera> _Camera;
    osg::ref_ptr<osg::TextureRectangle> _InTextureLeft;
    osg::ref_ptr<osg::TextureRectangle> _InTextureRight;
    osg::ref_ptr<osg::TextureRectangle> _OutTexture[4];
    int _TextureWidth;
    int _TextureHeight;
    int _StartDisparity;
    osg::ref_ptr<osg::Program> _FragmentProgram;
    osg::ref_ptr<osg::StateSet> _StateSet;
};

class AggregatePass {
public:
    AggregatePass(osg::TextureRectangle *diff_tex0,
				  osg::TextureRectangle *diff_tex1,
				  osg::TextureRectangle *diff_tex2,
				  osg::TextureRectangle *diff_tex3,
				  osg::TextureRectangle *agg_tex_in,
				  osg::TextureRectangle *agg_tex_out,
				  int width, int height,
				  int start_disparity, int window_size);
    ~AggregatePass();
    osg::ref_ptr<osg::Group> getRoot() { return _RootGroup; }
    osg::ref_ptr<osg::TextureRectangle> getOutputTexture() { return _OutTexture; }
    void setShader(std::string filename);

private:
    osg::ref_ptr<osg::Group> createTexturedQuad();
    void setupCamera();
    
    osg::ref_ptr<osg::Group> _RootGroup;
    osg::ref_ptr<osg::Camera> _Camera;
    osg::ref_ptr<osg::TextureRectangle> _InTextureDifference[4];
    osg::ref_ptr<osg::TextureRectangle> _InTextureAggregate;
    osg::ref_ptr<osg::TextureRectangle> _OutTextureAggregate;
    osg::ref_ptr<osg::TextureRectangle> _OutTexture;
    int _TextureWidth;
    int _TextureHeight;
    int _StartDisparity;
    int _WindowSize;
    osg::ref_ptr<osg::Program> _FragmentProgram;
    osg::ref_ptr<osg::StateSet> _StateSet;
};

class SelectPass {
public:
    SelectPass(osg::TextureRectangle *in_tex, 
			   int width, int height,
			   int min_disparity, int max_disparity);
    ~SelectPass();
    osg::ref_ptr<osg::Group> getRoot() { return _RootGroup; }
    osg::ref_ptr<osg::TextureRectangle> getOutputTexture() { return _OutTexture; }
    void setShader(std::string filename);
	
private:
    osg::ref_ptr<osg::Group> createTexturedQuad();
    void createOutputTextures();
    void setupCamera();

    osg::ref_ptr<osg::Group> _RootGroup;
    osg::ref_ptr<osg::Camera> _Camera;
    osg::ref_ptr<osg::TextureRectangle> _InTexture;
	osg::ref_ptr<osg::TextureRectangle> _OutTexture;
	osg::ref_ptr<osg::Image> _OutImage;
	int _TextureWidth;
    int _TextureHeight;
    int _MinDisparity;
    int _MaxDisparity;
	osg::ref_ptr<osg::Program> _FragmentProgram;
    osg::ref_ptr<osg::StateSet> _StateSet;
};

class StereoMultipass {
public:
    StereoMultipass(osg::TextureRectangle *left_tex, 
					osg::TextureRectangle *right_tex,
					int width, int height, 
					int min_disparity, int max_disparity, int window_size);
    ~StereoMultipass();
    osg::ref_ptr<osg::Group> getRoot() { return _RootGroup; }
    osg::ref_ptr<osg::TextureRectangle> getOutputTexture() { return _SelectPass->getOutputTexture().get(); }
    void setShader(std::string filename);

private:
    osg::ref_ptr<osg::Group> createTexturedQuad();
    void createOutputTextures();
    void setupCamera();
    
    osg::ref_ptr<osg::Group> _RootGroup;
    osg::ref_ptr<osg::Camera> _Camera;
    osg::ref_ptr<osg::TextureRectangle> _InTexture;
    osg::ref_ptr<osg::TextureRectangle> _OutTexture[2];
	int _TextureWidth;
    int _TextureHeight;
	osg::ref_ptr<osg::Program> _FragmentProgram;
    osg::ref_ptr<osg::StateSet> _StateSet;

    SelectPass *_SelectPass;

    int flip;
    int flop;
};

#endif //STEREOMULTIPASS_H
