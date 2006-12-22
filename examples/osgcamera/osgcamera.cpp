// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface to create an OpenGL
// graphics window, and OSG for rendering.


#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <iostream>

int main( int argc, char **argv )
{
    if (argc<2) 
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }


    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts(2);
    osg::Referenced::setThreadSafeReferenceCounting(true);

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }

    osgViewer::Viewer viewer;
    
    viewer.setSceneData(loadedModel.get());

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.getCamera()->setClearColor(osg::Vec4f(0.6f,0.6f,0.8f,1.0f));

    viewer.setUpViewAcrossAllScreens();
    viewer.realize();

    unsigned int numFrames = 0;
    unsigned int maxFrames = 100;

    while(!viewer.done() && numFrames<maxFrames)
    {
        viewer.frame();
        ++numFrames;
    }

    return 0;
}
