#include <stdio.h>
#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Node>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/AnimationPathManipulator>


#include <osgUtil/Optimizer>

struct SortByAverageDistanceFunctor
{
    bool operator() (const osgUtil::RenderGraph* lhs,const osgUtil::RenderGraph* rhs) const
    {
        return lhs->getAverageDistance()<rhs->getAverageDistance();
    }
};

struct SortByMinimumDistanceFunctor
{
    bool operator() (const osgUtil::RenderGraph* lhs,const osgUtil::RenderGraph* rhs) const
    {
        return lhs->getMinimumDistance()<rhs->getMinimumDistance();
    }
};

struct MySortCallback : public osgUtil::RenderBin::SortCallback
{
    virtual void sortImplementation(osgUtil::RenderBin* renderbin) 
    {
 
        osgUtil::RenderBin::RenderGraphList& renderGraphList = renderbin->_renderGraphList;
        for(osgUtil::RenderBin::RenderGraphList::iterator itr=renderGraphList.begin();
            itr!=renderGraphList.end();
            ++itr)
        {
            (*itr)->sortFrontToBack();
        }
        
//        std::sort(renderGraphList.begin(),renderGraphList.end(),SortByAverageDistanceFunctor());
       std::sort(renderGraphList.begin(),renderGraphList.end(),SortByMinimumDistanceFunctor());
    }
};

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
    out <<"    -O \"options string\" - create an options string to pass to the reader/wrter "<< std::endl;
    out <<"                          for controlling reading and writing parameters."<< std::endl;
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
    out <<"    -f                  - start with a full screen, borderless window." << std::endl;
    out <<"    -p  pathfile        - Use the AnimationPathManipulator (key binding '4') and use\n"
	  "                          pathfile to define the animation path.  pathfile is an ascii\n"
	  "                          file containing lines of eight floating point numbers representing\n"
	  "                          time, position (x,y,z) and attitude (x,y,z,w as a quaternion)." << std::endl;
    out << std::endl;
}

int main( int argc, char **argv )
{
    std::string pathfile;

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(std::cout,argv[0]);
        return 1;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) {
	if( std::string(argv[i]) == "-p" ) {
	    if( (i+1) >= argc ) {
		write_usage( std::cout, argv[0]);
	    	return 1;
	    }
	    else
		pathfile = std::string(argv[++i]);
	}
	else 
	    commandLine.push_back(argv[i]);
    }

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
        return 1;
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

    if( !pathfile.empty() ) {
	osgGA::AnimationPathManipulator *apm = new osgGA::AnimationPathManipulator(pathfile);
	if( apm->valid() ) 
        {
            //viewer.registerCameraManipulator(apm);
            // selecting the animation path manipulator up front seems to disable view frustum 
            // in a heavy duty way that I'm do understand yet, so I'm commenting out for the
            // time being.  Simply pressing '4' will get you the animation path working correctly.
            unsigned int no = viewer.registerCameraManipulator(apm);
            viewer.selectCameraManipulator(no);
        }
	else
	    delete apm;
    }
    
//     osgUtil::RenderBin* depth_renderbin = osgUtil::RenderBin::getRenderBinPrototype("DepthSortedBin");
//     if (depth_renderbin)
//     {
//         depth_renderbin->setSortMode(osgUtil::RenderBin::SORT_BY_STATE);
//     }

    osgUtil::RenderStage* renderstage = viewer.getViewportSceneView(0)->getRenderStage();
    if (renderstage)
    {
       renderstage->setSortCallback(new MySortCallback);
//        renderstage->setSortMode(osgUtil::RenderBin::SORT_FRONT_TO_BACK);
    }
    


    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
