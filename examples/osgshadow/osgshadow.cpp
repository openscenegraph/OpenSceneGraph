#include <osg/ArgumentParser>

#include <osgProducer/Viewer>

#include <osgShadow/OccluderGeometry>

#include <osgDB/ReadFile>


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc, argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the example which demonstrates using of GL_ARB_shadow extension implemented in osg::Texture class");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName());
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--with-base-texture", "Adde base texture to shadowed model.");
    arguments.getApplicationUsage()->addCommandLineOption("--no-base-texture", "Adde base texture to shadowed model.");

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments. getApplicationUsage());

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


    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    if (!model)
    {
        osg::notify(osg::NOTICE)<<"No model loaded, please specify a model to load."<<std::endl;
        return 1;
    }
    
    
    osg::ref_ptr<osgShadow::OccluderGeometry> occluder = new osgShadow::OccluderGeometry;
    occluder->computeOccluderGeometry(model.get());
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(occluder.get());

    viewer.setSceneData(geode.get());

    // create the windows and run the threads.
    viewer.realize();

    while (!viewer.done())
    {
      // wait for all cull and draw threads to complete.
      viewer.sync();

      // update the scene by traversing it with the the update visitor which will
      // call all node update callbacks and animations.
      viewer.update();
         
      // fire off the cull and draw traversals of the scene.
      viewer.frame();
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
