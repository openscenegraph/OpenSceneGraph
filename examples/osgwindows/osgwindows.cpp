/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include <Producer/CameraConfig>

static Producer::CameraConfig *BuildConfig(void)
{
#define TWO_SEPERATE_WINDOWS

#ifdef TWO_SEPERATE_WINDOWS
    Producer::RenderSurface *rs1 = new Producer::RenderSurface;
    rs1->setScreenNum(0);
    rs1->setWindowName("osgwindows");
    //rs1->useBorder(false);
    rs1->setWindowRectangle(0,0,640,480);
    Producer::Camera *camera1 = new Producer::Camera;
    camera1->setRenderSurface(rs1);
    camera1->setOffset( 1.0, 0.0 );

    Producer::RenderSurface *rs2 = new Producer::RenderSurface;
    rs2->setScreenNum(0);
    rs2->setWindowName("osgwindows");
    //rs2->useBorder(false);
    rs2->setWindowRectangle(640,0,640,480);
    Producer::Camera *camera2 = new Producer::Camera;
    camera2->setRenderSurface(rs2);
    camera2->setOffset( -1.0, 0.0 );

    rs1->setInputRectangle( Producer::RenderSurface::InputRectangle(0.0,0.5,0.0,1.0));
    rs2->setInputRectangle( Producer::RenderSurface::InputRectangle(0.5,1.0,0.0,1.0));

    Producer::InputArea *ia = new Producer::InputArea;
    ia->addRenderSurface(rs1);
    ia->addRenderSurface(rs2);

    Producer::CameraConfig *cfg = new Producer::CameraConfig;
    cfg->addCamera("Camera 1",camera1);
    cfg->addCamera("Camera 2", camera2);
    cfg->setInputArea(ia);
    return cfg;

#else
    // one window with four camera's.
    Producer::Camera* pcam1 = new Producer::Camera ();
    pcam1->setProjectionRectangle (0.0f, 0.5f, 0.5f, 1.0f);
   
    Producer::Camera* pcam2 = new Producer::Camera ();
    pcam2->setRenderSurface (pcam1->getRenderSurface ());
    pcam2->setProjectionRectangle (0.5f, 1.0f, 0.5f, 1.0f);

    Producer::Camera* pcam3 = new Producer::Camera ();
    pcam3->setRenderSurface (pcam1->getRenderSurface ());
    pcam3->setProjectionRectangle (0.0f, 0.5f, 0.0f, 0.5f);

    Producer::Camera* pcam4 = new Producer::Camera ();
    pcam4->setRenderSurface (pcam1->getRenderSurface ());
    pcam4->setProjectionRectangle (0.5f, 1.0f, 0.0f, 0.5f);

    Producer::CameraConfig *cfg = new Producer::CameraConfig;
    cfg->addCamera("Camera 1",pcam1);
    cfg->addCamera("Camera 2",pcam2);
    cfg->addCamera("Camera 3",pcam3);
    cfg->addCamera("Camera 4",pcam4);
    
    return cfg;
#endif
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrate hows to set up programitically a Producer::CameraConfig and use it to set up multiple window views.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(BuildConfig());

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

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
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }


    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // create the windows and run the threads.
    viewer.realize();

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

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}

