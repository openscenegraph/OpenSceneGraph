#if 1

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/AnimationPathManipulator>
#include <iostream>

class ThreadingHandler : public osgGA::GUIEventHandler 
{
public: 

    ThreadingHandler() {}
        
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;
    
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()=='m')
                {
                    switch(viewer->getThreadingModel())
                    {
                        case(osgViewer::Viewer::SingleThreaded):
                            viewer->setThreadingModel(osgViewer::Viewer::ThreadPerContext);
                            osg::notify(osg::NOTICE)<<"Threading model 'ThreadPerContext' selected."<<std::endl;
                            break;
                        case(osgViewer::Viewer::ThreadPerContext):
                            viewer->setThreadingModel(osgViewer::Viewer::ThreadPerCamera);
                            osg::notify(osg::NOTICE)<<"Threading model 'ThreadPerCamera' selected."<<std::endl;
                            break;
                        case(osgViewer::Viewer::ThreadPerCamera):
                            viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
                            osg::notify(osg::NOTICE)<<"Threading model 'SingleTheaded' selected."<<std::endl;
                            break;
                    }
                    return true;
                }
                if (ea.getKey()=='e')
                {
                    switch(viewer->getEndBarrierPosition())
                    {
                        case(osgViewer::Viewer::BeforeSwapBuffers):
                            viewer->setEndBarrierPosition(osgViewer::Viewer::AfterSwapBuffers);
                            osg::notify(osg::NOTICE)<<"Threading model 'AfterSwapBuffers' selected."<<std::endl;
                            break;
                        case(osgViewer::Viewer::AfterSwapBuffers):
                            viewer->setEndBarrierPosition(osgViewer::Viewer::BeforeSwapBuffers);
                            osg::notify(osg::NOTICE)<<"Threading model 'BeforeSwapBuffers' selected."<<std::endl;
                            break;
                    }
                    return true;
                }
            }
            default: break;
        }
        
        return false;
    }
    
    bool _done;
};


class ModelHandler : public osgGA::GUIEventHandler 
{
public: 

    ModelHandler():
        _position(0) {}
    
    typedef std::vector<std::string> Filenames;
    Filenames _filenames;
    unsigned int _position;
    
    void add(const std::string& filename) { _filenames.push_back(filename); }
        
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;
    
        if (_filenames.empty()) return false;
    
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()=='l')
                {                    
                    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( _filenames[_position] );
                    ++_position;
                    if (_position>=_filenames.size()) _position = 0;
                    
                    if (model.valid())
                    {
                        viewer->setSceneData(model.get());
                    }
                    
                    return true;
                }
            }
            default: break;
        }
        
        return false;
    }
    
    bool _done;
};


void singleWindowMultipleCameras(osgViewer::Viewer& viewer)
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }
    
    unsigned int width, height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (gc.valid())
    {
        osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<std::endl;

        // need to ensure that the window is cleared make sure that the complete window is set the correct colour
        // rather than just the parts of the window that are under the camera's viewports
        gc->setClearColor(osg::Vec4f(0.2f,0.2f,0.6f,1.0f));
        gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
}

void multipleWindowMultipleCameras(osgViewer::Viewer& viewer)
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }
    
    unsigned int width, height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);


    unsigned int numCameras = 6;
    double aspectRatioScale = (double)numCameras;
    double translate_x = double(numCameras)-1;
    for(unsigned int i=0; i<numCameras;++i, translate_x -= 2.0)
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->screenNum = i / 3;
        traits->x = (i*width)/numCameras;
        traits->y = 0;
        traits->width = width/numCameras-1;
        traits->height = height;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (gc.valid())
        {
            osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<std::endl;
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
        if (!apm.valid() || !(apm->valid()) ) 
        {
            apm = 0;
        }
    }

    osgViewer::Viewer viewer;
    
    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::Viewer::ThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::Viewer::ThreadPerCamera); }
    
    bool limitNumberOfFrames = false;
    unsigned int maxFrames = 10;
    while (arguments.read("--run-till-frame-number",maxFrames)) { limitNumberOfFrames = true; }

    // alternative viewer window setups.
    while (arguments.read("-1")) { singleWindowMultipleCameras(viewer); }
    while (arguments.read("-2")) { multipleWindowMultipleCameras(viewer); }

#if 0
    if (apm.valid()) viewer.setCameraManipulator(apm.get());
    else viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
#else

    viewer.setCameraManipulator( new osgGA::FlightManipulator() );

#endif


#if 0

    ModelHandler* modelHandler = new ModelHandler;
    for(int i=1; i<arguments.argc();++i)
    {
        modelHandler->add(arguments[i]);
    }
    
    viewer.addEventHandler(modelHandler);

#else

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }

    viewer.setSceneData(loadedModel.get());
#endif

    viewer.realize();

    unsigned int numFrames = 0;
    while(!viewer.done() && !(limitNumberOfFrames && numFrames>=maxFrames))
    {
        viewer.frame();
        ++numFrames;
    }

    return 0;
}
#else

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

int main( int, char **)
{
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("cow.osg");

    for(unsigned int i=0; i<5; ++i)
    {
        osg::notify(osg::NOTICE)<<"New frame *******************************"<<std::endl;

        osgViewer::Viewer viewer;
        viewer.setSceneData(model.get());
        viewer.run();
        osg::notify(osg::NOTICE)<<std::endl<<std::endl;
    }
    return 0;
}

#endif
