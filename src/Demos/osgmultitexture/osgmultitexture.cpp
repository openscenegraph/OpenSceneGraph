#include <osgGLUT/Viewer>

#include <osg/Notify>

#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>


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
    
    osg::Image* image = osgDB::readImageFile("Images/reflect.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        osg::TexGen* texgen = new osg::TexGen;
        texgen->setMode(osg::TexGen::SPHERE_MAP);

        osg::TexEnv* texenv = new osg::TexEnv;
        texenv->setMode(osg::TexEnv::BLEND);
        texenv->setColor(osg::Vec4(0.3f,0.3f,0.3f,0.3f));

        osg::StateSet* stateset = new osg::StateSet;
        stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);
        stateset->setTextureAttributeAndModes(1,texgen,osg::StateAttribute::ON);
        stateset->setTextureAttribute(1,texenv);
        
        rootnode->setStateSet(stateset);
    }
    else
    {
        osg::notify(osg::NOTICE)<<"unable to load reflect map, model will not be mutlitextured"<<std::endl;
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
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
