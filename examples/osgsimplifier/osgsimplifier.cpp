/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osgUtil/Simplifier>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    float sampleRatio = 0.5f;

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // read the sample ratio if one is supplied
    while (arguments.read("-s",sampleRatio)) {}

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    std::string outputFileName;
    while (arguments.read("-o",outputFileName)) {}

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

    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

    osg::Timer_t end_load_tick = osg::Timer::instance()->tick();

    std::cout << "Time to load = "<<osg::Timer::instance()->delta_s(start_tick,end_load_tick)<<std::endl;

    osgUtil::Simplifier simplifier(sampleRatio);
    loadedModel->accept(simplifier);

    osg::Timer_t end_simplifier_tick = osg::Timer::instance()->tick();

    std::cout << "Time to simplify = "<<osg::Timer::instance()->delta_s(end_load_tick, end_simplifier_tick)<<std::endl;

    osgUtil::TriStripVisitor tsv;
    tsv.setMinStripSize(3);
    loadedModel->accept(tsv);
    tsv.stripify();

    osg::Timer_t end_tristrip_tick = osg::Timer::instance()->tick();

    std::cout << "Time to tri strip = "<<osg::Timer::instance()->delta_s(end_simplifier_tick, end_tristrip_tick)<<std::endl;

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(loadedModel.get());

    if (!outputFileName.empty())
    {
        std::cout << "Writing out scene graph as '" << outputFileName << "'"<<std::endl;
        osgDB::writeNodeFile(*loadedModel,outputFileName);
        return 0;
    }

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

    return 0;
}

