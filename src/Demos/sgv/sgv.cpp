#include <osg/Transform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osgUtil/Optimizer>


int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        osg::notify(osg::NOTICE)<<"usage:"<< std::endl;
        osg::notify(osg::NOTICE)<<"    sgv [options] infile1 [infile2 ...]"<< std::endl;
        osg::notify(osg::NOTICE)<< std::endl;
        osg::notify(osg::NOTICE)<<"options:"<< std::endl;
        osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<< std::endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<< std::endl;
        osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<< std::endl;
        osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<< std::endl;
        osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<< std::endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<< std::endl;
        osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<< std::endl;
        osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<< std::endl;
        osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<< std::endl;
        osg::notify(osg::NOTICE)<< std::endl;

        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(commandLine);
    if (!rootnode) return 0;
    
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
