/* OpenSceneGraph example, osgcompositeviewer.
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

#include <iostream>

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

#include <osgGA/TerrainManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>

#include <osgViewer/CompositeViewer>

#include <osg/io_utils>

class MyPager : public osgDB::DatabasePager
{
public:
 virtual void updateSceneGraph(const osg::FrameStamp& frameStamp)
 {
   if (frameStamp.getFrameNumber() % 60 == 0)
   {
     osg::Timer_t start = osg::Timer::instance()->tick();
     osgDB::DatabasePager::updateSceneGraph(frameStamp);
     double d = osg::Timer::instance()->delta_m(start, osg::Timer::instance()->tick());
     std::cout << "DatabasePager update took " << d << " ms. Length of active nodes = " << _activePagedLODList->size() << std::endl;
   }
 }
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    if (!scene)
    {
        scene = osgDB::readNodeFile("http://www.openscenegraph.org/data/earth_bayarea/earth.ive");
    }

    if (!scene)
    {
        std::cout << argv[0] << ": requires filename argument." << std::endl;
        return 1;
    }

    // construct the viewer.
    osgViewer::CompositeViewer viewer(arguments);

    if (viewer.getNumViews()==0)
    {

        osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
        if (!wsi)
        {
            osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
            return 1;
        }

        unsigned int width, height;
        wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 100;
        traits->y = 100;
        traits->width = 1000;
        traits->height = 800;
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

        // view one
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName("View one");
            viewer.addView(view);

            view->setSceneData(scene.get());
            view->getCamera()->setName("Cam one");
            view->getCamera()->setViewport(new osg::Viewport(0,0, traits->width/2, traits->height/2));
            view->getCamera()->setGraphicsContext(gc.get());

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet(view->getCamera()->getOrCreateStateSet());

            view->setCameraManipulator(new osgGA::TerrainManipulator);
            view->addEventHandler( statesetManipulator.get() );

            view->addEventHandler( new osgViewer::StatsHandler );
            view->addEventHandler( new osgViewer::HelpHandler );
            view->addEventHandler( new osgViewer::WindowSizeHandler );
            view->addEventHandler( new osgViewer::ThreadingHandler );
            view->addEventHandler( new osgViewer::RecordCameraPathHandler );
        }

        // view two
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName("View two");
            viewer.addView(view);

            view->setSceneData(scene.get());
            view->getCamera()->setName("Cam two");
            view->getCamera()->setViewport(new osg::Viewport(traits->width/2,0, traits->width/2, traits->height/2));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TerrainManipulator);

        }

        // view three
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName("View three");
            viewer.addView(view);

            view->setSceneData(scene.get());

            view->getCamera()->setName("Cam three");
            view->getCamera()->setProjectionMatrixAsPerspective(30.0, double(traits->width) / double(traits->height/2), 1.0, 1000.0);
            view->getCamera()->setViewport(new osg::Viewport(0, traits->height/2, traits->width, traits->height/2));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TerrainManipulator);

            // attach custom database pager
            view->setDatabasePager(new MyPager);
            view->getDatabasePager()->setTargetMaximumNumberOfPageLOD(1);
        }

    }


    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::CompositeViewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::CompositeViewer::CullDrawThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::CompositeViewer::CullThreadPerCameraDrawThreadPerContext); }

     // run the viewer's main frame loop
     return viewer.run();
}

