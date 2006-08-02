#include <osg/MatrixTransform>
#include <osg/ClipNode>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>
#include <osg/LineStipple>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgProducer/Viewer>

#include <osgUtil/Optimizer>


osg::Node* decorate_with_clip_node(osg::Node* subgraph)
{
    osg::Group* rootnode = new osg::Group;
    

    // create wireframe view of the model so the user can see
    // what parts are being culled away.
    osg::StateSet* stateset = new osg::StateSet;
    //osg::Material* material = new osg::Material;
    osg::PolygonMode* polymode = new osg::PolygonMode;
    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    
    osg::Group* wireframe_subgraph = new osg::Group;
    wireframe_subgraph->setStateSet(stateset);
    wireframe_subgraph->addChild(subgraph);
    rootnode->addChild(wireframe_subgraph);

/*
    // simple approach to adding a clipnode above a subrgaph.

    // create clipped part.
    osg::ClipNode* clipped_subgraph = new osg::ClipNode;

    osg::BoundingSphere bs = subgraph->getBound();
    bs.radius()*= 0.4f;

    osg::BoundingBox bb;
    bb.expandBy(bs);


    clipped_subgraph->createClipBox(bb);
    clipped_subgraph->addChild(subgraph);
    rootnode->addChild(clipped_subgraph);
*/


    // more complex approach to managing ClipNode, allowing
    // ClipNode node to be transformed independantly from the subgraph
    // that it is clipping.
    
    osg::MatrixTransform* transform= new osg::MatrixTransform;

    osg::NodeCallback* nc = new osg::AnimationPathCallback(subgraph->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    transform->setUpdateCallback(nc);

    osg::ClipNode* clipnode = new osg::ClipNode;
    osg::BoundingSphere bs = subgraph->getBound();
    bs.radius()*= 0.4f;

    osg::BoundingBox bb;
    bb.expandBy(bs);

    clipnode->createClipBox(bb);
    clipnode->setCullingActive(false);

    transform->addChild(clipnode);
    rootnode->addChild(transform);


    // create clipped part.
    osg::Group* clipped_subgraph = new osg::Group;

    clipped_subgraph->setStateSet(clipnode->getStateSet());
    clipped_subgraph->addChild(subgraph);
    rootnode->addChild(clipped_subgraph);

    return rootnode;
}


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use multi-pass and osg::ClipNode to clip parts of the scene away..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // initialize the viewer.
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
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        return 1;
    }
  
    // decorate the scenegraph with a clip node.
    osg::Node* rootnode = decorate_with_clip_node(loadedModel);

  
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // set the scene to render
    viewer.setSceneData(rootnode);

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
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
