#include <osg/GL>
#include <osgGLUT/glut>
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

void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] infile1 [infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName      - load plugin of name libraryName"<< std::endl;
    out <<"                          i.e. -l osgdb_pfb"<< std::endl;
    out <<"                          Useful for loading reader/writers which can load"<< std::endl;
    out <<"                          other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName    - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                          i.e. -e pfb"<< std::endl;
    out <<"                          Useful short hand for specifying full library name as"<< std::endl;
    out <<"                          done with -l above, as it automatically expands to"<< std::endl;
    out <<"                          the full library name appropriate for each platform."<< std::endl;
    out <<std::endl;
    out <<"    -stereo             - switch on stereo rendering, using the default of,"<< std::endl;
    out <<"                          ANAGLYPHIC or the value set in the OSG_STEREO_MODE "<< std::endl;
    out <<"                          environmental variable. See doc/stereo.html for "<< std::endl;
    out <<"                          further details on setting up accurate stereo "<< std::endl;
    out <<"                          for your system. "<< std::endl;
    out <<"    -stereo ANAGLYPHIC  - switch on anaglyphic(red/cyan) stereo rendering."<< std::endl;
    out <<"    -stereo QUAD_BUFFER - switch on quad buffered stereo rendering."<< std::endl;
    out <<std::endl;
    out <<"    -stencil            - use a visual with stencil buffer enabled, this "<< std::endl;
    out <<"                          also allows the depth complexity statistics mode"<< std::endl;
    out <<"                          to be used (press 'p' three times to cycle to it)."<< std::endl;
    out << std::endl;
}

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
        text->setText("Then place a osg::Projection node above the subgraph\nto create an othrograph projection.");
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

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(std::cout,argv[0]);
        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(commandLine);
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
