/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/PolygonOffset>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/FrontFace>

#include <osgText/Text>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>

#include <osgViewer/CompositeViewer>

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    if (!scene) return 1;

    // construct the viewer.
    osgViewer::CompositeViewer viewer;
    
    if (arguments.read("-1"))
    {    
#if 1    
        osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
        if (!wsi) 
        {
            osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
            return 1;
        }

        unsigned int width, height;
        wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 0;
        traits->y = 0;
        traits->width = width;
        traits->height = height;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (gc.valid())
        {
            osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<std::endl;

            // need to ensure that the window is cleared make sure that the complete window is set the correct colour
            // rather than just the parts of the window that are under the camera's viewports
            gc->setClearColor(osg::Vec4f(0.2f,0.2f,0.6f,1.0f));
            gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else
        {
            osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
        }

        osgViewer::View* view_one = new osgViewer::View;
        view_one->setSceneData(scene.get());
        view_one->getCamera()->setViewport(new osg::Viewport(0,0, width/2, height/2));
        view_one->getCamera()->setGraphicsContext(gc.get());
        view_one->setCameraManipulator(new osgGA::TrackballManipulator);
        viewer.addView(view_one);

        osgViewer::View* view_two = new osgViewer::View;
        view_two->setSceneData(scene.get());
        view_two->getCamera()->setViewport(new osg::Viewport(width/2,0, width/2, height/2));
        view_two->getCamera()->setGraphicsContext(gc.get());
        view_two->setCameraManipulator(new osgGA::TrackballManipulator);
        viewer.addView(view_two);
#endif
        osgViewer::View* view_three = new osgViewer::View;
        view_three->setSceneData(osgDB::readNodeFile("town.ive"));
        view_three->setUpViewAcrossAllScreens();
#if 0        
        view_three->getCamera()->setViewport(new osg::Viewport(0, height/2, width, height/2));
        view_three->getCamera()->setGraphicsContext(gc.get());
#endif        
        view_three->setCameraManipulator(new osgGA::FlightManipulator);
        viewer.addView(view_three);
    }
    else
    {
        osgViewer::View* view_one = new osgViewer::View;
        view_one->setUpViewOnSingleScreen(0);
        view_one->setSceneData(scene.get());
        view_one->setCameraManipulator(new osgGA::TrackballManipulator);
        viewer.addView(view_one);

        osgViewer::View* view_two = new osgViewer::View;
        view_two->setUpViewOnSingleScreen(1);
        view_two->setSceneData(scene.get());
        view_two->setCameraManipulator(new osgGA::TrackballManipulator);
        viewer.addView(view_two);
    }
    
    
    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::CompositeViewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::CompositeViewer::ThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::CompositeViewer::ThreadPerCamera); }
 
    return viewer.run();
}
