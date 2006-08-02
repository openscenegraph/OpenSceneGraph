#include <osgProducer/Viewer>

#include <osg/Notify>

#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates a simple use of multi-texturing.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
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
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
    if (!rootnode)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    osg::Image* image = osgDB::readImageFile("Images/reflect.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        osg::TexGen* texgen = new osg::TexGen;
        texgen->setMode(osg::TexGen::SPHERE_MAP);

        osg::TexEnv* texenv = new osg::TexEnv;
        texenv->setMode(osg::TexEnv::BLEND);
        texenv->setColor(osg::Vec4(0.3f,0.3f,0.3f,0.3f));

        osg::StateSet* stateset = new osg::StateSet;
        stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);
        stateset->setTextureAttributeAndModes(1,texgen,osg::StateAttribute::ON);
        stateset->setTextureAttribute(1,texenv);
        
        rootnode->setStateSet(stateset);
    }
    else
    {
        osg::notify(osg::NOTICE)<<"unable to load reflect map, model will not be mutlitextured"<<std::endl;
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    
    // create the windows and run the threads.
    viewer.realize();

    for(unsigned int contextID = 0; 
        contextID<viewer.getDisplaySettings()->getMaxNumberOfGraphicsContexts();
        ++contextID)
    {
        osg::Texture::Extensions* textExt = osg::Texture::getExtensions(contextID,false);
        if (textExt)
        {
            if (!textExt->isMultiTexturingSupported())
            {
                std::cout<<"Warning: multi-texturing not supported by OpenGL drivers, unable to run application."<<std::endl;
                return 1;
            }
        }
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
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
