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
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Use specificed animation path file for camera animation");
   
    // read the commandline args.
    std::string pathfile;
    while (arguments.read("-p",pathfile)) {}

    float camera_fov=-1;
    while (arguments.read("-fov",camera_fov)) {}

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


    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
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
