#include <osg/Node>
#include <osg/GeoSet>
#include <osg/Notify>
#include <osg/Transform>
#include <osg/Texture>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>




void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] image_left_eye image_right_eye"<< std::endl;
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
    out<<"example:"<<std::endl;
    out<<"     Images/dog_left_eye.jpg Images/dog_right_eye.jpg"<<std::endl;
    out<<std::endl;
}

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 0;
    }

    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);


    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);
    
    
    // default to enabling stereo.
    viewer.getDisplaySettings()->setStereo(true);

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);


    if (commandLine.size()<2)
    {
        osg::notify(osg::NOTICE) << "Please specify two images required for stereo imaging."<<std::endl;
        return 0;
    }
    
    osg::Image* imageLeft = osgDB::readImageFile(commandLine[0]);
    osg::Image* imageRight = osgDB::readImageFile(commandLine[1]);
    

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
