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

#if 1

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"View::setUpViewAcrossAllScreens() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return 0;
    }
    
    unsigned int width, height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
#if 0
    traits->windowDecoration = false;
#else
    traits->windowDecoration = true;
#endif            
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (gw)
    {
        osg::notify(osg::NOTICE)<<"  GraphicsWindow has been created successfully."<<gw<<std::endl;

        gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(0, 0, width, height );
    }
    else
    {
        osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
    }

    unsigned int numCameras = 2;
    double aspectRatioScale = 1.0;///(double)numCameras;
    for(unsigned int i=0; i<numCameras;++i)
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport((i*width)/numCameras,(i*height)/numCameras, width/numCameras, height/numCameras));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::scale(aspectRatioScale,1.0,1.0));
    }

    viewer.setUpRenderingSupport();
    viewer.assignSceneDataToCameras();
    
#else

    viewer.setUpViewAcrossAllScreens();

#endif    
    
    viewer.realize();

    gw->requestWarpPointer(100,100);

    bool limitNumberOfFrames = false;
    unsigned int numFrames = 0;
    unsigned int maxFrames = 10;

    while(!viewer.done() && !(limitNumberOfFrames && numFrames>=maxFrames))
    {
        viewer.frame();
        ++numFrames;
    }

    return 0;
}
