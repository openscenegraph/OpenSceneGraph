#include <osg/Node>
#include <osg/Notify>
#include <osg/MatrixTransform>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGLUT/Viewer>

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
    
    // default to enabling stereo.
    viewer.getDisplaySettings()->setStereo(true);

    if (arguments.argc()<=2)
    {
        osg::notify(osg::NOTICE) << "Please specify two images required for stereo imaging."<<std::endl;
        return 0;
    }
    
    osg::Image* imageLeft = osgDB::readImageFile(arguments[1]);
    osg::Image* imageRight = osgDB::readImageFile(arguments[2]);
    

    if (imageLeft && imageRight)
    {    
    
        float average_s = (imageLeft->s()+imageRight->s())*0.5f;
        float average_t = (imageLeft->t()+imageRight->t())*0.5f;
    
        osg::Geode* geodeLeft = osg::createGeodeForImage(imageLeft,average_s,average_t);
        geodeLeft->setNodeMask(0x01);

        osg::Geode* geodeRight = osg::createGeodeForImage(imageRight,average_s,average_t);
        geodeRight->setNodeMask(0x02);


        osg::Group* rootNode = new osg::Group;
        rootNode->addChild(geodeLeft);
        rootNode->addChild(geodeRight);


        // add model to viewer.
        osgUtil::SceneView* sceneview = new osgUtil::SceneView;
        sceneview->setDisplaySettings(viewer.getDisplaySettings());
        sceneview->setDefaults();
        sceneview->setCullMask(0xffffffff);
        sceneview->setCullMaskLeft(0x00000001);
        sceneview->setCullMaskRight(0x00000002);
        sceneview->setSceneData(rootNode);
        viewer.addViewport( sceneview );

        // register trackball.
        viewer.registerCameraManipulator(new osgGA::TrackballManipulator);

        viewer.open();

        viewer.run();
    }
    else
    {
        osg::notify(osg::NOTICE) << "Unable to load two images required for stereo imaging."<<std::endl;
        return 0;
    }
    
    return 0;
}
