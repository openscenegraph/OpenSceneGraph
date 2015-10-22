/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial applications,
 * as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <iostream>


// The idea of user stats is that you record times or values in the viewer's
// stats, and you also tell the stats handler to watch those values each
// frame. The stats handler can display the stats in three ways:
//  - A numeric time beside the stat name
//    Requires that an elapsed time be recorded in the viewer's stats for the
//    "timeTakenName".
//  - A bar in the top bar graph
//    Requires that two times (relative to the viewer's start tick) be
//    recorded in the viewer's stats for the "beginTimeName" and "endTimeName".
//  - A line in the bottom graph
//    Requires that an elapsed time be recorded in the viewer's stats for the
//    "timeTakenName".


// Anything you want to time has to use a consistent name in both the stats
// handler and the viewer stats, so it's a good idea to use constants to make
// sure the names are the same everywhere.
const std::string frameNumberName    = "Custom Frame Number";
const std::string frameTimeName      = "Custom Frame Time";
const std::string customTimeName     = "Custom";
const std::string operation1TimeName = "Operation1";
const std::string operation2TimeName = "Operation2";
const std::string otherThreadTimeName = "Thread";


void initUserStats(osgViewer::StatsHandler* statsHandler)
{
    // This line displays the frame number. It's not averaged, just displayed as is.
    statsHandler->addUserStatsLine("Frame", osg::Vec4(0.7,0.7,0.7,1), osg::Vec4(0.7,0.7,0.7,0.5),
                                   frameNumberName, 1.0, false, false, "", "", 0.0);

    // This line displays the frame time (from beginning of event to end of draw). No bars.
    statsHandler->addUserStatsLine("MS/frame", osg::Vec4(1,0,1,1), osg::Vec4(1,0,1,0.5),
                                   frameTimeName, 1000.0, true, false, "", "", 0.02);

    // This line displays the sum of update and main camera cull times.
    statsHandler->addUserStatsLine("Custom", osg::Vec4(1,1,1,1), osg::Vec4(1,1,1,0.5),
                                   customTimeName + " time taken", 1000.0, true, false, customTimeName + " begin", customTimeName + " end", 0.016);

    // This line displays the time taken by a function below ( doSomethingAndTimeIt() )
    statsHandler->addUserStatsLine("Sleep1", osg::Vec4(1,0,0,1), osg::Vec4(1,0,0,0.5),
                                   operation1TimeName + " time taken", 1000.0, true, false, operation1TimeName + " begin", operation1TimeName + " end", 0.016);

    // This line displays the time taken by a function below ( doSomethingAndTimeIt() )
    statsHandler->addUserStatsLine("Sleep2", osg::Vec4(1,0.5,0.5,1), osg::Vec4(1,0.5,0.5,0.5),
                                   operation2TimeName + " time taken", 1000.0, true, false, operation2TimeName + " begin", operation2TimeName + " end", 0.016);

    // This line displays the time taken by a function below ( doSomethingAndTimeIt() )
    statsHandler->addUserStatsLine("Thread", osg::Vec4(0,0.5,0,1), osg::Vec4(0,0.5,0,0.5),
                                   otherThreadTimeName + " time taken", 1000.0, true, false, otherThreadTimeName + " begin", otherThreadTimeName + " end", 0.016);
}


void updateUserStats(osgViewer::Viewer& viewer)
{
    // Test the custom stats line by just adding up the update and cull
    // times for the viewer main camera for the previous frame.
    if (viewer.getViewerStats()->collectStats("update") && viewer.getCamera()->getStats()->collectStats("rendering"))
    {
        // First get the frame number. The code below assumes that
        // updateUserStats() is called after advance(), so the frame number
        // that will be returned is for the frame that has just started and is
        // not rendered yet. The previous frame is framenumber-1, but we can't
        // use that frame's timings because it's probably not finished
        // rendering yet (in multithreaded viewer modes). So we'll use the
        // timings for framenumber-2 for this demo.
        unsigned int framenumber = viewer.getFrameStamp()->getFrameNumber();

        // Get the update time and the viewer main camera's cull time. We use
        // getAveragedAttribute() in order to get the actual time elapsed as
        // calculated by the stats.
        double update = 0.0, cull = 0.0;
        viewer.getViewerStats()->getAveragedAttribute("Update traversal time taken", update);
        viewer.getCamera()->getStats()->getAveragedAttribute("Cull traversal time taken", cull);

        // Get various begin and end times, note these are not elapsed times
        // in a frame but rather the simulation time at those moments.
        double eventBegin = 0.0, updateBegin = 0.0, cullEnd = 0.0, drawEnd = 0.0;
        viewer.getViewerStats()->getAttribute(framenumber-2, "Event traversal begin time", eventBegin);
        viewer.getViewerStats()->getAttribute(framenumber-2, "Update traversal begin time", updateBegin);
        viewer.getCamera()->getStats()->getAttribute(framenumber-2, "Cull traversal end time", cullEnd);
        viewer.getCamera()->getStats()->getAttribute(framenumber-2, "Draw traversal end time", drawEnd);

        // This line displays the frame number. It's not averaged, just displayed as is.
        viewer.getViewerStats()->setAttribute(framenumber, frameNumberName, framenumber);

        // This line displays the frame time (from beginning of event to end of draw). No bars.
        viewer.getViewerStats()->setAttribute(framenumber-1, frameTimeName, drawEnd - eventBegin);

        // This line displays the sum of update and main camera cull times.
        viewer.getViewerStats()->setAttribute(framenumber-1, customTimeName + " time taken", update+cull);
        // Since we give begin and end times that correspond to the begin of
        // the update phase and the end of the cull phase, the bar in the
        // graph will not correspond to the summed times above if something
        // happened between update and cull (as in this demo). Also, we need
        // to translate the updateBegin and cullEnd times by one frame since
        // we're taking the times for framenumber-2 but using them to display
        // in the framenumber-1 graph.
        viewer.getViewerStats()->setAttribute(framenumber-1, customTimeName + " begin", updateBegin + (1.0/60.0));
        viewer.getViewerStats()->setAttribute(framenumber-1, customTimeName + " end", cullEnd + (1.0/60.0));
    }
}


/// Utility function you call before something you want to time, so that the
/// recorded times will all be consistent using the viewer's time.
void startTiming(osgViewer::Viewer& viewer, const std::string& name)
{
    osg::Timer_t tick = osg::Timer::instance()->tick();
    double currentTime = osg::Timer::instance()->delta_s(viewer.getStartTick(), tick);
    unsigned int framenumber = viewer.getFrameStamp()->getFrameNumber();

    viewer.getViewerStats()->setAttribute(framenumber, name + " begin", currentTime);
}

/// Utility function you call after something you want to time, so that the
/// recorded times will all be consistent using the viewer's time.
void endTiming(osgViewer::Viewer& viewer, const std::string& name)
{
    osg::Timer_t tick = osg::Timer::instance()->tick();
    double currentTime = osg::Timer::instance()->delta_s(viewer.getStartTick(), tick);
    unsigned int framenumber = viewer.getFrameStamp()->getFrameNumber();

    viewer.getViewerStats()->setAttribute(framenumber, name + " end", currentTime);

    double begin = 0.0;
    double elapsed = 0.0;
    if (viewer.getViewerStats()->getAttribute(framenumber, name + " begin", begin))
    {
        elapsed = currentTime - begin;
    }

    viewer.getViewerStats()->setAttribute(framenumber, name + " time taken", elapsed);
}


/// Will just sleep for the given number of milliseconds in the same thread
/// as the caller, recording the time taken in the viewer's stats.
void doSomethingAndTimeIt(osgViewer::Viewer& viewer, const std::string& name, double milliseconds)
{
    startTiming(viewer, name);

    //------------------------------------------------------------
    // Your processing goes here.

    // Do nothing for the specified number of  milliseconds, just so we can
    // see it in the stats.
    osg::Timer_t startTick = osg::Timer::instance()->tick();
    while (osg::Timer::instance()->delta_m(startTick, osg::Timer::instance()->tick()) < milliseconds)
    {
        OpenThreads::Thread::YieldCurrentThread();
    }
    //------------------------------------------------------------

    endTiming(viewer, name);
}


/// Thread that will sleep for the given number of milliseconds, recording
/// the time taken in the viewer's stats, whenever its process() method is
/// called.
class UselessThread : public OpenThreads::Thread
{
public:
    UselessThread(osgViewer::Viewer& viewer, double timeToRun)
        : _viewer(viewer)
        , _timeToRun(timeToRun)
        , _done(false)
        , _process(false)
    {
    }

    void run()
    {
        while (!_done)
        {
            if (_process)
            {
                startTiming(_viewer, otherThreadTimeName);

                //------------------------------------------------------------
                // Your processing goes here.

                // Do nothing for the specified number of  milliseconds, just so we can
                // see it in the stats.
                osg::Timer_t startTick = osg::Timer::instance()->tick();
                while (osg::Timer::instance()->delta_m(startTick, osg::Timer::instance()->tick()) < _timeToRun)
                {
                    OpenThreads::Thread::YieldCurrentThread();
                }
                //------------------------------------------------------------

                endTiming(_viewer, otherThreadTimeName);

                _process = false;
            }
            else
            {
                OpenThreads::Thread::microSleep(50);
            }
        }
    }

    int cancel()
    {
        _done = true;
        return OpenThreads::Thread::cancel();
    }

    void process()
    {
        _process = true;
    }

protected:
    osgViewer::Viewer& _viewer;
    double _timeToRun;
    bool _done;
    bool _process;
};


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("--login <url> <username> <password>","Provide authentication information for http file access.");

    osgViewer::Viewer viewer(arguments);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
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

    std::string url, username, password;
    while(arguments.read("--login",url, username, password))
    {
        if (!osgDB::Registry::instance()->getAuthenticationMap())
        {
            osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
            osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(
                url,
                new osgDB::AuthenticationDetails(username, password)
            );
        }
    }

    viewer.setCameraManipulator(new osgGA::TrackballManipulator);

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the thread model handler
    viewer.addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    osgViewer::StatsHandler* statsHandler = new osgViewer::StatsHandler;
    viewer.addEventHandler(statsHandler);

    initUserStats(statsHandler);

    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel);

    viewer.setSceneData(loadedModel);

    viewer.realize();

    // Start up a thread that will just run for a fixed time each frame, in
    // parallel to the frame loop.
    UselessThread thread(viewer, 6.0);
    thread.start();

    while (!viewer.done())
    {
        viewer.advance();

        updateUserStats(viewer);

        // Eat up some time on the viewer thread before the event phase.
        doSomethingAndTimeIt(viewer, operation1TimeName, 2.0);

        // Start taking some time on the other thread.
        thread.process();

        viewer.eventTraversal();
        viewer.updateTraversal();

        // Eat up some time on the viewer thread between the update and cull
        // phases.
        doSomethingAndTimeIt(viewer, operation2TimeName, 3.0);

        viewer.renderingTraversals();
    }

    thread.cancel();
    thread.join();

}
