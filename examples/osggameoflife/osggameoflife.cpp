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

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/TextureRectangle>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>

#include <iostream>

#include "GameOfLifePass.h"

GameOfLifePass *golpass;
osg::ref_ptr<osg::StateSet> geomss; // stateset where we can attach textures

osg::Node* createScene(osg::Image *start_im)
{
    int width = start_im->s();
    int height = start_im->t();

    osg::Group* topnode = new osg::Group;

    // create quad to display image on
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

    osg::ref_ptr<osg::Vec3Array> vcoords = new osg::Vec3Array; // vertex coords
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    // initial viewer camera looks along y
    vcoords->push_back(osg::Vec3d(0, 0, 0));
    vcoords->push_back(osg::Vec3d(width, 0, 0));
    vcoords->push_back(osg::Vec3d(width, 0, height));
    vcoords->push_back(osg::Vec3d(0, 0, height));

    geom->setVertexArray(vcoords.get());
    geom->setTexCoordArray(0,tcoords.get());
    geom->addPrimitiveSet(da.get());
    geom->setColorArray(colors.get(), osg::Array::BIND_OVERALL);
    geomss = geom->getOrCreateStateSet();
    geomss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    geode->addDrawable(geom.get());

    topnode->addChild(geode.get());

    // create the ping pong processing passes
    golpass = new GameOfLifePass(start_im);
    topnode->addChild(golpass->getRoot().get());

    // attach the output of the processing to the geom
    geomss->setTextureAttributeAndModes(0,
                                        golpass->getOutputTexture().get(),
                                        osg::StateAttribute::ON);
    return topnode;
}

int main(int argc, char *argv[])
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates ping pong rendering with FBOs and multiple rendering branches. It uses Conway's Game of Life to illustrate the concept.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] --startim start_image");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--startim","The initial image to seed the game of life with.");

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    std::string startName("");
    while(arguments.read("--startim", startName)) {}

    if (startName == "") {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // load the image
    osg::ref_ptr<osg::Image> startIm = osgDB::readRefImageFile(startName);

    if (!startIm) {
        std::cout << "Could not load start image.\n";
        return(1);
    }

    osg::ref_ptr<osg::Node> scene = createScene(startIm.get());

    // construct the viewer.
    osgViewer::Viewer viewer;
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.setSceneData(scene);

    viewer.realize();
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    while(!viewer.done())
    {
        viewer.frame();
        // flip the textures after we've completed a frame
        golpass->flip();
        // attach the proper output to view
        geomss->setTextureAttributeAndModes(0,
                                            golpass->getOutputTexture().get(),
                                            osg::StateAttribute::ON);
    }

    return 0;
}
