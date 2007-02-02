/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <iostream>

class MotionBlurOperation: public osg::Operation
{
public:
    MotionBlurOperation(double persistence):
        osg::Operation("MotionBlur",true),
        cleared_(false),
        persistence_(persistence)
    {
    }

    virtual void operator () (osg::Object* object)
    {
        osg::GraphicsContext* gc = dynamic_cast<osg::GraphicsContext*>(object);
        if (!gc) return;
    
        double t = gc->getState()->getFrameStamp()->getSimulationTime();

        if (!cleared_)
        {
            // clear the accumulation buffer
            glClearColor(0, 0, 0, 0);
            glClear(GL_ACCUM_BUFFER_BIT);
            cleared_ = true;
            t0_ = t;
        }

        double dt = fabs(t - t0_);
        t0_ = t;

        // compute the blur factor
        double s = powf(0.2, dt / persistence_);

        // scale, accumulate and return
        glAccum(GL_MULT, s);
        glAccum(GL_ACCUM, 1 - s);
        glAccum(GL_RETURN, 1.0f);
    }

private:
    bool cleared_;
    double t0_;
    double persistence_;
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an OpenSceneGraph example that shows how to use the accumulation buffer to achieve a simple motion blur effect.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-P or --persistence","Set the motion blur persistence time");
    

    // construct the viewer.
    osgViewer::Viewer viewer;

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    double persistence = 0.25;
    arguments.read("-P", persistence) || arguments.read("--persistence", persistence);

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }


    // set the display settings we can to request, OsgCameraGroup will read this.
    osg::DisplaySettings::instance()->setMinimumNumAccumBits(8,8,8,8);

    // pass the loaded scene graph to the viewer.
    viewer.setSceneData(loadedModel.get());

    // create the windows and run the threads.
    viewer.realize();

    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        (*itr)->add(new MotionBlurOperation(persistence));
    }

    return viewer.run();
}
