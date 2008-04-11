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

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/TextureRectangle>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>

#include "StereoPass.h"
#include "StereoMultipass.h"

osg::Node* createScene(osg::Image *left, osg::Image *right, unsigned int min_disp, unsigned int max_disp, unsigned int window_size, bool single_pass)
{
    int width = left->s();
    int height = left->t();

    osg::Group* topnode = new osg::Group;

    // create four quads so we can display up to four images
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    
    // each geom will contain a quad
    osg::ref_ptr<osg::DrawArrays> da = new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4);
    
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

    osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array; // texture coords
    tcoords->push_back(osg::Vec2(0, 0));
    tcoords->push_back(osg::Vec2(width, 0));
    tcoords->push_back(osg::Vec2(width, height));
    tcoords->push_back(osg::Vec2(0, height));
    
    osg::ref_ptr<osg::StateSet> geomss[4]; // stateset where we can attach textures
    osg::ref_ptr<osg::TextureRectangle> texture[4];

    for (int i=0;i<4;i++) {
	osg::ref_ptr<osg::Vec3Array> vcoords = new osg::Vec3Array; // vertex coords
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	
	// tile the quads on the screen
	// 2 3
	// 0 1
	int xoff, zoff;
	xoff = (i%2);
	zoff = i>1 ? 1 : 0;
	
	// initial viewer camera looks along y
	vcoords->push_back(osg::Vec3d(0+(xoff * width), 0, 0+(zoff * height)));
	vcoords->push_back(osg::Vec3d(width+(xoff * width), 0, 0+(zoff * height)));
	vcoords->push_back(osg::Vec3d(width+(xoff * width), 0, height+(zoff * height)));
	vcoords->push_back(osg::Vec3d(0+(xoff * width), 0, height+(zoff * height)));

	geom->setVertexArray(vcoords.get());
	geom->setTexCoordArray(0,tcoords.get());
	geom->addPrimitiveSet(da.get());
	geom->setColorArray(colors.get());
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);
	geomss[i] = geom->getOrCreateStateSet();
	geomss[i]->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	texture[i] = new osg::TextureRectangle;
	texture[i]->setResizeNonPowerOfTwoHint(false);
	texture[i]->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture[i]->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

	geode->addDrawable(geom.get());
    }

    // attach the input images to the bottom textures of the view
    texture[0]->setImage(left);
    texture[1]->setImage(right);
    geomss[0]->setTextureAttributeAndModes(0, texture[0].get(), osg::StateAttribute::ON);
    geomss[1]->setTextureAttributeAndModes(0, texture[1].get(), osg::StateAttribute::ON);

    topnode->addChild(geode.get());
    
    // create the processing passes
    if (single_pass) {
	StereoPass *stereopass = new StereoPass(texture[0].get(), texture[1].get(),
						width, height,
						min_disp, max_disp, window_size);
    
	topnode->addChild(stereopass->getRoot().get());
	
	// attach the output of the processing to the top left geom
	geomss[2]->setTextureAttributeAndModes(0,
					       stereopass->getOutputTexture().get(),
					       osg::StateAttribute::ON);
    } else {
	StereoMultipass *stereomp = new StereoMultipass(texture[0].get(), texture[1].get(),
						width, height,
						min_disp, max_disp, window_size);
	topnode->addChild(stereomp->getRoot().get());
	// attach the output of the processing to the top left geom
	geomss[2]->setTextureAttributeAndModes(0,
					       stereomp->getOutputTexture().get(),
					       osg::StateAttribute::ON);
    }

    return topnode;
}

int main(int argc, char *argv[])
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates a stereo matching algorithm. It uses multiple render targets and multiple passes with texture ping-pong.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] --left left_image --right right_image --min min_disparity --max max_disparity --window window_size");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--left","The left image of the stereo pair to load.");
    arguments.getApplicationUsage()->addCommandLineOption("--right","The right image of the stereo pair to load.");
    arguments.getApplicationUsage()->addCommandLineOption("--min","The minimum disparity to start matching pixels.");
    arguments.getApplicationUsage()->addCommandLineOption("--max","The maximum disparity to stop matching pixels.");
    arguments.getApplicationUsage()->addCommandLineOption("--window","The window size used to match areas around pixels.");
    arguments.getApplicationUsage()->addCommandLineOption("--single","Use a single pass instead on multiple passes.");

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    std::string leftName("");
    while(arguments.read("--left", leftName)) {}

    std::string rightName("");
    while(arguments.read("--right", rightName)) {}

    unsigned int minDisparity = 0;
    while (arguments.read("--min", minDisparity)) {}

    unsigned int maxDisparity = 31;
    while (arguments.read("--max", maxDisparity)) {}

    unsigned int windowSize = 5;
    while (arguments.read("--window", windowSize)) {}

    bool useSinglePass = false;
    while (arguments.read("--single")) { useSinglePass = true; }

    if (leftName == "" || rightName=="") {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // load the images
    osg::ref_ptr<osg::Image> leftIm = osgDB::readImageFile(leftName);
    osg::ref_ptr<osg::Image> rightIm = osgDB::readImageFile(rightName);
   
    osg::Node* scene = createScene(leftIm.get(), rightIm.get(), minDisparity, maxDisparity, windowSize, useSinglePass);
    
    // construct the viewer.
    osgViewer::Viewer viewer;
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.setSceneData(scene);

    return viewer.run();
}
