/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the GNU Public License (GPL) version 1.0 or 
 * (at your option) any later version. 
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>
#include <osgGA/TrackballManipulator>

#include "SlideEventHandler.h"
#include "PointsEventHandler.h"
#include "SlideShowConstructor.h"


class FindHomePositionVisitor : public osg::NodeVisitor
{
public:

    FindHomePositionVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
        
    void apply(osg::Node& node)
    {
        SlideShowConstructor::HomePosition* homePosition = dynamic_cast<SlideShowConstructor::HomePosition*>(node.getUserData());
        if (homePosition)
        {
            _homePosition = homePosition;
        }
        
        traverse(node);
    }
        
    osg::ref_ptr<SlideShowConstructor::HomePosition> _homePosition;
        
};

// add in because MipsPro can't handle no osgGA infront of TrackballManipulator
// but VisualStudio6.0 can't handle the osgGA...
using namespace osgGA;
class SlideShowTrackballManipulator : public osgGA::TrackballManipulator
{
    public:
    
        SlideShowTrackballManipulator()
        {
        }
    
        virtual void home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
        {
        
            FindHomePositionVisitor fhpv;
            if (_node.valid()) _node->accept(fhpv);
        
            if (fhpv._homePosition.valid())
            {
                computePosition(fhpv._homePosition->eye,
                                fhpv._homePosition->center,
                                fhpv._homePosition->up);

                us.requestRedraw();
            }
            else
            {
               TrackballManipulator::home(ea,us);
            }
        }
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the application for presenting 3D interactive slide shows.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-a","Turn auto stepping on by default");
    arguments.getApplicationUsage()->addCommandLineOption("-d <float>","Time delay in seconds between layers/slides");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    SlideShowTrackballManipulator* sstbm = new SlideShowTrackballManipulator;

    viewer.addCameraManipulator(sstbm);

    // set up the value with sensible default event handler.
    viewer.setUpViewer(osgProducer::Viewer::DRIVE_MANIPULATOR |
                       osgProducer::Viewer::FLIGHT_MANIPULATOR |
                       osgProducer::Viewer::STATE_MANIPULATOR |
                       osgProducer::Viewer::HEAD_LIGHT_SOURCE |
                       osgProducer::Viewer::STATS_MANIPULATOR |
                       osgProducer::Viewer::VIEWER_MANIPULATOR |
                       osgProducer::Viewer::ESCAPE_SETS_DONE);

    // read any time delay argument.
    float timeDelayBetweenSlides = 1.5f;
    while (arguments.read("-d",timeDelayBetweenSlides)) {}

    bool autoSteppingActive = false;
    while (arguments.read("-a")) autoSteppingActive = true;

    // register the slide event handler - which moves the presentation from slide to slide, layer to layer.
    SlideEventHandler* seh = new SlideEventHandler;
    viewer.getEventHandlerList().push_front(seh);
    
    seh->setAutoSteppingActive(autoSteppingActive);
    seh->setTimeDelayBetweenSlides(timeDelayBetweenSlides);

    // register the handler for modifying the point size
    PointsEventHandler* peh = new PointsEventHandler;
    viewer.getEventHandlerList().push_front(peh);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    osg::Timer_t end_tick = timer.tick();

    std::cout << "Time to load = "<<timer.delta_s(start_tick,end_tick)<<std::endl;

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    // pass the model to the slide event handler so it knows which to manipulate.
    seh->set(loadedModel.get());

    // set the scene to render
    viewer.setSceneData(loadedModel.get());


    // pass the global stateset to the point event handler so that it can
    // alter the point size of all points in the scene.
    peh->setStateSet(viewer.getGlobalStateSet());

    // create the windows and run the threads.
    viewer.realize();

    // set all the sceneview's up so that their left and right add cull masks are set up.
    for(osgProducer::OsgCameraGroup::SceneHandlerList::iterator itr=viewer.getSceneHandlerList().begin();
        itr!=viewer.getSceneHandlerList().end();
        ++itr)
    {
        osgUtil::SceneView* sceneview = (*itr)->getSceneView();
        sceneview->setCullMask(0xffffffff);
        sceneview->setCullMaskLeft(0x00000001);
        sceneview->setCullMaskRight(0x00000002);
//        sceneview->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE,radius);
    }

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}

