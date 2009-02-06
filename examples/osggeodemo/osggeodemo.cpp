// Geo demo written by Geoff Michel, November 2002.
/* OpenSceneGraph example, osggeodemo.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <stdio.h>
#include <osgViewer/Viewer>

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

#include <iostream>

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

    // construct the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
    if (!rootnode)
    {
        osg::notify(osg::NOTICE)<<"Please specify a geo model filename on the command line."<<std::endl;
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
        viewer.addEventHandler(ghand);
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
                    viewer.addEventHandler(ghand);
                }
            }
        }
    }

    return viewer.run();
}
