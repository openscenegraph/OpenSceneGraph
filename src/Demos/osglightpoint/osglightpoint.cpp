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
    
    osg::Transform* transform = osgNew osg::Transform;

    osg::Vec3 start_delta(0.0f,10.0f,0.0f);
    osg::Vec3 end_delta(0.0f,10.0f,1.0f);

    int noStepsX = 100;
    int noStepsY = 100;

//     osgSim::BlinkSequence* bs = osgNew osgSim::BlinkSequence;
//     bs->addPulse(1.0,osg::Vec4(1.0f,0.0f,0.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
//     bs->addPulse(1.5,osg::Vec4(1.0f,1.0f,0.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
//     bs->addPulse(1.0,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
    

//    osgSim::Sector* sector = osgNew osgSim::ConeSector(osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0),osg::inDegrees(45.0));
//    osgSim::Sector* sector = osgNew osgSim::ElevationSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(45.0));
//    osgSim::Sector* sector = osgNew osgSim::AzimSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(90.0));
//     osgSim::Sector* sector = osgNew osgSim::AzimElevationSector(osg::inDegrees(180),osg::inDegrees(90), // azim range
//                                                                 osg::inDegrees(0.0),osg::inDegrees(90.0), // elevation range
//                                                                 osg::inDegrees(5.0));

    for(int i=0;i<noStepsY;++i)
    {

//         osgSim::BlinkSequence* local_bs = osgNew osgSim::BlinkSequence(*bs);
//         local_bs->setSequenceGroup(osgNew osgSim::BlinkSequence::SequenceGroup((double)i*0.1));        
//         start._blinkSequence = local_bs;

//        start._sector = sector;

        osgSim::LightPointNode* lpn = osgNew osgSim::LightPointNode;
        addToLightPointNode(*lpn,start,end,noStepsX);
        
        start._position += start_delta;
        end._position += end_delta;
        
        transform->addChild(lpn);    
    }
        
    osg::Group* group = osgNew osg::Group;
    group->addChild(transform);
    
    
    return group;
}


int main( int argc, char **argv )
{

    // initialize the GLUT
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

    osg::Group* rootnode = osgNew osg::Group;

    // load the nodes from the commandline arguments.
    rootnode->addChild(osgDB::readNodeFiles(commandLine));
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
