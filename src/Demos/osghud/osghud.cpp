#include <osgGLUT/Viewer>

#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/Depth>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgText/Text>


#include <osgUtil/Optimizer>

osg::Node* createHUD()
{
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/times.ttf");

    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

    osg::Vec3 position(150.0f,800.0f,0.0f);
    osg::Vec3 delta(0.0f,-120.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setText("Head Up Displays are simple :-)");
        text->setPosition(position);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setText("All you need to do is create your text in a subgraph.");
        text->setPosition(position);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setText("Disable depth test in this subgraph to ensure its always ontop.");
        text->setPosition(position);
        
        position += delta;
    }    

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setText("Then place an osg::Projection node above the subgraph\nto create an orthographic projection.");
        text->setPosition(position);
        
        position += delta;
    }    

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setText("And add an osg::ModelViewMatrix set to ABSOLUTE to ensure\nit remains independent from any external model view matrices.");
        text->setPosition(position);
        
        position += delta;
    }    

    // create the hud.
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(geode);

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    return projection;

}

struct MyCallback : public osg::NodeCallback
{

    MyCallback(const std::string& str):_message(str) {}

    virtual void operator() (osg::Node* node,osg::NodeVisitor* nv)
    {
        std::cout<<"In my callback '"<<_message<<"'"<<std::endl;
        traverse(node,nv);
    }
    
    std::string _message;
};




int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

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
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
    
    // make sure the root node is group so we can add extra nodes to it.
    osg::Group* group = dynamic_cast<osg::Group*>(rootnode);
    if (!group)
    {
        group = new osg::Group;
        group->addChild(rootnode);
        rootnode = group;
    }

    // add the HUD subgraph.    
    group->addChild(createHUD());
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
