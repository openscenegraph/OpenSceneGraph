#include <osg/Transform>
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

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>
#include <osgUtil/TransformCallback>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osgUtil/Optimizer>

void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] infile1 [infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName      - load plugin of name libraryName"<< std::endl;
    out <<"                          i.e. -l osgdb_pfb"<< std::endl;
    out <<"                          Useful for loading reader/writers which can load"<< std::endl;
    out <<"                          other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName    - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                          i.e. -e pfb"<< std::endl;
    out <<"                          Useful short hand for specifying full library name as"<< std::endl;
    out <<"                          done with -l above, as it automatically expands to"<< std::endl;
    out <<"                          the full library name appropriate for each platform."<< std::endl;
    out <<std::endl;
    out <<"    -stereo             - switch on stereo rendering, using the default of,"<< std::endl;
    out <<"                          ANAGLYPHIC or the value set in the OSG_STEREO_MODE "<< std::endl;
    out <<"                          environmental variable. See doc/stereo.html for "<< std::endl;
    out <<"                          further details on setting up accurate stereo "<< std::endl;
    out <<"                          for your system. "<< std::endl;
    out <<"    -stereo ANAGLYPHIC  - switch on anaglyphic(red/cyan) stereo rendering."<< std::endl;
    out <<"    -stereo QUAD_BUFFER - switch on quad buffered stereo rendering."<< std::endl;
    out <<std::endl;
    out <<"    -stencil            - use a visual with stencil buffer enabled, this "<< std::endl;
    out <<"                          also allows the depth complexity statistics mode"<< std::endl;
    out <<"                          to be used (press 'p' three times to cycle to it)."<< std::endl;
    out << std::endl;
}


osg::Node* decorate_with_clip_node(osg::Node* subgraph)
{
    osg::Group* rootnode = osgNew osg::Group;
    

    // create wireframe view of the model so the user can see
    // what parts are being culled away.
    osg::StateSet* stateset = new osg::StateSet;
    osg::Material* material = new osg::Material;
    osg::PolygonMode* polymode = new osg::PolygonMode;
    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE_ON);
    
    osg::Group* wireframe_subgraph = osgNew osg::Group;
    wireframe_subgraph->setStateSet(stateset);
    wireframe_subgraph->addChild(subgraph);
    rootnode->addChild(wireframe_subgraph);

/*
    // simple approach to adding a clipnode above a subrgaph.

    // create clipped part.
    osg::ClipNode* clipped_subgraph = osgNew osg::ClipNode;

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
    
    osg::Transform* transform= osgNew osg::Transform;

    osg::NodeCallback* nc = new osgUtil::TransformCallback(subgraph->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    transform->setAppCallback(nc);

    osg::ClipNode* clipnode = osgNew osg::ClipNode;
    osg::BoundingSphere bs = subgraph->getBound();
    bs.radius()*= 0.4f;

    osg::BoundingBox bb;
    bb.expandBy(bs);

    clipnode->createClipBox(bb);
    clipnode->setCullingActive(false);

    transform->addChild(clipnode);
    rootnode->addChild(transform);


    // create clipped part.
    osg::Group* clipped_subgraph = osgNew osg::Group;

    clipped_subgraph->setStateSet(clipnode->getStateSet());
    clipped_subgraph->addChild(subgraph);
    rootnode->addChild(clipped_subgraph);

    return rootnode;
}


int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);
    
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(commandLine);
    if (!loadedModel)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
  
    // decorate the scenegraph with a clip node.
    osg::Node* rootnode = decorate_with_clip_node(loadedModel);

  
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
    viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
