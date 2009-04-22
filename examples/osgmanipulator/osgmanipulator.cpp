/* OpenSceneGraph example, osgmanipulator.
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

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
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

#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Material>

#include <iostream>

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

osg::Node* createDemoScene(osgManipulator::CommandManager* cmdMgr) {
 
    osg::Group* root = new osg::Group;

    osg::ref_ptr<osg::Geode> geode_1 = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform> transform_1 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> geode_2 = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform> transform_2 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> geode_3 = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform> transform_3 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> geode_4 = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform> transform_4 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> geode_5 = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform> transform_5 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> geode_6 = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform> transform_6 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> geode_7 = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform> transform_7 = new osg::MatrixTransform;

 



    const float radius = 0.8f;
    const float height = 1.0f;
    osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
    hints->setDetailRatio(2.0f);
    osg::ref_ptr<osg::ShapeDrawable> shape;

    shape = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f, 0.0f, -2.0f), 10, 10.0f, 0.1f), hints.get());
    shape->setColor(osg::Vec4(0.5f, 0.5f, 0.7f, 1.0f));
    geode_1->addDrawable(shape.get());

    shape = new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(0.0f, 0.0f, 0.0f), radius * 2,radius), hints.get());
    shape->setColor(osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f));
    geode_2->addDrawable(shape.get());

    shape = new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(-3.0f, 0.0f, 0.0f), radius,radius), hints.get());
    shape->setColor(osg::Vec4(0.6f, 0.8f, 0.8f, 1.0f));
    geode_3->addDrawable(shape.get());

    shape = new osg::ShapeDrawable(new osg::Cone(osg::Vec3(3.0f, 0.0f, 0.0f), 2 * radius,radius), hints.get());
    shape->setColor(osg::Vec4(0.4f, 0.9f, 0.3f, 1.0f));
    geode_4->addDrawable(shape.get());

    shape = new osg::ShapeDrawable(new osg::Cone(osg::Vec3(0.0f, -3.0f, 0.0f), radius, height), hints.get());
    shape->setColor(osg::Vec4(0.2f, 0.5f, 0.7f, 1.0f));
    geode_5->addDrawable(shape.get());

    shape = new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(0.0f, 3.0f, 0.0f), radius, height), hints.get());
    shape->setColor(osg::Vec4(1.0f, 0.3f, 0.3f, 1.0f));
    geode_6->addDrawable(shape.get());

    shape = new osg::ShapeDrawable(new osg::Cone(osg::Vec3(0.0f, 0.0f, 3.0f), 2.0f, 2.0f), hints.get());
    shape->setColor(osg::Vec4(0.8f, 0.8f, 0.4f, 1.0f));
    geode_7->addDrawable(shape.get());






    // material
    osg::ref_ptr<osg::Material> matirial = new osg::Material;
    matirial->setColorMode(osg::Material::DIFFUSE);
    matirial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
    matirial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
    matirial->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
    root->getOrCreateStateSet()->setAttributeAndModes(matirial.get(), osg::StateAttribute::ON);

      transform_1.get()->addChild(addDraggerToScene(geode_1.get(),cmdMgr,"TabBoxDragger"));
    transform_2.get()->addChild(addDraggerToScene(geode_2.get(),cmdMgr,"TabPlaneDragger"));
    transform_3.get()->addChild(addDraggerToScene(geode_3.get(),cmdMgr,"TabPlaneTrackballDragger"));
    transform_4.get()->addChild(addDraggerToScene(geode_4.get(),cmdMgr,"TrackballDragger"));
    transform_5.get()->addChild(addDraggerToScene(geode_5.get(),cmdMgr,"Translate1DDragger"));
    transform_6.get()->addChild(addDraggerToScene(geode_6.get(),cmdMgr,"Translate2DDragger"));
    transform_7.get()->addChild(addDraggerToScene(geode_7.get(),cmdMgr,"TranslateAxisDragger"));

    root->addChild(transform_1.get());
    root->addChild(transform_2.get());
    root->addChild(transform_3.get());
    root->addChild(transform_4.get());
    root->addChild(transform_5.get());
    root->addChild(transform_6.get());
    root->addChild(transform_7.get());

 
 
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

        PickModeHandler():
            _mode(VIEW), 
            _activeDragger(0)
        {
        }        
        
        bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa,
                    osg::Object*, osg::NodeVisitor*)
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            if (!view) return false;

            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Tab &&
                ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN &&
                _activeDragger == 0)
            {
                _mode = ! _mode;
            }
            
            if (VIEW == _mode) return false;

            switch (ea.getEventType())
            {
                case osgGA::GUIEventAdapter::PUSH:
                {
                    osgUtil::LineSegmentIntersector::Intersections intersections;

                    _pointer.reset();

                    if (view->computeIntersections(ea.getX(),ea.getY(),intersections))
                    {
                        _pointer.setCamera(view->getCamera());
                        _pointer.setMousePosition(ea.getX(), ea.getY());

                        for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
                            hitr != intersections.end();
                            ++hitr)
                        {
                            _pointer.addIntersection(hitr->nodePath, hitr->getLocalIntersectPoint());
                        }
                        for (osg::NodePath::iterator itr = _pointer._hitList.front().first.begin();
                             itr != _pointer._hitList.front().first.end();
                             ++itr)
                        {
                            osgManipulator::Dragger* dragger = dynamic_cast<osgManipulator::Dragger*>(*itr);
                            if (dragger)
                            {

                                dragger->handle(_pointer, ea, aa);
                                _activeDragger = dragger;
                                break;
                            }                   
                        }
                    }
                }
                case osgGA::GUIEventAdapter::DRAG:
                case osgGA::GUIEventAdapter::RELEASE:
                {
                    if (_activeDragger)
                    {
                        _pointer._hitIter = _pointer._hitList.begin();
                        _pointer.setCamera(view->getCamera());
                        _pointer.setMousePosition(ea.getX(), ea.getY());

                        _activeDragger->handle(_pointer, ea, aa);
                    }
                    break;
                }
        default:
            break;
            }

            if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
            {
                _activeDragger = 0;
                _pointer.reset();
            }

            return true;
        }
        
    private:
        unsigned int _mode;
        osgManipulator::Dragger* _activeDragger;
        osgManipulator::PointerInfo _pointer;
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line parameters");
    arguments.getApplicationUsage()->addCommandLineOption("--help-env","Display environmental variables available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-keys","Display keyboard & mouse bindings available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-all","Display all command line, env vars and keyboard & mouse bindings.");

    arguments.getApplicationUsage()->addCommandLineOption("--dragger <draggername>","Use the specified dragger for manipulation [TabPlaneDragger,TabPlaneTrackballDragger,TrackballDragger,Translate1DDragger,Translate2DDragger,TranslateAxisDragger,TabBoxDragger]");
    

    // construct the viewer.
    osgViewer::Viewer viewer;

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

    std::string dragger_name = "TabBoxDragger";
    arguments.read("--dragger", dragger_name);

    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // read the scene from the list of file specified command line args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // create a command manager
    osg::ref_ptr<osgManipulator::CommandManager> cmdMgr = new osgManipulator::CommandManager;

    // if no model has been successfully loaded report failure.
    bool tragger2Scene(true);
    if (!loadedModel) 
    {
        //std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        //return 1;
        loadedModel = createDemoScene(cmdMgr.get());
        tragger2Scene=false;
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

    
    // pass the loaded scene graph to the viewer.
    if ( tragger2Scene ) {
        viewer.setSceneData(addDraggerToScene(loadedModel.get(), cmdMgr.get(), dragger_name));
    } else { 
        viewer.setSceneData(loadedModel.get());
    }
    viewer.addEventHandler(new PickModeHandler());

    return viewer.run();
}

