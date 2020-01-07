#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/AnimationPath>

#include "UpdateProperty.h"
#include "CameraProperty.h"
#include "CameraPathProperty.h"
#include "EventProperty.h"

#include "CaptureSettings.h"

#include <osgGA/StateSetManipulator>


struct ScreenShot : public osg::Camera::DrawCallback
{
    ScreenShot(GLenum pixelFormat, bool flip):
        _pixelFormat(pixelFormat),
        _flip(flip) {}

    virtual void operator () (osg::RenderInfo& renderInfo) const
    {
        if (!_frameCapture)
        {
            OSG_NOTICE<<"No FrameCamera assigned"<<std::endl;
            return;
        }

        unsigned int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
        
        CameraNumMap::const_iterator itr = _cameraNumMap.find(renderInfo.getCurrentCamera());
        std::string outputFileName = (itr!=_cameraNumMap.end()) ?
                                     _frameCapture->getOutputFileName(itr->second, frameNumber) :
                                     _frameCapture->getOutputFileName(frameNumber);
                                     
        OSG_NOTICE<<"outputFileName="<<outputFileName<<std::endl;

        osg::Camera* camera = renderInfo.getCurrentCamera();
        osg::Viewport* viewport = camera ? camera->getViewport() : 0;
        if (viewport)
        {
            OSG_NOTICE<<"Doing read of ="<<viewport->x()<<", "<<viewport->y()<<", "<<viewport->width()<<", "<<viewport->height()<<" with pixelFormat=0x"<<std::hex<<_pixelFormat<<std::dec<<std::endl;

            glReadBuffer(camera->getDrawBuffer());
            osg::ref_ptr<osg::Image> image = new osg::Image;
            
            image->readPixels(viewport->x(),viewport->y(),viewport->width(),viewport->height(),
                              _pixelFormat, GL_UNSIGNED_BYTE, 1);

            if (_flip) image->flipVertical();

            osgDB::writeImageFile(*image, outputFileName);
        }
        
    }

    typedef std::map<const osg::Camera*, unsigned int> CameraNumMap;

    GLenum                                      _pixelFormat;
    bool                                        _flip;
    osg::ref_ptr<gsc::CaptureSettings>          _frameCapture;
    CameraNumMap                                _cameraNumMap;
};

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of 3D textures.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-i <filename>","Input scene (or presentation) filename.");
    arguments.getApplicationUsage()->addCommandLineOption("-o <filename>","Base output filename of the images, recommended to use something like Images/image.png");
    arguments.getApplicationUsage()->addCommandLineOption("--cs <filename>","Load pre-generated configuration file for run.");
    arguments.getApplicationUsage()->addCommandLineOption("--ouput-cs <filename>","Output configuration file with settings provided on commandline.");
    arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Use specificied camera path file to control camera position.");
    arguments.getApplicationUsage()->addCommandLineOption("--offscreen","Use an pbuffer to render the images offscreen.");
    arguments.getApplicationUsage()->addCommandLineOption("--screen","Use an window to render the images.");
    arguments.getApplicationUsage()->addCommandLineOption("--width <width>","Window/output image width.");
    arguments.getApplicationUsage()->addCommandLineOption("--height <height>","Window/output image height.");
    arguments.getApplicationUsage()->addCommandLineOption("--screen-distance <distance>","Set the distance of the viewer from the physical screen.");
    arguments.getApplicationUsage()->addCommandLineOption("--screen-width <width>","Set the width of the physical screen.");
    arguments.getApplicationUsage()->addCommandLineOption("--screen-height <height>","Set the height of the physical screen.");
    arguments.getApplicationUsage()->addCommandLineOption("--ms <s>","Number of multi-samples to use when rendering, an enable a single sample buffer.");
    arguments.getApplicationUsage()->addCommandLineOption("--samples <s>","Number of multi-samples to use when rendering.");
    arguments.getApplicationUsage()->addCommandLineOption("--sampleBuffers <sb>","Number of sample buffers to use when rendering.");
    arguments.getApplicationUsage()->addCommandLineOption("-f <fps>","Number of frames per second in simulation time.");
    arguments.getApplicationUsage()->addCommandLineOption("-n <frames>","Number of frames to render/images to create.");
    arguments.getApplicationUsage()->addCommandLineOption("-d <time>","Duration of rendering run (duration = frames/fps).");
    arguments.getApplicationUsage()->addCommandLineOption("--center <x> <y> <z>","View center.");
    arguments.getApplicationUsage()->addCommandLineOption("--eye <x> <y> <z>","Camera eye point.");
    arguments.getApplicationUsage()->addCommandLineOption("--up <x> <y> <z>","Camera up vector.");
    arguments.getApplicationUsage()->addCommandLineOption("--rotation-center <x> <y> <z>","Position to rotatate around.");
    arguments.getApplicationUsage()->addCommandLineOption("--rotation-axis <x> <y> <z>","Axis to rotate around.");
    arguments.getApplicationUsage()->addCommandLineOption("--rotation-speed <v>","Degrees per second.");
    arguments.getApplicationUsage()->addCommandLineOption("--stereo <mode>","OFF | HORIZONTAL_SPLIT | VERTICAL_SPLIT");

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    osgViewer::Viewer viewer;

    typedef std::list< osg::ref_ptr<gsc::CaptureSettings> > CaptureSettingsList;
    CaptureSettingsList frameCaptureList;

    osg::ref_ptr<gsc::CaptureSettings> fc = new gsc::CaptureSettings;

    double duration = 0.0;
    double fps = 0.0f;
    unsigned int nframes = 0;

    bool readCaptureSettings = false;
    std::string filename;
    if (arguments.read("--cs",filename))
    {
        osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(filename);
        gsc::CaptureSettings* input_cs = dynamic_cast<gsc::CaptureSettings*>(object.get());
        if (input_cs) { fc = input_cs; readCaptureSettings = true; }
        else
        {
            OSG_NOTICE<<"Unable to read CaptureSettings from file: "<<filename<<std::endl;
            if (object.valid()) OSG_NOTICE<<"Object read, "<<object.get()<<", className()="<<object->className()<<std::endl;
            return 1;
        }
    }

    float screenWidth = fc->getScreenWidth()!=0.0 ? fc->getScreenWidth() : osg::DisplaySettings::instance()->getScreenWidth();
    if (arguments.read("--screen-width",screenWidth)) {}

    float screenHeight = fc->getScreenHeight()!=0.0 ? fc->getScreenHeight() : osg::DisplaySettings::instance()->getScreenHeight();
    if (arguments.read("--screen-height",screenHeight)) {}

    float screenDistance = fc->getScreenDistance()!=0.0 ? fc->getScreenDistance() : osg::DisplaySettings::instance()->getScreenDistance();
    if (arguments.read("--screen-distance",screenDistance)) {}

    fc->setScreenWidth(screenWidth);
    osg::DisplaySettings::instance()->setScreenWidth(screenWidth);
    
    fc->setScreenHeight(screenHeight);
    osg::DisplaySettings::instance()->setScreenHeight(screenHeight);

    fc->setScreenDistance(screenDistance);
    osg::DisplaySettings::instance()->setScreenDistance(screenDistance);

    bool useScreenSizeForProjectionMatrix = true;

    if (arguments.read("-i",filename)) fc->setInputFileName(filename);
    if (arguments.read("-o",filename)) fc->setOutputFileName(filename);
    if (arguments.read("-p",filename))
    {
        osg::ref_ptr<gsc::CameraPathProperty> cpp = new gsc::CameraPathProperty;
        cpp->setAnimationPathFileName(filename);

        double startTime = 0, endTime = 1.0f;
        if (cpp->getTimeRange(startTime, endTime))
        {
            OSG_NOTICE<<"Camera path time range "<<startTime<<", "<<endTime<<std::endl;
            if (startTime!=0.0)
            {
                cpp->resetTimeRange(0.0, endTime-startTime);
                if (cpp->getTimeRange(startTime, endTime))
                {
                    OSG_NOTICE<<"   new time range "<<startTime<<", "<<endTime<<std::endl;
                }
                else
                {
                    OSG_NOTICE<<"   failed to set new time range "<<startTime<<", "<<endTime<<std::endl;
                }
            }
            duration = endTime;            
        }
        else
        {
            OSG_NOTICE<<"Camera path time range "<<startTime<<", "<<endTime<<std::endl;
        }

        fc->addUpdateProperty(cpp.get());
    }
    else
    {
        osg::ref_ptr<gsc::CameraProperty> cp = fc->getPropertyOfType<gsc::CameraProperty>();

        bool newCameraProperty = false;
        bool valueSet = false;
        
        if (!cp)
        {
            newCameraProperty = true;
            cp = new gsc::CameraProperty;

            osg::ref_ptr<osg::Node> node;
            if (!fc->getInputFileName().empty())
            {
                osgDB::readRefNodeFile(fc->getInputFileName());
            }
            if (node.valid())
            {
                cp->setToModel(node.get());
                valueSet = true;
            }
            
        }
        
        osg::Vec3d vec;
        while (arguments.read("--center",vec.x(), vec.y(), vec.z())) { cp->setCenter(vec); valueSet = true; }
        while (arguments.read("--eye",vec.x(), vec.y(), vec.z())) { cp->setEyePoint(vec); valueSet = true; }
        while (arguments.read("--up",vec.x(), vec.y(), vec.z())) { cp->setUpVector(vec); valueSet = true; }
        while (arguments.read("--rotation-center",vec.x(), vec.y(), vec.z())) { cp->setRotationCenter(vec); valueSet = true; }
        while (arguments.read("--rotation-axis",vec.x(), vec.y(), vec.z())) { cp->setRotationAxis(vec); valueSet = true; }

        double speed;
        while (arguments.read("--rotation-speed",speed)) { cp->setRotationSpeed(speed); valueSet = true; }

        if (newCameraProperty && valueSet)
        {
            fc->addUpdateProperty(cp.get());
        }
    }

    std::string stereoMode;
    if (arguments.read("--stereo", stereoMode))
    {        
        if      (stereoMode=="HORIZONTAL_SPLIT") fc->setStereoMode(gsc::CaptureSettings::HORIZONTAL_SPLIT);
        else if (stereoMode=="VERTICAL_SPLIT") fc->setStereoMode(gsc::CaptureSettings::VERTICAL_SPLIT);
        else if (stereoMode=="OFF") fc->setStereoMode(gsc::CaptureSettings::OFF);
    }

    if (arguments.read("--offscreen")) fc->setOffscreen(true);
    if (arguments.read("--screen")) fc->setOffscreen(false);

    if (arguments.read("--flip")) fc->setOutputImageFlip(true);
    if (arguments.read("--no-flip")) fc->setOutputImageFlip(false);

    unsigned int width = 1024;
    if (arguments.read("--width",width)) fc->setWidth(width);

    unsigned int height = 512;
    if (arguments.read("--height",height)) fc->setHeight(height);

    if (arguments.read("--rgb")) fc->setPixelFormat(gsc::CaptureSettings::RGB);
    if (arguments.read("--rgba")) fc->setPixelFormat(gsc::CaptureSettings::RGBA);
    
    osg::Vec4 clearColor(0.0f,0.0f,0.0f,0.0f);
    while (arguments.read("--clear-color",clearColor[0],clearColor[1],clearColor[2],clearColor[3])) {}


    unsigned int samples = 0;
    if (arguments.read("--samples",samples)) fc->setSamples(samples);

    unsigned int sampleBuffers = 0;
    if (arguments.read("--sampleBuffers",sampleBuffers)) fc->setSampleBuffers(sampleBuffers);

    unsigned int ms = 0;
    if (arguments.read("--ms",ms))
    {
        fc->setSamples(ms);
        fc->setSampleBuffers(1);
    }

    if (arguments.read("-f",fps)) fc->setFrameRate(fps);

    if (arguments.read("-n",nframes)) fc->setNumberOfFrames(nframes);

    if (arguments.read("-d",duration)) {}


    std::string key;
    double time;
    while(arguments.read("--key-down",time, key) && key.size()>=1)
    {
        OSG_NOTICE<<"keydown "<<key<<", "<<time<<std::endl;
        osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter;
        event->setTime(time);
        event->setEventType(osgGA::GUIEventAdapter::KEYDOWN);
        event->setKey(key[0]);
        fc->addUpdateProperty(new gsc::EventProperty(event.get()));
    }

    while(arguments.read("--key-up",time, key) && key.size()>=1)
    {
        OSG_NOTICE<<"keyup "<<key<<", "<<time<<std::endl;
        osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter;
        event->setTime(time);
        event->setEventType(osgGA::GUIEventAdapter::KEYUP);
        event->setKey(key[0]);
        fc->addUpdateProperty(new gsc::EventProperty(event.get()));
    }

    double mouse_x, mouse_y;
    while(arguments.read("--mouse-move",time, mouse_x, mouse_y))
    {
        OSG_NOTICE<<"mouse move "<<time<<", "<<mouse_x<<", "<<mouse_y<<std::endl;
        osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter;
        event->setTime(time);
        event->setEventType(osgGA::GUIEventAdapter::MOVE);
        event->setX(mouse_x);
        event->setY(mouse_y);
        fc->addUpdateProperty(new gsc::EventProperty(event.get()));
    }

    while(arguments.read("--mouse-drag",time, mouse_x, mouse_y))
    {
        OSG_NOTICE<<"mouse drag "<<time<<", "<<mouse_x<<", "<<mouse_y<<std::endl;
        osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter;
        event->setTime(time);
        event->setEventType(osgGA::GUIEventAdapter::DRAG);
        event->setX(mouse_x);
        event->setY(mouse_y);
        fc->addUpdateProperty(new gsc::EventProperty(event.get()));
    }


    if (!readCaptureSettings)
    {
        if (duration!=0.0)
        {
            if (fps!=0.0) nframes = static_cast<unsigned int>(ceil(duration*fps));
            else if (nframes!=0) fps = duration/static_cast<double>(nframes);
            else
            {
                fps = 60.0;
                nframes = static_cast<unsigned int>(ceil(duration/fps));
            }
        }
        else // duration == 0.0
        {
            if (fps==0.0) fps=60.0;
            if (nframes==0) nframes=1;

            duration = static_cast<double>(nframes)/fps;
        }

        fc->setNumberOfFrames(nframes);
        fc->setFrameRate(fps);
        OSG_NOTICE<<"Duration="<<duration<<", FPS="<<fps<<", Number of Frames="<<nframes<<std::endl;
    }
    



    if (arguments.read("--output-cs",filename))
    {
        osgDB::writeObjectFile(*fc, filename);
        return 1;
    }

    

    if (fc.valid())
    {
        frameCaptureList.push_back(fc);
    }

    if (frameCaptureList.empty())
    {
        OSG_NOTICE<<"No settings provided"<<std::endl;
        return 1;
    }


    // setup viewer
    {
        osg::ref_ptr<osg::DisplaySettings> ds = new osg::DisplaySettings;

        bool stereo = fc->getStereoMode()!=gsc::CaptureSettings::OFF;
        osg::DisplaySettings::StereoMode stereoMode = fc->getStereoMode()==gsc::CaptureSettings::VERTICAL_SPLIT ? osg::DisplaySettings::VERTICAL_SPLIT : osg::DisplaySettings::HORIZONTAL_SPLIT;
        double fovx_multiple = fc->getStereoMode()==gsc::CaptureSettings::HORIZONTAL_SPLIT ? 2.0 : 1;
        double fovy_multiple = fc->getStereoMode()==gsc::CaptureSettings::VERTICAL_SPLIT ? 2.0 : 1;
        ds->setStereoMode(stereoMode);
        ds->setStereo(stereo);

        if (fc->getScreenWidth()!=0.0) ds->setScreenWidth(fc->getScreenWidth());
        if (fc->getScreenHeight()!=0.0) ds->setScreenHeight(fc->getScreenHeight());
        if (fc->getScreenDistance()!=0.0) ds->setScreenDistance(fc->getScreenDistance());

        
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds.get());

        traits->readDISPLAY();
        if (traits->displayNum<0) traits->displayNum = 0;

        traits->x = 0;
        traits->y = 0;
        traits->width = fc->getWidth();
        traits->height = fc->getHeight();
        traits->alpha = (fc->getPixelFormat() == gsc::CaptureSettings::RGBA) ? 8 : 0;
        traits->samples = fc->getSamples();
        traits->sampleBuffers = fc->getSampleBuffers();
        traits->windowDecoration = !(fc->getOffscreen());
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->pbuffer = fc->getOffscreen();

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (!gc)
        {
            OSG_NOTICE<<"Failed to created requested graphics context"<<std::endl;
            return 1;
        }

        viewer.getCamera()->setClearColor(clearColor);
        viewer.getCamera()->setGraphicsContext(gc.get());
        viewer.getCamera()->setDisplaySettings(ds.get());

        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
        if (gw)
        {
            OSG_INFO<<"GraphicsWindow has been created successfully."<<std::endl;
            gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(0, 0, fc->getWidth(),  fc->getHeight());
        }
        else
        {
            OSG_NOTICE<<"PixelBuffer has been created succseffully "<<traits->width<<", "<<traits->height<<std::endl;
        }

        if (useScreenSizeForProjectionMatrix)
        {
            OSG_NOTICE<<"Setting projection matrix"<<std::endl;
            
            double vfov = osg::RadiansToDegrees(atan2(screenHeight/2.0f,screenDistance)*2.0);
            // double hfov = osg::RadiansToDegrees(atan2(width/2.0f,distance)*2.0);

            viewer.getCamera()->setProjectionMatrixAsPerspective( vfov*fovy_multiple, (screenWidth/screenHeight)*fovx_multiple, 0.1, 1000.0);

            OSG_NOTICE<<"setProjectionMatrixAsPerspective( "<<vfov*fovy_multiple<<", "<<(screenWidth/screenHeight)*fovx_multiple<<", "<<0.1<<", "<<1000.0<<");"<<std::endl;

            
        }
        else
        {
            double fovy, aspectRatio, zNear, zFar;
            viewer.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

            double newAspectRatio = double(traits->width) / double(traits->height);
            double aspectRatioChange = newAspectRatio / aspectRatio;
            if (aspectRatioChange != 1.0)
            {
                viewer.getCamera()->getProjectionMatrix() *= osg::Matrix::scale(fovx_multiple/aspectRatioChange,fovy_multiple,1.0);
            }
        }

        // set up stereo masks
        viewer.getCamera()->setCullMask(0xffffffff);
        viewer.getCamera()->setCullMaskLeft(0x00000001);
        viewer.getCamera()->setCullMaskRight(0x00000002);

        viewer.getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;

        viewer.getCamera()->setDrawBuffer(buffer);
        viewer.getCamera()->setReadBuffer(buffer);
    }

    std::string outputPath = osgDB::getFilePath(fc->getOutputFileName());
    if (!outputPath.empty())
    {
        osgDB::FileType type = osgDB::fileType(outputPath);
        switch(type)
        {
            case(osgDB::FILE_NOT_FOUND):
                if (!osgDB::makeDirectory(outputPath))
                {
                    OSG_NOTICE<<"Error: could not create directory ["<<outputPath<<"]."<<std::endl;                    
                    return 1;
                }
                OSG_NOTICE<<"Created directory ["<<outputPath<<"]."<<std::endl;
                break;
            case(osgDB::REGULAR_FILE):
                OSG_NOTICE<<"Error: filepath for output files is regular file, not a directory as required."<<std::endl;
                return 1;
            case(osgDB::DIRECTORY):
                OSG_NOTICE<<"Valid path["<<outputPath<<"] provided for output files."<<std::endl;
                break;
        }
    }

    GLenum pixelFormat = (fc->getPixelFormat()==gsc::CaptureSettings::RGBA) ? GL_RGBA : GL_RGB;
    

    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    viewer.realize();

    // set up screen shot
    osg::ref_ptr<ScreenShot> screenShot = new ScreenShot(pixelFormat, fc->getOutputImageFlip());;
    {

        osgViewer::Viewer::Cameras cameras;
        viewer.getCameras(cameras);
        if (cameras.size()>1)
        {
            unsigned int cameraNum = 0;
            for(osgViewer::Viewer::Cameras::iterator itr = cameras.begin();
                itr != cameras.end();
                ++itr, ++cameraNum)
            {
                osg::Camera* camera = *itr;
                camera->setFinalDrawCallback(screenShot.get());
                screenShot->_cameraNumMap[camera] = cameraNum;
            }
        }
        else if (cameras.size()==1)
        {
            osg::Camera* camera = cameras.front();
            camera->setFinalDrawCallback(screenShot.get());
        }
        else
        {
            OSG_NOTICE<<"No usable Cameras created."<<std::endl;
            return 1;
        }
    }

    for(CaptureSettingsList::iterator itr = frameCaptureList.begin();
        itr != frameCaptureList.end();
        ++itr)
    {
        gsc::CaptureSettings* fc = itr->get();
        screenShot->_frameCapture = fc;

        osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile(fc->getInputFileName());
        if (!model) break;

        viewer.setSceneData(model.get());

        double simulationTime = 0.0;
                
        for(unsigned int i=0; i<fc->getNumberOfFrames(); ++i)
        {
            OSG_NOTICE<<"fc.getOutputFileName("<<i<<")="<<fc->getOutputFileName(i)<<std::endl;

            viewer.advance(simulationTime);
            
            gsc::CaptureSettings::Properties& pl = fc->getProperties();
            for(gsc::CaptureSettings::Properties::iterator plitr = pl.begin();
                plitr != pl.end();
                ++plitr)
            {
                (*plitr)->update(&viewer);
            }

            viewer.eventTraversal();
            viewer.updateTraversal();
            viewer.renderingTraversals();

            // advance simulationTime and number of frames rendered
            simulationTime += 1.0/fc->getFrameRate();
        }
    }

    osg::ref_ptr<osg::Object> object = new osgGA::StateSetManipulator;
    osg::ref_ptr<osgGA::StateSetManipulator> ss = dynamic_cast<osgGA::StateSetManipulator*>(object.get());
   
    return 0;
}
