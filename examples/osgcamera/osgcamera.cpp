// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface to create an OpenGL
// graphics window, and OSG for rendering.


#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/AnimationPathManipulator>
#include <iostream>

void singleWindowMultipleCameras(osgViewer::Viewer& viewer)
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"View::setUpViewAcrossAllScreens() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
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
        osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<gw<<std::endl;

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
}

void multipleWindowMultipleCameras(osgViewer::Viewer& viewer)
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"View::setUpViewAcrossAllScreens() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }
    
    unsigned int width, height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);


    unsigned int numCameras = 4;
    double aspectRatioScale = (double)numCameras;
    double translate_x = double(numCameras)-1;
    for(unsigned int i=0; i<numCameras;++i, translate_x -= 2.0)
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->screenNum = i / 2;
        traits->x = (i*width)/numCameras;
        traits->y = 0;
        traits->width = width/numCameras-1;
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
            osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<gw<<std::endl;

            gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(0, 0, traits->width, traits->height );
        }
        else
        {
            osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
        }

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0, width/numCameras, height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrix::scale(aspectRatioScale, 1.0, 1.0)*osg::Matrix::translate(translate_x, 0.0, 0.0), osg::Matrix() );
    }

    viewer.setUpRenderingSupport();
    viewer.assignSceneDataToCameras();
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    if (argc<2) 
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }


    std::string pathfile;
    osg::ref_ptr<osgGA::AnimationPathManipulator> apm = 0;
    while (arguments.read("-p",pathfile))
    {
        apm = new osgGA::AnimationPathManipulator(pathfile);
        if(!apm.valid() || !(apm->valid()) ) 
        {
            apm = 0;
        }
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }

    osgViewer::Viewer viewer;
    
    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::Viewer::ThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::Viewer::ThreadPerCamera); }

    viewer.setSceneData(loadedModel.get());

    if (apm.valid()) viewer.setCameraManipulator(apm.get());
    else viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

#if 0

    // singleWindowMultipleCameras(viewer);
    
    multipleWindowMultipleCameras(viewer);
    
    
#else

    viewer.setUpViewAcrossAllScreens();

#endif
    
    viewer.realize();

    bool limitNumberOfFrames = true;
    unsigned int numFrames = 0;
    unsigned int maxFrames = 10;

    while(!viewer.done() && !(limitNumberOfFrames && numFrames>=maxFrames))
    {
        viewer.frame();
        ++numFrames;
    }

    return 0;
}
