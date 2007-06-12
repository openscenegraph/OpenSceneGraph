/* OpenSceneGraph example, osgcamera.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#if 1

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/AnimationPathManipulator>
#include <iostream>

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

void multipleWindowMultipleCameras(osgViewer::Viewer& viewer, bool multipleScreens)
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
        traits->screenNum = multipleScreens ? i / 3 : 0;
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

    osgViewer::Viewer viewer(arguments);
    
    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext); }
    while (arguments.read("-d")) { viewer.setThreadingModel(osgViewer::Viewer::DrawThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext); }
    
    bool limitNumberOfFrames = false;
    unsigned int maxFrames = 10;
    while (arguments.read("--run-till-frame-number",maxFrames)) { limitNumberOfFrames = true; }

    // alternative viewer window setups.
    while (arguments.read("-1")) { singleWindowMultipleCameras(viewer); }
    while (arguments.read("-2")) { multipleWindowMultipleCameras(viewer, false); }
    while (arguments.read("-3")) { multipleWindowMultipleCameras(viewer, true); }

    if (apm.valid()) viewer.setCameraManipulator(apm.get());
    else viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::ThreadingHandler);

    std::string configfile;
    while (arguments.read("--config", configfile))
    {
        osg::notify(osg::NOTICE)<<"Trying to read config file "<<configfile<<std::endl;
        osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(configfile);
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(object.get());
        if (view)
        {
            osg::notify(osg::NOTICE)<<"Read config file succesfully"<<std::endl;
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Failed to read config file : "<<configfile<<std::endl;
            return 1;
        }
    }

    while (arguments.read("--write-config", configfile)) { osgDB::writeObjectFile(viewer, configfile); }


    if (arguments.read("-m"))
    {
        ModelHandler* modelHandler = new ModelHandler;
        for(int i=1; i<arguments.argc();++i)
        {
            modelHandler->add(arguments[i]);
        }

        viewer.addEventHandler(modelHandler);
    }
    else
    {
        // load the scene.
        osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

        if (!loadedModel) loadedModel = osgDB::readNodeFile("cow.osg");

        if (!loadedModel) 
        {
            std::cout << argv[0] <<": No data loaded." << std::endl;
            return 1;
        }

        viewer.setSceneData(loadedModel.get());
    }
    
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
