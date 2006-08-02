// Geo demo written by Geoff Michel, November 2002.

#include <stdio.h>
#include <osgProducer/Viewer>

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
#include "../../src/osgPlugins/geo/osgGeoAnimation.h"


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
    inline float getMouseX(void) {return mouse_x;}; 
    inline float getMouseY(void) {return mouse_y;};
    
private:
    float    mouse_x, mouse_y;
};

static geodemoEventHandler *ghand=NULL;
inline double DEG2RAD(const double val) { return val*0.0174532925199432957692369076848861;}
inline double RAD2DEG(const double val) { return val*57.2957795130823208767981548141052;}

double dodynamics(const double /*time*/, const double val, const std::string name)
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
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example of how to control animation in Geo files.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--fov <float>","Set the Field of View");
   
    float camera_fov=-1;
    while (arguments.read("--fov",camera_fov)) {}

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
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );

    geoHeader *gh = dynamic_cast<geoHeader *>(rootnode);
    if (gh)
    { // it is a geo file, so set function to update its animation variables.
        ghand=new geodemoEventHandler();
        gh->setUserUpdate(dodynamics);
        viewer.getEventHandlerList().push_front(ghand);
    }
    else
    { // maybe a group with geo models below.
        osg::Group *gpall=dynamic_cast<osg::Group *>(rootnode);
        if (gpall)
        {
            int nchild=gpall->getNumChildren();
            for (int i=0; i<nchild; i++)
            {
                osg::Node *nod=gpall->getChild(i);
                gh = dynamic_cast<geoHeader *>(nod);
                if (gh)
                {
                    ghand=new geodemoEventHandler();
                    gh->setUserUpdate(dodynamics);
                    viewer.getEventHandlerList().push_front(ghand);
                }
            }
        }
    }

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
