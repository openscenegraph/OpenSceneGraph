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


///////////////////////////////////////////////////////////////////////////////
// globals
#define        TEXT_POLYGON    "Polygon Font - jygq"
#define        TEXT_OUTLINE    "Outline Font - jygq"
#define        TEXT_TEXTURE    "Texture Font - jygq"
#define        TEXT_BITMAP        "Bitmap Font - jygq"
#define        TEXT_PIXMAP        "Pixmap Font - jygq"

#define        TEXT_COL_2D        osg::Vec4(.9,.9,.9,1)
#define        TEXT_COL_3D        osg::Vec4(.99,.3,.2,1)


std::string    timesFont("fonts/times.ttf");
std::string    arialFont("fonts/arial.ttf");

int    gFontSize=18;
int    gFontSize1=24;
osgText::Text::AlignmentType gAlignment=osgText::Text::LEFT_BOTTOM;

void set2dScene(osg::Group* rootNode)
{
    osg::Geode* geode = new osg::Geode();
    rootNode->addChild(geode);
    
    osg::Vec3 position(150.0f,10.0f,0.0f);
    osg::Vec3 delta(90.0f,120.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    
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
    
    // create the hud.
    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1024,0,768));
    
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
    modelview_abs->setMatrix(osg::Matrix::identity());
    
    
    set2dScene(modelview_abs);

    projection->addChild(modelview_abs);
//     projection->setAppCallback(new MyCallback("App callback"));
//     projection->setCullCallback(new MyCallback("Cull callback"));

    group->addChild(projection);
    
     
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
