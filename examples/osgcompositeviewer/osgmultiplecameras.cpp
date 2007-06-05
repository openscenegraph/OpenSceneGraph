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
#include <osgGA/StateSetManipulator>

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

    if (!scene) return 1;

    // construct the viewer.
    osgViewer::CompositeViewer viewer;
    
    
    if (arguments.read("-1"))
    {
        {
            osgViewer::View* view = new osgViewer::View;
            view->setSceneData(osgDB::readNodeFile("fountain.osg"));

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
            viewer.addView(view);

            view->setUpViewOnSingleScreen(1);
            view->setSceneData(scene.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);
            
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
            viewer.addView(view);

            view->setSceneData(scene.get());
            view->getCamera()->setViewport(new osg::Viewport(0,0, traits->width/2, traits->height/2));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet(view->getCamera()->getOrCreateStateSet());

            view->addEventHandler( statesetManipulator.get() );
        }

        // view two
        {
            osgViewer::View* view = new osgViewer::View;
            viewer.addView(view);

            view->setSceneData(scene.get());
            view->getCamera()->setViewport(new osg::Viewport(traits->width/2,0, traits->width/2, traits->height/2));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);
            
            // add the handler for doing the picking
            view->addEventHandler(new PickHandler());
            
        }

        // view three
        {
            osgViewer::View* view = new osgViewer::View;
            viewer.addView(view);

            view->setSceneData(osgDB::readNodeFile("cessnafire.osg"));

            view->getCamera()->setProjectionMatrixAsPerspective(30.0, double(traits->width) / double(traits->height/2), 1.0, 1000.0);
            view->getCamera()->setViewport(new osg::Viewport(0, traits->height/2, traits->width, traits->height/2));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);
        }

    }
    
    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::CompositeViewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::CompositeViewer::ThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::CompositeViewer::ThreadPerCamera); }
 
     // run the viewer's main frame loop
     return viewer.run();
}
