#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Transform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>

#include <osgSim/LightPointNode>


#define INTERPOLATE(member) lp.member = start.member*rstart + end.member*rend;

void addToLightPointNode(osgSim::LightPointNode& lpn,osgSim::LightPoint& start,osgSim::LightPoint& end,unsigned int noSteps)
{
    if (noSteps<=1)
    {
        lpn.addLightPoint(start);
        return;
    }
    
    float rend = 0.0f;
    float rdelta = 1.0f/((float)noSteps-1.0f);
    
    lpn._lightPointList.reserve(noSteps);
    
    for(unsigned int i=0;i<noSteps;++i,rend+=rdelta)
    {
        float rstart = 1.0f-rend;
        osgSim::LightPoint lp(start);
        INTERPOLATE(_position)
        INTERPOLATE(_intensity);
        INTERPOLATE(_color);
        INTERPOLATE(_radius);
        //INTERPOLATE(_minPixelSize);
        //INTERPOLATE(_maxPixelSize);
//        INTERPOLATE(_maxVisibileDistance2);

        lpn.addLightPoint(lp);
        
   }
}

#undef INTERPOLATE

osg::Node* createLightPointsDatabase()
{
    osgSim::LightPoint start;
    osgSim::LightPoint end;

    start._position.set(0.0f,0.0f,0.0f);
    start._color.set(1.0f,0.0f,0.0f,1.0f);
    
    end._position.set(1000.0f,0.0f,0.0f);
    end._color.set(1.0f,1.0f,1.0f,1.0f);
    
    osg::Transform* transform = new osg::Transform;

    osg::Vec3 start_delta(0.0f,10.0f,0.0f);
    osg::Vec3 end_delta(0.0f,10.0f,1.0f);

    int noStepsX = 100;
    int noStepsY = 100;

//     osgSim::BlinkSequence* bs = new osgSim::BlinkSequence;
//     bs->addPulse(1.0,osg::Vec4(1.0f,0.0f,0.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
//     bs->addPulse(1.5,osg::Vec4(1.0f,1.0f,0.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
//     bs->addPulse(1.0,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
    

//    osgSim::Sector* sector = new osgSim::ConeSector(osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0),osg::inDegrees(45.0));
//    osgSim::Sector* sector = new osgSim::ElevationSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(45.0));
//    osgSim::Sector* sector = new osgSim::AzimSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(90.0));
//     osgSim::Sector* sector = new osgSim::AzimElevationSector(osg::inDegrees(180),osg::inDegrees(90), // azim range
//                                                                 osg::inDegrees(0.0),osg::inDegrees(90.0), // elevation range
//                                                                 osg::inDegrees(5.0));

    for(int i=0;i<noStepsY;++i)
    {

//         osgSim::BlinkSequence* local_bs = new osgSim::BlinkSequence(*bs);
//         local_bs->setSequenceGroup(new osgSim::BlinkSequence::SequenceGroup((double)i*0.1));        
//         start._blinkSequence = local_bs;

//        start._sector = sector;

        osgSim::LightPointNode* lpn = new osgSim::LightPointNode;
        addToLightPointNode(*lpn,start,end,noStepsX);
        
        start._position += start_delta;
        end._position += end_delta;
        
        transform->addChild(lpn);    
    }
        
    osg::Group* group = new osg::Group;
    group->addChild(transform);
    
    
    return group;
}


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

    // initialize the viewer.
    osgGLUT::Viewer viewer(arguments);

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


    osg::Group* rootnode = new osg::Group;

    // load the nodes from the commandline arguments.
    rootnode->addChild(osgDB::readNodeFiles(arguments));
    rootnode->addChild(createLightPointsDatabase());
    if (!rootnode)
    {
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
