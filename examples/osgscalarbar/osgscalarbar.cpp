#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/BlendFunc>
#include <osg/ClearNode>

#include <osgUtil/Tesselator>
#include <osgUtil/TransformCallback>
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

osg::Node* createScalarBar()
{
//     ScalarsToColors* stc = new ScalarsToColors(0.0f,1.0f);
// 	ScalarBar* sb = new ScalarBar(2,3,stc);
//
//     // Create a custom color set
//     std::vector<osg::Vec4> cs;
//     cs.push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));   // R
//     cs.push_back(osg::Vec4(0.0f,0.0f,1.0f,1.0f));   // B
//
// 	// Create a custom scalar printer
//     struct MyScalarPrinter: public ScalarBar::ScalarPrinter
// 	{
//         std::string printScalar(float scalar)
// 		{
// 		    std::cout<<"In MyScalarPrinter::printScalar"<<std::endl;
//             if(scalar==0.0f) return "Hello";
// 			else if(scalar==1.0f) return "Goodbye";
// 			else return ScalarBar::ScalarPrinter::printScalar(scalar);
// 		}
// 	};
//
//     ColorRange* stc = new ColorRange(0.0f,1.0f,cs);
// 	//ScalarBar* sb = new ScalarBar(2, 2, stc, "ScalarBar", ScalarBar::HORIZONTAL, 0.25f, new MyScalarPrinter);
// 	ScalarBar* sb = new ScalarBar(2, 2, stc, "ScalarBar", ScalarBar::VERTICAL, 4.0f, new MyScalarPrinter);
// 	sb->setScalarPrinter(new MyScalarPrinter);
//
// 	return sb;

    ScalarBar *sb = new ScalarBar;
    ScalarBar::TextProperties tp;
	tp._fontFile = "fonts/times.ttf";

	sb->setTextProperties(tp);

    return sb;

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

    osg::Node* node = createScalarBar();

    // add model to viewer.
    viewer.setSceneData( node );

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

    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}
