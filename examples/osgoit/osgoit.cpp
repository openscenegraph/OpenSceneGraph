/* OpenSceneGraph example, myosgoit.
*
*  Author: Christian Buchner, based on original osgoit by Mathias Frhlich
*
*  This demo provides a DepthPeeling object that can correctly compose
*  solid and transparent geometry within the same scene. The transparent
*  geometry can also use GLSL shaders, as demonstrated in the 3D HeatMap.
*  The solid geometry is only rendered once, and its depth buffer blitted
*  into the cameras rendering the transparency layers.
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

#include "DepthPeeling.h"
#include "HeatMap.h"

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Math>
#include <osg/PositionAttitudeTransform>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/LightModel>
#include <osgDB/ReadFile>

#include <limits>
#include <iostream>

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc, argv);
    arguments.getApplicationUsage()->addKeyboardMouseBinding("m", "Increase the number of depth peeling layers");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("n", "Decrease the number of depth peeling layers");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("l", "Toggle display of the individual or composed layer textures");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("p", "Increase the layer offset");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("o", "Decrease the layer offset");

    // Have the usual viewer
    osgViewer::Viewer viewer(arguments);

    osg::DisplaySettings* displaySettings = new osg::DisplaySettings;
    viewer.setDisplaySettings(displaySettings);
   
    // Add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);
   
    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();
   
    // read the dump truck, we will need it twice
    osg::ref_ptr<osg::Node> dt = osgDB::readNodeFile("dumptruck.osg");

    // display a solid version of the dump truck
    osg::ref_ptr<osg::PositionAttitudeTransform> solidModel = new osg::PositionAttitudeTransform;
    solidModel->setPosition(osg::Vec3f(7.0f, -2.0f, 7.0f));
    solidModel->addChild(dt.get());

    // generate the 3D heatmap surface to display
    osg::ref_ptr<Heatmap> hm = new Heatmap(30, 30, 10, 30, 30, 1.0, 0.25);
    float data[30][30];
    for (int x=0; x < 30; ++x)
        for (int y=0; y < 30; ++y)
            data[y][x] = (double)rand() / RAND_MAX;
    hm->setData((float*)data, 10.0, 1.0, 0.25);

    // add a transparent version of the truck to the scene also
    osg::ref_ptr<osg::PositionAttitudeTransform> transparentTruck = new osg::PositionAttitudeTransform;
    transparentTruck->setPosition(osg::Vec3f(7.0f, -25.0f, 7.0f));

    // set the states of the truck so that it actually appears transparently and nicely lit.
    osg::StateSet *state = transparentTruck->getOrCreateStateSet();
    state->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    state->setAttribute(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    osg::Material* material = new osg::Material;
    material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.2f,0.2f,0.2f,0.3f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(0.8f,0.8f,0.8f,0.3f));
    state->setAttribute(material,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    osg::LightModel *lm = new osg::LightModel();
    lm->setTwoSided(true);
    state->setAttribute(lm, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    (transparentTruck.get())->addChild(dt.get());

    // place the heatmap and a transparent dump truck in the transparent geometry group
    osg::ref_ptr<osg::Group> transparentModel = new osg::Group;
    (transparentModel.get())->addChild(hm.get());
    (transparentModel.get())->addChild(transparentTruck.get());

    // The initial size set to 0, 0. We get a resize event for the right size...
    DepthPeeling* depthPeeling = new DepthPeeling(0, 0);
    // the heat map already uses two textures bound to unit 0 and 1, so we can use TexUnit 2 for the peeling
    depthPeeling->setTexUnit(2);
    depthPeeling->setSolidScene(solidModel.get());
    depthPeeling->setTransparentScene(transparentModel.get());
    viewer.setSceneData(depthPeeling->getRoot());

    // Add the event handler for the depth peeling stuff
    viewer.addEventHandler(new DepthPeeling::EventHandler(depthPeeling));

    // force a resize event, so the DepthPeeling object updates _texWidth and _texHeight
    viewer.realize();
    int x, y, width, height;
    osgViewer::ViewerBase::Windows windows;
    viewer.getWindows(windows);
    windows.front()->getWindowRectangle(x,y,width,height);
    viewer.getEventQueue()->windowResize(x,y,width,height);

    return viewer.run();
}
