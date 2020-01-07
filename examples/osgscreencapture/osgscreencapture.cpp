/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial applications,
 * as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <iostream>
#include <sstream>
#include <string.h>

class WindowCaptureCallback : public osg::Camera::DrawCallback
{
    public:

        enum Mode
        {
            READ_PIXELS,
            SINGLE_PBO,
            DOUBLE_PBO,
            TRIPLE_PBO
        };

        enum FramePosition
        {
            START_FRAME,
            END_FRAME
        };

        struct ContextData : public osg::Referenced
        {

            ContextData(osg::GraphicsContext* gc, Mode mode, GLenum readBuffer, const std::string& name):
                _gc(gc),
                _mode(mode),
                _readBuffer(readBuffer),
                _fileName(name),
                _pixelFormat(GL_BGRA),
                _type(GL_UNSIGNED_BYTE),
                _width(0),
                _height(0),
                _currentImageIndex(0),
                _currentPboIndex(0),
                _reportTimingFrequency(100),
                _numTimeValuesRecorded(0),
                _timeForReadPixels(0.0),
                _timeForFullCopy(0.0),
                _timeForMemCpy(0.0)

            {
                _previousFrameTick = osg::Timer::instance()->tick();

                if (gc->getTraits())
                {
                    if (gc->getTraits()->alpha)
                    {
                        osg::notify(osg::NOTICE)<<"Select GL_BGRA read back format"<<std::endl;
                        _pixelFormat = GL_BGRA;
                    }
                    else
                    {
                        osg::notify(osg::NOTICE)<<"Select GL_BGR read back format"<<std::endl;
                        _pixelFormat = GL_BGR;
                    }
                }

                getSize(gc, _width, _height);

                std::cout<<"Window size "<<_width<<", "<<_height<<std::endl;

                // single buffered image
                _imageBuffer.push_back(new osg::Image);

                // double buffer PBO.
                switch(_mode)
                {
                    case(READ_PIXELS):
                        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with out PixelBufferObject."<<std::endl;
                        break;
                    case(SINGLE_PBO):
                        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with a single PixelBufferObject."<<std::endl;
                        _pboBuffer.push_back(0);
                        break;
                    case(DOUBLE_PBO):
                        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with a double buffer PixelBufferObject."<<std::endl;
                        _pboBuffer.push_back(0);
                        _pboBuffer.push_back(0);
                        break;
                    case(TRIPLE_PBO):
                        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with a triple buffer PixelBufferObject."<<std::endl;
                        _pboBuffer.push_back(0);
                        _pboBuffer.push_back(0);
                        _pboBuffer.push_back(0);
                        break;
                    default:
                        break;
                }
            }

            void getSize(osg::GraphicsContext* gc, int& width, int& height)
            {
                if (gc->getTraits())
                {
                    width = gc->getTraits()->width;
                    height = gc->getTraits()->height;
                }
            }

            void updateTimings(osg::Timer_t tick_start,
                               osg::Timer_t tick_afterReadPixels,
                               osg::Timer_t tick_afterMemCpy,
                               unsigned int dataSize);

            void read()
            {
                osg::GLExtensions* ext = osg::GLExtensions::Get(_gc->getState()->getContextID(),true);

                if (ext->isPBOSupported && !_pboBuffer.empty())
                {
                    if (_pboBuffer.size()==1)
                    {
                        singlePBO(ext);
                    }
                    else
                    {
                        multiPBO(ext);
                    }
                }
                else
                {
                    readPixels();
                }
            }

            void readPixels();

            void singlePBO(osg::GLExtensions* ext);

            void multiPBO(osg::GLExtensions* ext);

            typedef std::vector< osg::ref_ptr<osg::Image> >             ImageBuffer;
            typedef std::vector< GLuint > PBOBuffer;

            osg::GraphicsContext*   _gc;
            Mode                    _mode;
            GLenum                  _readBuffer;
            std::string             _fileName;

            GLenum                  _pixelFormat;
            GLenum                  _type;
            int                     _width;
            int                     _height;

            unsigned int            _currentImageIndex;
            ImageBuffer             _imageBuffer;

            unsigned int            _currentPboIndex;
            PBOBuffer               _pboBuffer;

            unsigned int            _reportTimingFrequency;
            unsigned int            _numTimeValuesRecorded;
            double                  _timeForReadPixels;
            double                  _timeForFullCopy;
            double                  _timeForMemCpy;
            osg::Timer_t            _previousFrameTick;
        };

        WindowCaptureCallback(Mode mode, FramePosition position, GLenum readBuffer):
            _mode(mode),
            _position(position),
            _readBuffer(readBuffer)
        {
        }

        FramePosition getFramePosition() const { return _position; }

        ContextData* createContextData(osg::GraphicsContext* gc) const
        {
            std::stringstream filename;
            filename << "test_"<<_contextDataMap.size()<<".jpg";
            return new ContextData(gc, _mode, _readBuffer, filename.str());
        }

        ContextData* getContextData(osg::GraphicsContext* gc) const
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            osg::ref_ptr<ContextData>& data = _contextDataMap[gc];
            if (!data) data = createContextData(gc);

            return data.get();
        }

        virtual void operator () (osg::RenderInfo& renderInfo) const
        {
            glReadBuffer(_readBuffer);

            osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
            osg::ref_ptr<ContextData> cd = getContextData(gc);
            cd->read();
        }

        typedef std::map<osg::GraphicsContext*, osg::ref_ptr<ContextData> > ContextDataMap;

        Mode                        _mode;
        FramePosition               _position;
        GLenum                      _readBuffer;
        mutable OpenThreads::Mutex  _mutex;
        mutable ContextDataMap      _contextDataMap;


};

void WindowCaptureCallback::ContextData::updateTimings(osg::Timer_t tick_start,
                                                       osg::Timer_t tick_afterReadPixels,
                                                       osg::Timer_t tick_afterMemCpy,
                                                       unsigned int dataSize)
{
    if (!_reportTimingFrequency) return;

    double timeForReadPixels = osg::Timer::instance()->delta_s(tick_start, tick_afterReadPixels);
    double timeForFullCopy = osg::Timer::instance()->delta_s(tick_start, tick_afterMemCpy);
    double timeForMemCpy = osg::Timer::instance()->delta_s(tick_afterReadPixels, tick_afterMemCpy);

    _timeForReadPixels += timeForReadPixels;
    _timeForFullCopy += timeForFullCopy;
    _timeForMemCpy += timeForMemCpy;

    ++_numTimeValuesRecorded;

    if (_numTimeValuesRecorded==_reportTimingFrequency)
    {
        timeForReadPixels = _timeForReadPixels/double(_numTimeValuesRecorded);
        timeForFullCopy = _timeForFullCopy/double(_numTimeValuesRecorded);
        timeForMemCpy = _timeForMemCpy/double(_numTimeValuesRecorded);

        double averageFrameTime =  osg::Timer::instance()->delta_s(_previousFrameTick, tick_afterMemCpy)/double(_numTimeValuesRecorded);
        double fps = 1.0/averageFrameTime;
        _previousFrameTick = tick_afterMemCpy;

        _timeForReadPixels = 0.0;
        _timeForFullCopy = 0.0;
        _timeForMemCpy = 0.0;

        _numTimeValuesRecorded = 0;

        double numMPixels = double(_width * _height) / 1000000.0;
        double numMb = double(dataSize) / (1024*1024);

        int prec = osg::notify(osg::NOTICE).precision(5);

        if (timeForMemCpy==0.0)
        {
            osg::notify(osg::NOTICE)<<"fps = "<<fps<<", full frame copy = "<<timeForFullCopy*1000.0f<<"ms rate = "<<numMPixels / timeForFullCopy<<" Mpixel/sec, copy speed = "<<numMb / timeForFullCopy<<" Mb/sec"<<std::endl;
        }
        else
        {
            osg::notify(osg::NOTICE)<<"fps = "<<fps<<", full frame copy = "<<timeForFullCopy*1000.0f<<"ms rate = "<<numMPixels / timeForFullCopy<<" Mpixel/sec, "<<numMb / timeForFullCopy<< " Mb/sec "<<
                                      "time for memcpy = "<<timeForMemCpy*1000.0<<"ms  memcpy speed = "<<numMb / timeForMemCpy<<" Mb/sec"<<std::endl;
        }
        osg::notify(osg::NOTICE).precision(prec);

    }

}

void WindowCaptureCallback::ContextData::readPixels()
{
    // std::cout<<"readPixels("<<_fileName<<" image "<<_currentImageIndex<<" "<<_currentPboIndex<<std::endl;

    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();
    unsigned int nextPboIndex = _pboBuffer.empty() ? 0 : (_currentPboIndex+1)%_pboBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        std::cout<<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    osg::Image* image = _imageBuffer[_currentImageIndex].get();

    osg::Timer_t tick_start = osg::Timer::instance()->tick();

#if 1
    image->readPixels(0,0,_width,_height,
                      _pixelFormat,_type);
#endif

    osg::Timer_t tick_afterReadPixels = osg::Timer::instance()->tick();

    updateTimings(tick_start, tick_afterReadPixels, tick_afterReadPixels, image->getTotalSizeInBytes());

    if (!_fileName.empty())
    {
        // osgDB::writeImageFile(*image, _fileName);
    }

    _currentImageIndex = nextImageIndex;
    _currentPboIndex = nextPboIndex;
}

void WindowCaptureCallback::ContextData::singlePBO(osg::GLExtensions* ext)
{
    // std::cout<<"singelPBO(  "<<_fileName<<" image "<<_currentImageIndex<<" "<<_currentPboIndex<<std::endl;

    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        std::cout<<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    GLuint& pbo = _pboBuffer[0];

    osg::Image* image = _imageBuffer[_currentImageIndex].get();
    if (image->s() != _width ||
        image->t() != _height)
    {
        osg::notify(osg::NOTICE)<<"Allocating image "<<std::endl;
        image->allocateImage(_width, _height, 1, _pixelFormat, _type);

        if (pbo!=0)
        {
            osg::notify(osg::NOTICE)<<"deleting pbo "<<pbo<<std::endl;
            ext->glDeleteBuffers (1, &pbo);
            pbo = 0;
        }
    }


    if (pbo==0)
    {
        ext->glGenBuffers(1, &pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        osg::notify(osg::NOTICE)<<"Generating pbo "<<pbo<<std::endl;
    }
    else
    {
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
    }

    osg::Timer_t tick_start = osg::Timer::instance()->tick();

#if 1
    glReadPixels(0, 0, _width, _height, _pixelFormat, _type, 0);
#endif

    osg::Timer_t tick_afterReadPixels = osg::Timer::instance()->tick();

    GLubyte* src = (GLubyte*)ext->glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB,
                                              GL_READ_ONLY_ARB);
    if(src)
    {
        memcpy(image->data(), src, image->getTotalSizeInBytes());

        ext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
    }

    ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

    osg::Timer_t tick_afterMemCpy = osg::Timer::instance()->tick();

    updateTimings(tick_start, tick_afterReadPixels, tick_afterMemCpy, image->getTotalSizeInBytes());

    if (!_fileName.empty())
    {
        // osgDB::writeImageFile(*image, _fileName);
    }


    _currentImageIndex = nextImageIndex;
}

void WindowCaptureCallback::ContextData::multiPBO(osg::GLExtensions* ext)
{
    // std::cout<<"multiPBO(  "<<_fileName<<" image "<<_currentImageIndex<<" "<<_currentPboIndex<<std::endl;
    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();
    unsigned int nextPboIndex = (_currentPboIndex+1)%_pboBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        std::cout<<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    GLuint& copy_pbo = _pboBuffer[_currentPboIndex];
    GLuint& read_pbo = _pboBuffer[nextPboIndex];

    osg::Image* image = _imageBuffer[_currentImageIndex].get();
    if (image->s() != _width ||
        image->t() != _height)
    {
        osg::notify(osg::NOTICE)<<"Allocating image "<<std::endl;
        image->allocateImage(_width, _height, 1, _pixelFormat, _type);

        if (read_pbo!=0)
        {
            osg::notify(osg::NOTICE)<<"deleting pbo "<<read_pbo<<std::endl;
            ext->glDeleteBuffers (1, &read_pbo);
            read_pbo = 0;
        }

        if (copy_pbo!=0)
        {
            osg::notify(osg::NOTICE)<<"deleting pbo "<<copy_pbo<<std::endl;
            ext->glDeleteBuffers (1, &copy_pbo);
            copy_pbo = 0;
        }
    }


    bool doCopy = copy_pbo!=0;
    if (copy_pbo==0)
    {
        ext->glGenBuffers(1, &copy_pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, copy_pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        osg::notify(osg::NOTICE)<<"Generating pbo "<<read_pbo<<std::endl;
    }

    if (read_pbo==0)
    {
        ext->glGenBuffers(1, &read_pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, read_pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        osg::notify(osg::NOTICE)<<"Generating pbo "<<read_pbo<<std::endl;
    }
    else
    {
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, read_pbo);
    }

    osg::Timer_t tick_start = osg::Timer::instance()->tick();

#if 1
    glReadPixels(0, 0, _width, _height, _pixelFormat, _type, 0);
#endif

    osg::Timer_t tick_afterReadPixels = osg::Timer::instance()->tick();

    if (doCopy)
    {

        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, copy_pbo);

        GLubyte* src = (GLubyte*)ext->glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB,
                                                  GL_READ_ONLY_ARB);
        if(src)
        {
            memcpy(image->data(), src, image->getTotalSizeInBytes());
            ext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
        }

        if (!_fileName.empty())
        {
            // osgDB::writeImageFile(*image, _fileName);
        }
    }

    ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

    osg::Timer_t tick_afterMemCpy = osg::Timer::instance()->tick();

    updateTimings(tick_start, tick_afterReadPixels, tick_afterMemCpy, image->getTotalSizeInBytes());

    _currentImageIndex = nextImageIndex;
    _currentPboIndex = nextPboIndex;
}

void addCallbackToViewer(osgViewer::ViewerBase& viewer, WindowCaptureCallback* callback)
{

    if (callback->getFramePosition()==WindowCaptureCallback::START_FRAME)
    {
        osgViewer::ViewerBase::Windows windows;
        viewer.getWindows(windows);
        for(osgViewer::ViewerBase::Windows::iterator itr = windows.begin();
            itr != windows.end();
            ++itr)
        {
            osgViewer::GraphicsWindow* window = *itr;
            osg::GraphicsContext::Cameras& cameras = window->getCameras();
            osg::Camera* firstCamera = 0;
            for(osg::GraphicsContext::Cameras::iterator cam_itr = cameras.begin();
                cam_itr != cameras.end();
                ++cam_itr)
            {
                if (firstCamera)
                {
                    if ((*cam_itr)->getRenderOrder() < firstCamera->getRenderOrder())
                    {
                        firstCamera = (*cam_itr);
                    }
                    if ((*cam_itr)->getRenderOrder() == firstCamera->getRenderOrder() &&
                        (*cam_itr)->getRenderOrderNum() < firstCamera->getRenderOrderNum())
                    {
                        firstCamera = (*cam_itr);
                    }
                }
                else
                {
                    firstCamera = *cam_itr;
                }
            }

            if (firstCamera)
            {
                osg::notify(osg::NOTICE)<<"First camera "<<firstCamera<<std::endl;

                firstCamera->setInitialDrawCallback(callback);
            }
            else
            {
                osg::notify(osg::NOTICE)<<"No camera found"<<std::endl;
            }
        }
    }
    else
    {
        osgViewer::ViewerBase::Windows windows;
        viewer.getWindows(windows);
        for(osgViewer::ViewerBase::Windows::iterator itr = windows.begin();
            itr != windows.end();
            ++itr)
        {
            osgViewer::GraphicsWindow* window = *itr;
            osg::GraphicsContext::Cameras& cameras = window->getCameras();
            osg::Camera* lastCamera = 0;
            for(osg::GraphicsContext::Cameras::iterator cam_itr = cameras.begin();
                cam_itr != cameras.end();
                ++cam_itr)
            {
                if (lastCamera)
                {
                    if ((*cam_itr)->getRenderOrder() > lastCamera->getRenderOrder())
                    {
                        lastCamera = (*cam_itr);
                    }
                    if ((*cam_itr)->getRenderOrder() == lastCamera->getRenderOrder() &&
                        (*cam_itr)->getRenderOrderNum() >= lastCamera->getRenderOrderNum())
                    {
                        lastCamera = (*cam_itr);
                    }
                }
                else
                {
                    lastCamera = *cam_itr;
                }
            }

            if (lastCamera)
            {
                osg::notify(osg::NOTICE)<<"Last camera "<<lastCamera<<std::endl;

                lastCamera->setFinalDrawCallback(callback);
            }
            else
            {
                osg::notify(osg::NOTICE)<<"No camera found"<<std::endl;
            }
        }
    }
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");

    osgViewer::Viewer viewer(arguments);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        std::string pathfile;
        char keyForAnimationPath = '5';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid())
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the thread model handler
    viewer.addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // add the record camera path handler
    viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the LOD Scale handler
    viewer.addEventHandler(new osgViewer::LODScaleHandler);

    GLenum readBuffer = GL_BACK;
    WindowCaptureCallback::FramePosition position = WindowCaptureCallback::END_FRAME;
    WindowCaptureCallback::Mode mode = WindowCaptureCallback::DOUBLE_PBO;

    while (arguments.read("--start-frame")) { position = WindowCaptureCallback::START_FRAME; readBuffer = GL_FRONT; }
    while (arguments.read("--end-frame")) position = WindowCaptureCallback::END_FRAME;

    while (arguments.read("--front")) readBuffer = GL_FRONT;
    while (arguments.read("--back")) readBuffer = GL_BACK;

    while (arguments.read("--no-pbo")) mode = WindowCaptureCallback::READ_PIXELS;
    while (arguments.read("--single-pbo")) mode = WindowCaptureCallback::SINGLE_PBO;
    while (arguments.read("--double-pbo")) mode = WindowCaptureCallback::DOUBLE_PBO;
    while (arguments.read("--triple-pbo")) mode = WindowCaptureCallback::TRIPLE_PBO;


    unsigned int width=1280;
    unsigned int height=1024;
    bool pbufferOnly = false;
    osg::ref_ptr<osg::GraphicsContext> pbuffer;
    if (arguments.read("--pbuffer",width,height) ||
        (pbufferOnly = arguments.read("--pbuffer-only",width,height)))
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 0;
        traits->y = 0;
        traits->width = width;
        traits->height = height;
        traits->red = 8;
        traits->green = 8;
        traits->blue = 8;
        traits->alpha = 8;
        traits->windowDecoration = false;
        traits->pbuffer = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        pbuffer = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (pbuffer.valid())
        {
            osg::notify(osg::NOTICE)<<"Pixel buffer has been created successfully."<<std::endl;
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Pixel buffer has not been created successfully."<<std::endl;
        }

    }

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel);

    viewer.setSceneData(loadedModel);


    if (pbuffer.valid())
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(pbuffer.get());
        camera->setViewport(new osg::Viewport(0,0,width,height));
        GLenum buffer = pbuffer->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setFinalDrawCallback(new WindowCaptureCallback(mode, position, readBuffer));

        if (pbufferOnly)
        {
            viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());

            viewer.realize();
        }
        else
        {
            viewer.realize();

            viewer.stopThreading();

            pbuffer->realize();

            viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());

            viewer.startThreading();
        }
    }
    else
    {
        viewer.realize();

        addCallbackToViewer(viewer, new WindowCaptureCallback(mode, position, readBuffer));
    }

    return viewer.run();

}
