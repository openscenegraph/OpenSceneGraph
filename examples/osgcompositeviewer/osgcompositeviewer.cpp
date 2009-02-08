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

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>

#include <osgViewer/CompositeViewer>

#include <osgFX/Scribe>

#include <osg/io_utils>

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public:

    PickHandler():
        _mx(0.0f),
        _my(0.0f) {}

    ~PickHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
    {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
        if (!view) return false;

        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
            {
                _mx = ea.getX();
                _my = ea.getY();
                break;
            }
            case(osgGA::GUIEventAdapter::RELEASE):
            {
                if (_mx==ea.getX() && _my==ea.getY())
                {
                    pick(view, ea.getX(), ea.getY());
                }
                break;
            }
            default:
                break;
        }
        return false;
    }

    void pick(osgViewer::View* view, float x, float y)
    {
        osg::Node* node = 0;
        osg::Group* parent = 0;

        osgUtil::LineSegmentIntersector::Intersections intersections;
        if (view->computeIntersections(x, y, intersections))
        {
            osgUtil::LineSegmentIntersector::Intersection intersection = *intersections.begin();
            osg::NodePath& nodePath = intersection.nodePath;
            node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
            parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;
        }

        // now we try to decorate the hit node by the osgFX::Scribe to show that its been "picked"
        if (parent && node)
        {

            osgFX::Scribe* parentAsScribe = dynamic_cast<osgFX::Scribe*>(parent);
            if (!parentAsScribe)
            {
                // node not already picked, so highlight it with an osgFX::Scribe
                osgFX::Scribe* scribe = new osgFX::Scribe();
                scribe->addChild(node);
                parent->replaceChild(node,scribe);
            }
            else
            {
                // node already picked so we want to remove scribe to unpick it.
                osg::Node::ParentList parentList = parentAsScribe->getParents();
                for(osg::Node::ParentList::iterator itr=parentList.begin();
                    itr!=parentList.end();
                    ++itr)
                {
                    (*itr)->replaceChild(parentAsScribe,node);
                }
            }
        }

    }

    float _mx, _my;

};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    if (!scene)
    {
        std::cout << argv[0] << ": requires filename argument." << std::endl;
        return 1;
    }

    // construct the viewer.
    osgViewer::CompositeViewer viewer(arguments);

    if (arguments.read("-1"))
    {
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName("Single view");
            view->setSceneData(osgDB::readNodeFile("fountain.osg"));

            view->addEventHandler( new osgViewer::StatsHandler );

            view->setUpViewAcrossAllScreens();
            view->setCameraManipulator(new osgGA::TrackballManipulator);
            viewer.addView(view);
        }
    }

    if (arguments.read("-2"))
    {

        // view one
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName("View one");
            viewer.addView(view);

            view->setUpViewOnSingleScreen(0);
            view->setSceneData(scene.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet(view->getCamera()->getOrCreateStateSet());

            view->addEventHandler( statesetManipulator.get() );
        }

        // view two
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName("View two");
            viewer.addView(view);

            view->setUpViewOnSingleScreen(1);
            view->setSceneData(scene.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);

            view->addEventHandler( new osgViewer::StatsHandler );


            // add the handler for doing the picking
            view->addEventHandler(new PickHandler());
        }
    }


    if (arguments.read("-3") || viewer.getNumViews()==0)
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
            view->setCameraManipulator(new osgGA::TrackballManipulator);

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet(view->getCamera()->getOrCreateStateSet());

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
            view->setCameraManipulator(new osgGA::TrackballManipulator);

            // add the handler for doing the picking
            view->addEventHandler(new PickHandler());

        }

        // view three
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName("View three");
            viewer.addView(view);

            view->setSceneData(osgDB::readNodeFile("cessnafire.osg"));

            view->getCamera()->setName("Cam three");
            view->getCamera()->setProjectionMatrixAsPerspective(30.0, double(traits->width) / double(traits->height/2), 1.0, 1000.0);
            view->getCamera()->setViewport(new osg::Viewport(0, traits->height/2, traits->width, traits->height/2));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);
        }

    }


    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::CompositeViewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::CompositeViewer::CullDrawThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::CompositeViewer::CullThreadPerCameraDrawThreadPerContext); }

     // run the viewer's main frame loop
     return viewer.run();
}

