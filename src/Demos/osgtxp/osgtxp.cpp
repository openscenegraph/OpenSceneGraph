#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Node>
#include <osg/Notify>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgTXP/TrPageArchive.h>
#include <osgTXP/TrPageViewer.h>


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
    out <<"    -f                  - start with a full screen, borderless window." << std::endl;
    out << std::endl;
}

using namespace txp;

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(std::cout,argv[0]);
        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    
    // initialize the viewer.
    PagingViewer *viewer = new PagingViewer();
    viewer->setWindowTitle(argv[0]);

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer->readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    //osgDB::readCommandLine(commandLine);

    // Initialize the TXP database
    bool loadAll = false;
    bool threadIt = false;
    std::string fileName;
    for(std::vector<std::string>::iterator itr=commandLine.begin();
        itr!=commandLine.end();
        ++itr)
    {
        if ((*itr)[0]!='-')
        {
            fileName = (*itr);

        }
        else
        {
            // Look for switches we want
            if (*itr=="-loadall")
            {
                loadAll = true;
                continue;
            }
            if (*itr=="-thread")
            {
                threadIt = true;
                continue;
            }
        }
    }
    if (fileName.empty()) {
    fprintf(stderr,"No TerraPage file specified on command line.\n");
    return 1;
    }
    // Open the TXP database
    TrPageArchive *txpArchive = new TrPageArchive();
    if (!txpArchive->OpenFile(fileName.c_str()))
    {
        fprintf(stderr,"Couldn't load TerraPage archive %s.\n",fileName.c_str());
        return 1;
    }
    
    // Note: Should be checking the return values
    txpArchive->LoadMaterials();
//    txpArchive->LoadModels();

    // Might need a page manager if we're paging
    OSGPageManager *pageManager = new OSGPageManager(txpArchive);
    osg::Group *rootNode=NULL;
    if (loadAll) {
        // Load the whole scenegraph
        rootNode = txpArchive->LoadAllTiles();
        if (!rootNode) {
            fprintf(stderr,"Couldn't load whole TerraPage archive %s.\n",fileName.c_str());
            return 1;
        }
        // add a viewport to the viewer and attach the scene graph.
        viewer->addViewport( rootNode );
    } else {
        viewer->Init(pageManager,(threadIt ? txp::OSGPageManager::ThreadFree : txp::OSGPageManager::ThreadNone));
        rootNode = new osg::Group();
        viewer->addViewport(rootNode);
    }
    
    // run optimization over the scene graph
//   osgUtil::Optimizer optimzer;
//    optimzer.optimize(rootnode);
     
    
    // register trackball, flight and drive.
    viewer->registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer->registerCameraManipulator(new osgGA::FlightManipulator);
    viewer->registerCameraManipulator(new osgGA::DriveManipulator);

    // Recenter the camera to the middle of the database
    osg::Vec3 center;
    txpArchive->GetCenter(center);  center[2] += 200;
    osgUtil::SceneView *sceneView = viewer->getViewportSceneView(0);
    osg::Camera *camera = sceneView->getCamera();
    osg::Vec3 eyePt = center;
    eyePt[0] -= 1000;
    osg::Vec3 upVec( 0, 0, 1 );
    camera->setLookAt(eyePt,center,upVec);

    // open the viewer window.
    viewer->open();
    
    // fire up the event loop.
    viewer->run();


    // Close things down  
        // (note from Robert Osfield, umm.... we should be using ref_ptr<> for handling memory here, this isn't robust..)
    delete pageManager;
    delete txpArchive;
    delete viewer;

    return 0;
}
