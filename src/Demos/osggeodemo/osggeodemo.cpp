// Geo demo written by Geoff Michel, November 2002.

#include <stdio.h>
#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Node>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/AnimationPathManipulator>

#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgUtil/Optimizer>

// currently not a satisfactory solution, but this is early days for the
// geo loader and having direct links with it. 
#include "../../osgPlugins/geo/osgGeoAnimation.h"

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
    out <<"    -p  pathfile        - Use the AnimationPathManipulator (key binding '4') and use\n"
      "                          pathfile to define the animation path.  pathfile is an ascii\n"
      "                          file containing lines of eight floating point numbers representing\n"
      "                          time, position (x,y,z) and attitude (x,y,z,w as a quaternion)." << std::endl;
    out << std::endl;
}

//== event trapper gets events

class geodemoEventHandler : public osgGA::GUIEventHandler
{
public:
    
    geodemoEventHandler( )     { mouse_x=mouse_y=0;}
    virtual ~geodemoEventHandler( ) {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
    {
        mouse_x=ea.getX();
        mouse_y=ea.getY();
        return false;
        
    }
    
    virtual void accept(osgGA::GUIEventHandlerVisitor& v)
    {
        v.visit(*this);
    }
    inline int getMouseX(void) {return mouse_x;}; 
    inline int getMouseY(void) {return mouse_y;};
    
private:
    int    mouse_x, mouse_y;
};

static geodemoEventHandler *ghand=NULL;
inline double DEG2RAD(const double val) { return val*0.0174532925199432957692369076848861;}
inline double RAD2DEG(const double val) { return val*57.2957795130823208767981548141052;}

double dodynamics(const double time, const double val, const std::string name)
{ // Each local variable named 'name' declared in the geo modeller is passed into here.
    // its current value is val; returns new value.  Time - elapsed time
    static double heading,speed; // these are only required for my 'dynamics'
    if (name == "xpos") {
        return (val+speed*sin(heading));
        //    std::cout << " nx " << (*itr->getValue()) ;
    } else if (name == "ypos") {
        return (val+speed*cos(heading));
        //    std::cout << " ny " << (*itr->getValue()) ;
    } else if (name == "sped") {
        speed=(0.00025*(ghand->getMouseY()-300)); // (*itr->getValue());
        return (speed);
    } else if (name == "heading") {
        heading-= 0.01*DEG2RAD(ghand->getMouseX()-400); // =DEG2RAD(*itr->getValue());
        return (RAD2DEG(heading));
    } else if (name == "conerot") {
        return ((ghand->getMouseX()-400));
    } else if (name == "planrot") {
        return ((ghand->getMouseY()-300)/200.0);
    } else if (name == "secint" || name == "minutehand"|| name == "hourhand") {
    //    std::cout << " updating " << name << " " << val << std::endl;
    }
    return val;
}

int main( int argc, char **argv )
{
    std::string pathfile;
    float camera_fov=-1;

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(std::cout,argv[0]);
        return 1;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) {
    if( std::string(argv[i]) == "-p" ) {
        if( (i+1) >= argc ) {
        write_usage( std::cout, argv[0]);
            return 1;
        }
        else
        pathfile = std::string(argv[++i]);
    } else if( std::string(argv[i]) == "-fov" ) {
        camera_fov=atof(argv[i+1]);
        ++i; // skip the value
    }
    else 
        commandLine.push_back(argv[i]);
    }

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);
    
    
    // get the path of the executable and use it to set internal data and library paths.
    std::string executablePath = osgDB::getFilePath(argv[0]);
    if (!executablePath.empty())
    {
        osg::notify(osg::NOTICE) << "Adding executable path '"<<executablePath<<"' to OpenSceneGraph data and library file paths."<<std::endl;
        osgDB::getDataFilePathList().push_front(executablePath);
        osgDB::getLibraryFilePathList().push_front(executablePath);
    }

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
        write_usage(osg::notify(osg::NOTICE),argv[0]);
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

    if( !pathfile.empty() ) {
    osgGA::AnimationPathManipulator *apm = new osgGA::AnimationPathManipulator(pathfile);
    if( apm->valid() ) 
            viewer.registerCameraManipulator(apm);
    else
        delete apm;
    }

    geoHeader *gh = dynamic_cast<geoHeader *>(rootnode);
    if (gh) { // it is a geo file, so set function to update its animation variables.
        ghand=new geodemoEventHandler();
        gh->setUserUpdate(dodynamics);
        viewer.prependEventHandler(ghand);
    } else { // maybe a group with geo models below.
        osg::Group *gpall=dynamic_cast<osg::Group *>(rootnode);
        if (gpall) {
            int nchild=gpall->getNumChildren();
            for (int i=0; i<nchild; i++) {
                osg::Node *nod=gpall->getChild(i);
                gh = dynamic_cast<geoHeader *>(nod);
                if (gh)
                {
                    ghand=new geodemoEventHandler();
                    gh->setUserUpdate(dodynamics);
                    viewer.prependEventHandler(ghand);
                }
            }
        }
    }
    osgUtil::SceneView *sc=viewer.getViewportSceneView(0);
    if (sc && camera_fov>0) {
        osg::Camera *cm=sc->getCamera();
        if (cm) cm->setFOV(camera_fov,camera_fov*(600.0f/800.0f),1.0f,1000.0f);
    }
        // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
