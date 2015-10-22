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
#include <osgViewer/ViewerEventHandlers>
#include <osg/CoordinateSystemNode>
#include <osgText/Text>

#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabBoxTrackballDragger>
#include <osgManipulator/TabPlaneDragger>
#include <osgManipulator/TabPlaneTrackballDragger>
#include <osgManipulator/Scale1DDragger>
#include <osgManipulator/Scale2DDragger>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/Translate1DDragger>
#include <osgManipulator/Translate2DDragger>
#include <osgManipulator/TranslateAxisDragger>
#include <osgManipulator/TranslatePlaneDragger>
#include <osgManipulator/RotateCylinderDragger>
#include <osgManipulator/AntiSquish>

#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/io_utils>

#include <iostream>

class PlaneConstraint : public osgManipulator::Constraint
{
public:
        PlaneConstraint() {}

        virtual bool constrain(osgManipulator::TranslateInLineCommand& command) const
        {
            OSG_NOTICE<<"PlaneConstraint TranslateInLineCommand "<<command.getTranslation()<<std::endl;
            return true;
        }
        virtual bool constrain(osgManipulator::TranslateInPlaneCommand& command) const
        {
            //command.setTranslation(osg::Vec3(0.0f,0.0f,0.0f));
            OSG_NOTICE<<"PlaneConstraint TranslateInPlaneCommand "<<command.getTranslation()<<std::endl;
            return true;
        }
        virtual bool constrain(osgManipulator::Scale1DCommand& command) const
        {
            //command.setScale(1.0f);
            OSG_NOTICE<<"PlaneConstraint Scale1DCommand"<<command.getScale()<<std::endl;
            return true;
        }
        virtual bool constrain(osgManipulator::Scale2DCommand& command) const
        {
            //command.setScale(osg::Vec2d(1.0,1.0));
            OSG_NOTICE<<"PlaneConstraint Scale2DCommand "<<command.getScale()<<std::endl;
            return true;
        }
        virtual bool constrain(osgManipulator::ScaleUniformCommand& command) const
        {
            OSG_NOTICE<<"PlaneConstraint ScaleUniformCommand"<<command.getScale()<<std::endl;
            return true;
        }
};



osgManipulator::Dragger* createDragger(const std::string& name)
{
    osgManipulator::Dragger* dragger = 0;
    if ("TabPlaneDragger" == name)
    {
        osgManipulator::TabPlaneDragger* d = new osgManipulator::TabPlaneDragger();
        d->setupDefaultGeometry();
        d->addConstraint(new PlaneConstraint());
        dragger = d;
    }
    else if ("TabPlaneTrackballDragger" == name)
    {
        osgManipulator::TabPlaneTrackballDragger* d = new osgManipulator::TabPlaneTrackballDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("TabBoxTrackballDragger" == name)
    {
        osgManipulator::TabBoxTrackballDragger* d = new osgManipulator::TabBoxTrackballDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("TrackballDragger" == name)
    {
        osgManipulator::TrackballDragger* d = new osgManipulator::TrackballDragger();
        d->setupDefaultGeometry();
        //d->setAxisLineWidth(5.0f);
        //d->setPickCylinderHeight(0.1f);
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
        d->setAxisLineWidth(5.0f);
        d->setPickCylinderRadius(0.05f);
        d->setConeHeight(0.2f);
        dragger = d;
    }
    else if ("TranslatePlaneDragger" == name)
    {
        osgManipulator::TranslatePlaneDragger* d = new osgManipulator::TranslatePlaneDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("Scale1DDragger" == name)
    {
        osgManipulator::Scale1DDragger* d = new osgManipulator::Scale1DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("Scale2DDragger" == name)
    {
        osgManipulator::Scale2DDragger* d = new osgManipulator::Scale2DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("RotateCylinderDragger" == name)
    {
        osgManipulator::RotateCylinderDragger* d = new osgManipulator::RotateCylinderDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if ("RotateSphereDragger" == name)
    {
        osgManipulator::RotateSphereDragger* d = new osgManipulator::RotateSphereDragger();
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

// The DraggerContainer node is used to fix the dragger's size on the screen
class DraggerContainer : public osg::Group
{
public:
    DraggerContainer() : _draggerSize(240.0f), _active(true) {}

    DraggerContainer( const DraggerContainer& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
    :   osg::Group(copy, copyop),
        _dragger(copy._dragger), _draggerSize(copy._draggerSize), _active(copy._active)
    {}

    META_Node( osgManipulator, DraggerContainer );

    void setDragger( osgManipulator::Dragger* dragger )
    {
        _dragger = dragger;
        if ( !containsNode(dragger) ) addChild( dragger );
    }

    osgManipulator::Dragger* getDragger() { return _dragger.get(); }
    const osgManipulator::Dragger* getDragger() const { return _dragger.get(); }

    void setDraggerSize( float size ) { _draggerSize = size; }
    float getDraggerSize() const { return _draggerSize; }

    void setActive( bool b ) { _active = b; }
    bool getActive() const { return _active; }

    void traverse( osg::NodeVisitor& nv )
    {
        if ( _dragger.valid() )
        {
            if ( _active && nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
            {
                osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(&nv);

                float pixelSize = cv->pixelSize(_dragger->getBound().center(), 0.48f);
                if ( pixelSize!=_draggerSize )
                {
                    float pixelScale = pixelSize>0.0f ? _draggerSize/pixelSize : 1.0f;
                    osg::Vec3d scaleFactor(pixelScale, pixelScale, pixelScale);

                    osg::Vec3 trans = _dragger->getMatrix().getTrans();
                    _dragger->setMatrix( osg::Matrix::scale(scaleFactor) * osg::Matrix::translate(trans) );
                }
            }
        }
        osg::Group::traverse(nv);
    }

protected:
    osg::ref_ptr<osgManipulator::Dragger> _dragger;
    float _draggerSize;
    bool _active;
};

osg::Node* addDraggerToScene(osg::Node* scene, const std::string& name, bool fixedSizeInScreen)
{
    scene->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

    osg::MatrixTransform* transform = new osg::MatrixTransform;
    transform->addChild(scene);

    osgManipulator::Dragger* dragger = createDragger(name);

    osg::Group* root = new osg::Group;
    root->addChild(transform);

    if ( fixedSizeInScreen )
    {
        DraggerContainer* draggerContainer = new DraggerContainer;
        draggerContainer->setDragger( dragger );
        root->addChild(draggerContainer);
    }
    else
        root->addChild(dragger);

    float scale = scene->getBound().radius() * 1.6;
    dragger->setMatrix(osg::Matrix::scale(scale, scale, scale) *
                       osg::Matrix::translate(scene->getBound().center()));

    if (dynamic_cast<osgManipulator::TabPlaneDragger*>(dragger))
    {
        dragger->addTransformUpdating(transform, osgManipulator::DraggerTransformCallback::HANDLE_TRANSLATE_IN_LINE);
    }
    else
    {
        dragger->addTransformUpdating(transform);
    }

    // we want the dragger to handle it's own events automatically
    dragger->setHandleEvents(true);

    // if we don't set an activation key or mod mask then any mouse click on
    // the dragger will activate it, however if do define either of ActivationModKeyMask or
    // and ActivationKeyEvent then you'll have to press either than mod key or the specified key to
    // be able to activate the dragger when you mouse click on it.  Please note the follow allows
    // activation if either the ctrl key or the 'a' key is pressed and held down.
    dragger->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_CTRL);
    dragger->setActivationKeyEvent('a');

    return root;
}

osg::Node* createDemoScene(bool fixedSizeInScreen) {

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

    transform_1.get()->addChild(addDraggerToScene(geode_1.get(),"TabBoxDragger",fixedSizeInScreen));
    transform_2.get()->addChild(addDraggerToScene(geode_2.get(),"TabPlaneDragger",fixedSizeInScreen));
    transform_3.get()->addChild(addDraggerToScene(geode_3.get(),"TabBoxTrackballDragger",fixedSizeInScreen));
    transform_4.get()->addChild(addDraggerToScene(geode_4.get(),"TrackballDragger",fixedSizeInScreen));
    transform_5.get()->addChild(addDraggerToScene(geode_5.get(),"Translate1DDragger",fixedSizeInScreen));
    transform_6.get()->addChild(addDraggerToScene(geode_6.get(),"Translate2DDragger",fixedSizeInScreen));
    transform_7.get()->addChild(addDraggerToScene(geode_7.get(),"TranslateAxisDragger",fixedSizeInScreen));

    root->addChild(transform_1.get());
    root->addChild(transform_2.get());
    root->addChild(transform_3.get());
    root->addChild(transform_4.get());
    root->addChild(transform_5.get());
    root->addChild(transform_6.get());
    root->addChild(transform_7.get());



    return root;
}

//
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

    arguments.getApplicationUsage()->addCommandLineOption("--dragger <draggername>","Use the specified dragger for manipulation [TabPlaneDragger, TabPlaneTrackballDragger, TrackballDragger, Translate1DDragger, Translate2DDragger, TranslateAxisDragger, TabBoxDragger, TranslatePlaneDragger, Scale1DDragger, Scale2DDragger, RotateCylinderDragger, RotateSphereDragger]");
    arguments.getApplicationUsage()->addCommandLineOption("--fixedDraggerSize","Fix the size of the dragger geometry in the screen space");

    bool fixedSizeInScreen = false;
    while (arguments.read("--fixedDraggerSize")) { fixedSizeInScreen = true; }


    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());


    if (arguments.read("--test-NodeMask"))
    {
        const osg::ref_ptr<osg::Group> group = new osg::Group();
        group->setNodeMask(0);

        const osg::ref_ptr<osgManipulator::AntiSquish> antiSquish = new osgManipulator::AntiSquish();

        group->addChild(antiSquish.get());

        const osg::ref_ptr<osg::Node> node = new osg::Node();
        node->setInitialBound(osg::BoundingSphere(osg::Vec3(0.0, 0.0, 0.0), 1.0));

        antiSquish->addChild(node.get());

        group->getBound();

        return 1;
    }



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
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    bool tragger2Scene(true);
    if (!loadedModel)
    {
        //std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        //return 1;
        loadedModel = createDemoScene(fixedSizeInScreen);
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
    optimizer.optimize(loadedModel);


    // pass the loaded scene graph to the viewer.
    if ( tragger2Scene ) {
        viewer.setSceneData(addDraggerToScene(loadedModel.get(), dragger_name, fixedSizeInScreen));
    } else {
        viewer.setSceneData(loadedModel);
    }


    return viewer.run();
}

