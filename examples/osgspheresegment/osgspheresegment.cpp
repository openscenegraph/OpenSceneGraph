#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/BlendFunc>
#include <osg/ClearNode>

#include <osgUtil/Tesselator>
#include <osgUtil/TransformCallback>
#include <osgUtil/CullVisitor>

#include <osgText/Text>

#include <osgGA/TrackballManipulator>
#include <osgProducer/Viewer>
#include <osgDB/ReadFile>

#include <osgSim/SphereSegment>

using namespace osgSim;

class MyNodeCallback: public osg::NodeCallback
{
    void operator()(osg::Node*,osg::NodeVisitor*);
};

// void MyNodeCallback::operator()(osg::Node* n,osg::NodeVisitor* nv)
// {
//     if(osgSim::SphereSegment* ss=dynamic_cast<osgSim::SphereSegment*>(n))
// 	{
// 		osg::Vec3 vec;
// 	    float azRange, elevRange;
// 	    ss->getArea(vec,azRange,elevRange);
//
// 		float azRangeDeg = osg::RadiansToDegrees(azRange);
//
// 		static bool azAscending = false;
//
//         if(azAscending){
// 		    azRangeDeg += 1.0f;
// 			if(azRangeDeg>89.0f) azAscending = false;
// 		}else{
// 		    azRangeDeg -= 1.0f;
// 			if(azRangeDeg<2.0f) azAscending = true;
// 		}
//
// 		ss->setArea(vec,osg::DegreesToRadians(azRangeDeg),elevRange);
//
// 	}
//     traverse(n,nv);
// }

void MyNodeCallback::operator()(osg::Node* n,osg::NodeVisitor* nv)
{
    if(osgSim::SphereSegment* ss=dynamic_cast<osgSim::SphereSegment*>(n))
	{
		osg::Vec3 vec;
	    float azRange, elevRange;
	    ss->getArea(vec,azRange,elevRange);

        static float angle = 0.0f;
		if(++angle > 359.0f) angle = 0.0f;
		vec.set(sin(osg::DegreesToRadians(angle)),cos(osg::DegreesToRadians(angle)),0.0f);

        std::cout<<"angle "<<angle<<" degrees, vec is "<<vec
		                <<", azRange is "<<osg::RadiansToDegrees(azRange)
						<<", elevRange is "<<osg::RadiansToDegrees(elevRange)
						<<std::endl;

		ss->setArea(vec,azRange,elevRange);
	}
    traverse(n,nv);
}

osg::Node* createSphereSegment()
{
	SphereSegment* ss = new SphereSegment(osg::Vec3(0.0f,0.0f,0.0f), 1.0f,
					osg::Vec3(0.0f,1.0f,0.0f),
					osg::DegreesToRadians(90.0f),
					osg::DegreesToRadians(45.0f),
					60);
	ss->setAllColors(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
	ss->setSideColor(osg::Vec4(0.0f,0.0f,1.0f,0.1f));
	//ss->setDrawMask(SphereSegment::DrawMask(SphereSegment::SPOKES | SphereSegment::EDGELINE));

    //ss->setUpdateCallback(new MyNodeCallback);

    return ss;
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

    osg::Node* node = createSphereSegment();

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
