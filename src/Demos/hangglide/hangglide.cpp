#include <osg/Group>
#include <osg/Notify>
#include <osg/Depth>
#include <osg/StateSet>
#include <osg/EarthSky>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include "GliderManipulator.h"

extern osg::Node *makeTerrain( void );
extern osg::Node *makeTrees( void );
extern osg::Node *makeTank( void );
extern osg::Node *makeWindsocks( void );
extern osg::Node *makeGliders( void );
extern osg::Node *makeGlider( void );
extern osg::Node *makeSky( void );
extern osg::Node *makeBase( void );
extern osg::Node *makeClouds( void );


int main( int argc, char **argv )
{
    glutInit( &argc, argv );

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
    osg::Node* rootnode = osgDB::readNodeFiles(commandLine);
    if (!rootnode)
    {
        // no database loaded so automatically create Ed Levin Park..
        osg::Group* group = new osg::Group;
        rootnode = group;
        
        // the base and sky subgraphs go to set the earth sky of the
        // model and clear the color and depth buffer for us, by using
        // osg::Depth, and setting their bin numbers to less than 0,
        // to force them to draw before the rest of the scene.
  
        osg::EarthSky* earthSky = new osg::EarthSky;
        earthSky->setRequiresClear(false); // we've got base and sky to do it.
        earthSky->addChild(makeSky());  // bin number -2 so drawn first.
        earthSky->addChild(makeBase()); // bin number -1 so draw second.      
        
        group->addChild(earthSky);

        // the rest of the scene drawn after the base and sky above.
        group->addChild(makeTrees()); // will drop into a transparent, depth sorted bin (1)
        group->addChild(makeTerrain()); // will drop into default bin - state sorted 0
        group->addChild(makeTank()); // will drop into default bin - state sorted 0
        // add the following in the future...
        // makeGliders
        // makeClouds

    }

    viewer.addViewport( rootnode );

    unsigned int pos = viewer.registerCameraManipulator(new GliderManipulator());

    // Open window so camera manipulator's warp pointer request will succeed
    viewer.open();

    viewer.selectCameraManipulator(pos);

    viewer.run();

    return 0;
}
