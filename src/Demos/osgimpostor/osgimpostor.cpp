#include <osg/Impostor>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>
#include <osgUtil/InsertImpostorsVisitor>

#include <GL/glut.h>
#include <osgGLUT/Viewer>

#include <osg/Quat>

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

    osg::Timer timer;
    osg::Timer_t before_load = timer.tick();
    
    // initialize the viewer.
    osgGLUT::Viewer viewer;
    
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the nodes from the commandline arguments.
    osg::Node* model = osgDB::readNodeFiles(commandLine);
    if (!model)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    

    // the osgUtil::InsertImpostorsVisitor used lower down to insert impostors
    // only operators on subclass of Group's, if the model top node is not
    // a group then it won't be able to insert an impostor.  We therefore
    // manually insert an impostor above the model.
    if (dynamic_cast<osg::Group*>(model)==0)
    {
        const osg::BoundingSphere& bs = model->getBound();
        if (bs.isValid())
        {

            osg::Impostor* impostor = new osg::Impostor;

            // standard LOD settings
            impostor->addChild(model);
            impostor->setRange(0,0.0f);
            impostor->setRange(1,1e7f);
            impostor->setCenter(bs.center());

            // impostor specfic settings.
            impostor->setImpostorThresholdToBound(5.0f);
            
            model = impostor;

        }
    }
    
    // we insert an impostor node above the model, so we keep a handle
    // on the rootnode of the model, the is required since the
    // InsertImpostorsVisitor can add a new root in automatically and
    // we would know about it, other than by following the parent path
    // up from model.  This is really what should be done, but I'll pass
    // on it right now as it requires a getRoots() method to be added to
    // osg::Node, and we're about to make a release so no new features! 
    osg::Group* rootnode = new osg::Group;
    rootnode->addChild(model);
    
    
    // now insert impostors in the model using the InsertImpostorsVisitor.
    osgUtil::InsertImpostorsVisitor ov;
    
    // traverse the model and collect all osg::Group's and osg::LOD's.
    // however, don't traverse the rootnode since we want to keep it as
    // the start of traversal, otherwise the insertImpostor could insert
    // and Impostor above the current root, making it nolonger a root!
    model->accept(ov);
    
    // insert the Impostors above groups and LOD's
    ov.insertImpostors();
    

    osg::Timer_t after_load = timer.tick();
	std::cout << "Time for load = "<<timer.delta_s(before_load,after_load)<<" seconds"<< std::endl;

    // add model to viewer.
    viewer.addViewport( rootnode );

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
    viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

    viewer.open();
    viewer.run();

    return 0;
}
