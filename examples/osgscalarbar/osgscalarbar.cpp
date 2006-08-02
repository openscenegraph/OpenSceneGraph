#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/BlendFunc>
#include <osg/ClearNode>
#include <osg/Projection>

#include <osgUtil/Tesselator>
#include <osgUtil/CullVisitor>


#include <osgGA/TrackballManipulator>
#include <osgProducer/Viewer>
#include <osgDB/ReadFile>

#include <osgSim/ScalarsToColors>
#include <osgSim/ColorRange>
#include <osgSim/ScalarBar>

#include <sstream>
#include <math.h>

using namespace osgSim;
using osgSim::ScalarBar;

#if defined(_MSC_VER)
// not have to have this pathway for just VS6.0 as its unable to handle the full
// ScalarBar::ScalarPrinter::printScalar scoping.

// Create a custom scalar printer
struct MyScalarPrinter: public ScalarBar::ScalarPrinter
{
    std::string printScalar(float scalar)
    {
        std::cout<<"In MyScalarPrinter::printScalar"<<std::endl;
        if(scalar==0.0f) return ScalarPrinter::printScalar(scalar)+" Bottom";
        else if(scalar==0.5f) return ScalarPrinter::printScalar(scalar)+" Middle";
        else if(scalar==1.0f) return ScalarPrinter::printScalar(scalar)+" Top";
        else return ScalarPrinter::printScalar(scalar);
    }
};
#else
// Create a custom scalar printer
struct MyScalarPrinter: public ScalarBar::ScalarPrinter
{
    std::string printScalar(float scalar)
    {
        std::cout<<"In MyScalarPrinter::printScalar"<<std::endl;
        if(scalar==0.0f) return ScalarBar::ScalarPrinter::printScalar(scalar)+" Bottom";
        else if(scalar==0.5f) return ScalarBar::ScalarPrinter::printScalar(scalar)+" Middle";
        else if(scalar==1.0f) return ScalarBar::ScalarPrinter::printScalar(scalar)+" Top";
        else return ScalarBar::ScalarPrinter::printScalar(scalar);
    }
};
#endif

osg::Node* createScalarBar()
{
#if 1
    //ScalarsToColors* stc = new ScalarsToColors(0.0f,1.0f);
    //ScalarBar* sb = new ScalarBar(2,3,stc,"STC_ScalarBar");

    // Create a custom color set
    std::vector<osg::Vec4> cs;
    cs.push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));   // R
    cs.push_back(osg::Vec4(0.0f,1.0f,0.0f,1.0f));   // G
    cs.push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));   // G
    cs.push_back(osg::Vec4(0.0f,0.0f,1.0f,1.0f));   // B
    cs.push_back(osg::Vec4(0.0f,1.0f,1.0f,1.0f));   // R


    ColorRange* cr = new ColorRange(0.0f,1.0f,cs);
    ScalarBar* sb = new ScalarBar(20, 11, cr, "ScalarBar", ScalarBar::VERTICAL, 0.1f, new MyScalarPrinter);
    sb->setScalarPrinter(new MyScalarPrinter);

    return sb;
#else
    ScalarBar *sb = new ScalarBar;
    ScalarBar::TextProperties tp;
    tp._fontFile = "fonts/times.ttf";

    sb->setTextProperties(tp);

    return sb;
#endif

}

osg::Node * createScalarBar_HUD()
{
    osgSim::ScalarBar * geode = new osgSim::ScalarBar;
    osgSim::ScalarBar::TextProperties tp;
    tp._fontFile = "fonts/times.ttf";
    geode->setTextProperties(tp);
    osg::StateSet * stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(11, "RenderBin");

    osg::MatrixTransform * modelview = new osg::MatrixTransform;
    modelview->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    osg::Matrixd matrix(osg::Matrixd::scale(1000,1000,1000) * osg::Matrixd::translate(120,10,0)); // I've played with these values a lot and it seems to work, but I have no idea why
    modelview->setMatrix(matrix);
    modelview->addChild(geode);

    osg::Projection * projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024)); // or whatever the OSG window res is
    projection->addChild(modelview);

    return projection; //make sure you delete the return sb line
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates both text, animation and billboard via custom transform to create the OpenSceneGraph logo..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("ps","Render the Professional Services logo");

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

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

    osg::Group* group = new osg::Group;
    group->addChild(createScalarBar());
    group->addChild(createScalarBar_HUD());

    // add model to viewer.
    viewer.setSceneData( group );

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
