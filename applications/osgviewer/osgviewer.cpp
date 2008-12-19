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
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osgText/Text>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <iostream>


void addEventHandlers(osgViewer::View* view)
{
    // set up the camera manipulators.
    view->setCameraManipulator( new osgGA::TrackballManipulator() );

    // add the state manipulator
    view->addEventHandler( new osgGA::StateSetManipulator(view->getCamera()->getOrCreateStateSet()) );
    
    // add the thread model handler
    view->addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    view->addEventHandler(new osgViewer::WindowSizeHandler);
        
    // add the stats handler
    view->addEventHandler(new osgViewer::StatsHandler);

    // add the record camera path handler
    view->addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the LOD Scale handler
    view->addEventHandler(new osgViewer::LODScaleHandler);

    // add the screen capture handler
    view->addEventHandler(new osgViewer::ScreenCaptureHandler);
}


class AddViewHandler : public osgGA::GUIEventHandler
{
public:
    AddViewHandler(osgViewer::CompositeViewer* viewer, osg::Node* sceneRoot) 
        : _viewer(viewer), _sceneRoot(sceneRoot) {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN && ea.getKey()== 'a')
        {
            osgViewer::View* view = new osgViewer::View;

            view->setUpViewInWindow(50, 50, 800, 600);
            view->getCamera()->getGraphicsContext()->realize();

            view->setSceneData(_sceneRoot.get());
            addEventHandlers(view);

            _viewer->stopThreading();

            _viewer->addView(view);

            osg::notify(osg::NOTICE)<<"osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts()="<<  osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts()<<std::endl;

            view->getSceneData()->setThreadSafeRefUnref(true);
            view->getSceneData()->resizeGLObjectBuffers(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());

            _viewer->startThreading();

            return true;
        }

        return false;
    }

protected:
    osg::observer_ptr<osgViewer::CompositeViewer> _viewer;
    osg::ref_ptr<osg::Node>                  _sceneRoot;
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

    osgViewer::CompositeViewer viewer(arguments);

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

    osg::ref_ptr<osgViewer::View> defaultView = new osgViewer::View;
    viewer.addView(defaultView.get());
    addEventHandlers(defaultView.get());

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
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
    //osgUtil::Optimizer optimizer;
    //optimizer.optimize(loadedModel.get());

    defaultView->setSceneData( loadedModel.get() );

    defaultView->setUpViewInWindow(50, 50, 800, 600);
    defaultView->addEventHandler(new AddViewHandler(&viewer, loadedModel.get()));

    viewer.realize();

    return viewer.run();
}
