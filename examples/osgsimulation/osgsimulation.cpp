#include <osgProducer/Viewer>

#include <osg/Group>
#include <osg/Geode>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/ParticleSystemUpdater>


//////////////////////////////////////////////////////////////////////////////
// MAIN SCENE GRAPH BUILDING FUNCTION
//////////////////////////////////////////////////////////////////////////////

void build_world(osg::Group *root)
{

    osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect;
    osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect;
    osgParticle::FireEffect* fire = new osgParticle::FireEffect;

    root->addChild(explosion);
    root->addChild(smoke);
    root->addChild(fire);

    osgParticle::ParticleSystemUpdater *psu = new osgParticle::ParticleSystemUpdater;

    psu->addParticleSystem(explosion->getParticleSystem());
    psu->addParticleSystem(smoke->getParticleSystem());
    psu->addParticleSystem(fire->getParticleSystem());

    // add the updater node to the scene graph
    root->addChild(psu);

}


//////////////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of particle systems.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file_left_eye image_file_right_eye");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

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
    
    osg::Group *root = new osg::Group;
    build_world(root);
   
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);
        
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
