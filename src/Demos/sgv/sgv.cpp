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

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Use specificed animation path file for camera animation");
   
    // read the commandline args.
    std::string pathfile;
    while (arguments.read("-p",pathfile)) {}

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
	osg::ref_ptr<osgGA::AnimationPathManipulator> apm = new osgGA::AnimationPathManipulator(pathfile);
	if( apm.valid() && apm->valid() ) 
        {
            unsigned int num = viewer.registerCameraManipulator(apm.get());
            viewer.selectCameraManipulator(num);
        }
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
