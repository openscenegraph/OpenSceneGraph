#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>
#include <osg/CoordinateSystemNode>
#include <osgText/Text>

#include <osgManipulator/CommandManager>
#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabPlaneDragger>
#include <osgManipulator/TabPlaneTrackballDragger>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/Translate1DDragger>
#include <osgManipulator/Translate2DDragger>
#include <osgManipulator/TranslateAxisDragger>

osgManipulator::Dragger* createDragger(const std::string& name)
{
    osgManipulator::Dragger* dragger = 0;
    if ("TabPlaneDragger" == name)
    {
        osgManipulator::TabPlaneDragger* d = new osgManipulator::TabPlaneDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("TabPlaneTrackballDragger" == name)
    {
        osgManipulator::TabPlaneTrackballDragger* d = new osgManipulator::TabPlaneTrackballDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("TrackballDragger" == name)
    {
        osgManipulator::TrackballDragger* d = new osgManipulator::TrackballDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("Translate1DDragger" == name)
    {
        osgManipulator::Translate1DDragger* d = new osgManipulator::Translate1DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("Translate2DDragger" == name)
    {
        osgManipulator::Translate2DDragger* d = new osgManipulator::Translate2DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("TranslateAxisDragger" == name)
    {
        osgManipulator::TranslateAxisDragger* d = new osgManipulator::TranslateAxisDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else
    {
        osgManipulator::TabBoxDragger* d = new osgManipulator::TabBoxDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
 
    return dragger;
}

osg::Node* createHUD()
{
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osgText::Text* text = new  osgText::Text;
    geode->addDrawable( text );

    osg::Vec3 position(50.0f,50.0f,0.0f);
    text->setPosition(position);
    text->setText("Use the Tab key to switch between the trackball and pick modes.");
    text->setFont(timesFont);

    osg::Camera* camera = new osg::Camera;

    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));

    // set the view matrix    
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    camera->addChild(geode);
    
    return camera;
}


bool computePixelCoords(osgProducer::Viewer* viewer,float x,float y,unsigned int cameraNum,float& pixel_x,float& pixel_y)
{
    Producer::KeyboardMouse* km = viewer->getKeyboardMouse();
    if (!km) return false;
	    
    if (cameraNum>=viewer->getNumberOfCameras()) return false;

    Producer::Camera* camera=viewer->getCamera(cameraNum);
    Producer::RenderSurface* rs = camera->getRenderSurface();

    Producer::InputArea* inputArea = km->getInputArea();
    if( inputArea != NULL )
    {
        // first locate which InputRectange is appropriate for specified RenderSurface.
        unsigned int i;
        for(i=0;i<inputArea->getNumRenderSurfaces();++i)
        {
            if (inputArea->getRenderSurface(i)==rs) break;
        }
        
        // the RenderSurface isn't in the InputArea list.. therefore cannot make mouse coords to it.
        if (i==inputArea->getNumRenderSurfaces()) return false;

        // we have a valid InputRectangle
	Producer::RenderSurface::InputRectangle ir = inputArea->getRenderSurface(i)->getInputRectangle();

        float rx = (x-ir.left())/ir.width();
        float ry = (y-ir.bottom())/ir.height();
        
        int wx, wy;
        unsigned int w, h;
        rs->getWindowRectangle( wx, wy, w, h );

        pixel_x = ((float)w)* rx;
        pixel_y = ((float)h)* ry;
    }
    else
    {
        float rx = (x+1.0f)*0.5f;
        float ry = (y+1.0f)*0.5f;

        int wx, wy;
        unsigned int w, h;
        rs->getWindowRectangle( wx, wy, w, h );

        pixel_x = ((float)w)* rx;
        pixel_y = ((float)h)* ry;
    }
    return true;
}

osg::Node* addDraggerToScene(osg::Node* scene, osgManipulator::CommandManager* cmdMgr, const std::string& name)
{
    scene->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

    osgManipulator::Selection* selection = new osgManipulator::Selection;
    selection->addChild(scene);

    osgManipulator::Dragger* dragger = createDragger(name);

    osg::Group* root = new osg::Group;
    root->addChild(dragger);
    root->addChild(selection);
    root->addChild(createHUD());

    float scale = scene->getBound().radius() * 1.6;
    dragger->setMatrix(osg::Matrix::scale(scale, scale, scale) *
                       osg::Matrix::translate(scene->getBound().center()));
    cmdMgr->connect(*dragger, *selection);

    return root;
}

class PickModeHandler : public osgGA::GUIEventHandler
{
    public:
        enum Modes
        {
            VIEW = 0,
            PICK
        };

        PickModeHandler(osgProducer::Viewer *viewer) : osgGA::GUIEventHandler(),
                                                       _viewer(viewer), _mode(VIEW), _activeDragger(0)
        {
        }        
        
        bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa,
                    osg::Object*, osg::NodeVisitor*)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Tab &&
                ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN &&
                _activeDragger == 0)
            {
                _mode = ! _mode;
            }
            
            if (VIEW == _mode) return false;

            for(unsigned int i=0;i<_viewer->getNumberOfCameras();++i)
            {
                if ((ea.getEventType() == osgGA::GUIEventAdapter::PUSH) &&
                    _viewer->computeIntersections(ea.getX(), ea.getY(), i, hitList))
                {
                    float pixel_x,pixel_y;
                    if (computePixelCoords(_viewer,ea.getX(),ea.getY(),i,pixel_x,pixel_y))
                    {
                        Producer::Camera* camera=_viewer->getCamera(i);

                        osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
                        osgUtil::SceneView* sv = sh ? sh->getSceneView() : 0;
                        if (! sv) continue;

                        for (osg::NodePath::iterator itr = hitList.front().getNodePath().begin();
                             itr != hitList.front().getNodePath().end();
                             ++itr)
                        {
                            osgManipulator::Dragger* dragger = dynamic_cast<osgManipulator::Dragger*>(*itr);
                            if (dragger)
                            {
                                dragger->handle(int(pixel_x+0.5), int(pixel_y+0.5), *sv,
                                                hitList, hitList.begin(),
                                                ea, aa);
                                _activeDragger = dragger;
                                break;
                            }                   
                        }
                    }
                }
                
                switch (ea.getEventType())
                {
                    case osgGA::GUIEventAdapter::DRAG:
                    case osgGA::GUIEventAdapter::RELEASE:
                        if (_activeDragger)
                        {
                            float pixel_x,pixel_y;
                            if (computePixelCoords(_viewer,ea.getX(),ea.getY(),i,pixel_x,pixel_y))
                            {
                                Producer::Camera* camera=_viewer->getCamera(i);

                                osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
                                osgUtil::SceneView* sv = sh ? sh->getSceneView() : 0;
                                if (_activeDragger && sv)
                                    _activeDragger->handle(int(pixel_x+0.5), int(pixel_y+0.5), *sv,
                                                           hitList, hitList.begin(),
                                                           ea, aa);
                            }
                        }
                        break;

		    default:
			break;
                }
                if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
                {
                    _activeDragger = 0;
                    hitList.clear();
                }
            }
            return true;
        }
        
    private:
        osgProducer::Viewer* _viewer;
        unsigned int _mode;
        osgManipulator::Dragger* _activeDragger;
        osgUtil::IntersectVisitor::HitList hitList;
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line parameters");
    arguments.getApplicationUsage()->addCommandLineOption("--help-env","Display environmental variables available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-keys","Display keyboard & mouse bindings available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-all","Display all command line, env vars and keyboard & mouse bindings.");

    arguments.getApplicationUsage()->addCommandLineOption("--dragger <draggername>","Use the specified dragger for manipulation");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    bool helpAll = arguments.read("--help-all");
    unsigned int helpType = ((helpAll || arguments.read("-h") || arguments.read("--help"))? osg::ApplicationUsage::COMMAND_LINE_OPTION : 0 ) |
                            ((helpAll ||  arguments.read("--help-env"))? osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE : 0 ) |
                            ((helpAll ||  arguments.read("--help-keys"))? osg::ApplicationUsage::KEYBOARD_MOUSE_BINDING : 0 );
    if (helpType)
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

    std::string dragger_name = "TabBoxDragger";
    arguments.read("--dragger", dragger_name);

    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // read the scene from the list of file specified command line args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
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
    }

    osg::Timer_t end_tick = osg::Timer::instance()->tick();

    std::cout << "Time to load = "<<osg::Timer::instance()->delta_s(start_tick,end_tick)<<std::endl;


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    osg::ref_ptr<osgManipulator::CommandManager> cmdMgr = new osgManipulator::CommandManager;
    
    // pass the loaded scene graph to the viewer.
    viewer.setSceneData(addDraggerToScene(loadedModel.get(), cmdMgr.get(), dragger_name));

    viewer.getEventHandlerList().push_front(new PickModeHandler(&viewer));

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

