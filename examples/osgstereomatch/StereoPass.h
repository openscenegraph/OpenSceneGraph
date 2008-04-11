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

#ifndef STEREOPASS_H
#define STEREOPASS_H 1

#include <osg/ref_ptr>
#include <osg/Group>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

class StereoPass {
public:
    StereoPass(osg::TextureRectangle *left_tex, 
			   osg::TextureRectangle *right_tex,
			   int width, int height,
			   int min_disparity, int max_disparity, int window_size);
    ~StereoPass();
    osg::ref_ptr<osg::Group> getRoot() { return _RootGroup; }
    osg::ref_ptr<osg::TextureRectangle> getOutputTexture() { return _OutTexture; }
    void setShader(std::string filename);
	
private:
    osg::ref_ptr<osg::Group> createTexturedQuad();
    void createOutputTextures();
    void setupCamera();

    osg::ref_ptr<osg::Group> _RootGroup;
    osg::ref_ptr<osg::Camera> _Camera;
    osg::ref_ptr<osg::TextureRectangle> _InTextureLeft;
    osg::ref_ptr<osg::TextureRectangle> _InTextureRight;
	osg::ref_ptr<osg::TextureRectangle> _OutTexture;
	
	int _TextureWidth;
    int _TextureHeight;
    int _MinDisparity;
    int _MaxDisparity;
    int _WindowSize;
	
    osg::ref_ptr<osg::Program> _FragmentProgram;
    osg::ref_ptr<osg::StateSet> _StateSet;
};

#endif //STEREOPASS_H
