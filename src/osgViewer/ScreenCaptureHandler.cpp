/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
*
* This library is open source and may be redistributed and/or modified under
* the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
* (at your option) any later version.  The full license is in LICENSE file
* included with this distribution, and on the openscenegraph.org website.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* OpenSceneGraph Public License for more details.
*/

#include <osgDB/WriteFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <string.h>

namespace osgViewer
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  WindowCaptureCallback
//

// From osgscreencapture example
/** Callback which will be added to a viewer's camera to do the actual screen capture. */
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

        WindowCaptureCallback(int numFrames, Mode mode, FramePosition position, GLenum readBuffer);

        FramePosition getFramePosition() const { return _position; }

        void setCaptureOperation(ScreenCaptureHandler::CaptureOperation* operation);
        ScreenCaptureHandler::CaptureOperation* getCaptureOperation() { return _contextDataMap.begin()->second->_captureOperation.get(); }

        void setFramesToCapture(int numFrames) { _numFrames = numFrames; }
        int getFramesToCapture() const { return _numFrames; }

        virtual void operator () (osg::RenderInfo& renderInfo) const;

        struct OSGVIEWER_EXPORT ContextData : public osg::Referenced
        {
            ContextData(osg::GraphicsContext* gc, Mode mode, GLenum readBuffer);

            void getSize(osg::GraphicsContext* gc, int& width, int& height);

            void updateTimings(osg::Timer_t tick_start,
                               osg::Timer_t tick_afterReadPixels,
                               osg::Timer_t tick_afterMemCpy,
                               osg::Timer_t tick_afterCaptureOperation,
                               unsigned int dataSize);

            void read();
            void readPixels();
            void singlePBO(osg::GLBufferObject::Extensions* ext);
            void multiPBO(osg::GLBufferObject::Extensions* ext);

            typedef std::vector< osg::ref_ptr<osg::Image> >             ImageBuffer;
            typedef std::vector< GLuint > PBOBuffer;

            osg::GraphicsContext*   _gc;
            unsigned int            _index;
            Mode                    _mode;
            GLenum                  _readBuffer;

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
            double                  _timeForMemCpy;
            double                  _timeForCaptureOperation;
            double                  _timeForFullCopy;
            double                  _timeForFullCopyAndOperation;
            osg::Timer_t            _previousFrameTick;

            osg::ref_ptr<ScreenCaptureHandler::CaptureOperation> _captureOperation;
        };

        typedef std::map<osg::GraphicsContext*, osg::ref_ptr<ContextData> > ContextDataMap;

        ContextData* createContextData(osg::GraphicsContext* gc) const;
        ContextData* getContextData(osg::GraphicsContext* gc) const;

        Mode                        _mode;
        FramePosition               _position;
        GLenum                      _readBuffer;
        mutable OpenThreads::Mutex  _mutex;
        mutable ContextDataMap      _contextDataMap;
        mutable int                 _numFrames;

        osg::ref_ptr<ScreenCaptureHandler::CaptureOperation> _defaultCaptureOperation;
};


WindowCaptureCallback::ContextData::ContextData(osg::GraphicsContext* gc, Mode mode, GLenum readBuffer)
    : _gc(gc),
      _index(_gc->getState()->getContextID()),
      _mode(mode),
      _readBuffer(readBuffer),
      _pixelFormat(GL_RGBA),
      _type(GL_UNSIGNED_BYTE),
      _width(0),
      _height(0),
      _currentImageIndex(0),
      _currentPboIndex(0),
      _reportTimingFrequency(100),
      _numTimeValuesRecorded(0),
      _timeForReadPixels(0.0),
      _timeForMemCpy(0.0),
      _timeForCaptureOperation(0.0),
      _timeForFullCopy(0.0),
      _timeForFullCopyAndOperation(0.0),
      _previousFrameTick(0)
{
    _previousFrameTick = osg::Timer::instance()->tick();

    osg::NotifySeverity level = osg::INFO;

    if (gc->getTraits())
    {
        if (gc->getTraits()->alpha)
        {
            OSG_NOTIFY(level)<<"ScreenCaptureHandler: Selected GL_RGBA read back format"<<std::endl;
            _pixelFormat = GL_RGBA;
        }
        else
        {
            OSG_NOTIFY(level)<<"ScreenCaptureHandler: Selected GL_RGB read back format"<<std::endl;
            _pixelFormat = GL_RGB;
        }
    }

    getSize(gc, _width, _height);

    //OSG_NOTICE<<"Window size "<<_width<<", "<<_height<<std::endl;

    // single buffered image
    _imageBuffer.push_back(new osg::Image);

    // double buffer PBO.
    switch(_mode)
    {
        case(READ_PIXELS):
            OSG_NOTIFY(level)<<"ScreenCaptureHandler: Reading window using glReadPixels, without PixelBufferObject."<<std::endl;
            break;
        case(SINGLE_PBO):
            OSG_NOTIFY(level)<<"ScreenCaptureHandler: Reading window using glReadPixels, with a single PixelBufferObject."<<std::endl;
            _pboBuffer.push_back(0);
            break;
        case(DOUBLE_PBO):
            OSG_NOTIFY(level)<<"ScreenCaptureHandler: Reading window using glReadPixels, with a double buffer PixelBufferObject."<<std::endl;
            _pboBuffer.push_back(0);
            _pboBuffer.push_back(0);
            break;
        case(TRIPLE_PBO):
            OSG_NOTIFY(level)<<"ScreenCaptureHandler: Reading window using glReadPixels, with a triple buffer PixelBufferObject."<<std::endl;
            _pboBuffer.push_back(0);
            _pboBuffer.push_back(0);
            _pboBuffer.push_back(0);
            break;
        default:
            break;
    }
}

void WindowCaptureCallback::ContextData::getSize(osg::GraphicsContext* gc, int& width, int& height)
{
    if (gc->getTraits())
    {
        width = gc->getTraits()->width;
        height = gc->getTraits()->height;
    }
}

void WindowCaptureCallback::ContextData::updateTimings(osg::Timer_t tick_start,
                                                       osg::Timer_t tick_afterReadPixels,
                                                       osg::Timer_t tick_afterMemCpy,
                                                       osg::Timer_t tick_afterCaptureOperation,
                                                       unsigned int /*dataSize*/)
{
    _timeForReadPixels = osg::Timer::instance()->delta_s(tick_start, tick_afterReadPixels);
    _timeForMemCpy = osg::Timer::instance()->delta_s(tick_afterReadPixels, tick_afterMemCpy);
    _timeForCaptureOperation = osg::Timer::instance()->delta_s(tick_afterMemCpy, tick_afterCaptureOperation);

    _timeForFullCopy = osg::Timer::instance()->delta_s(tick_start, tick_afterMemCpy);
    _timeForFullCopyAndOperation = osg::Timer::instance()->delta_s(tick_start, tick_afterCaptureOperation);
}

void WindowCaptureCallback::ContextData::read()
{
    osg::GLBufferObject::Extensions* ext = osg::GLBufferObject::getExtensions(_gc->getState()->getContextID(),true);

    if (ext->isPBOSupported() && !_pboBuffer.empty())
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


void WindowCaptureCallback::ContextData::readPixels()
{
    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();
    unsigned int nextPboIndex = _pboBuffer.empty() ? 0 : (_currentPboIndex+1)%_pboBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        //OSG_NOTICE<<"   Window resized "<<width<<", "<<height<<std::endl;
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

    if (_captureOperation.valid())
    {
        (*_captureOperation)(*image, _index);
    }

    osg::Timer_t tick_afterCaptureOperation = osg::Timer::instance()->tick();
    updateTimings(tick_start, tick_afterReadPixels, tick_afterReadPixels, tick_afterCaptureOperation, image->getTotalSizeInBytes());

    _currentImageIndex = nextImageIndex;
    _currentPboIndex = nextPboIndex;
}

void WindowCaptureCallback::ContextData::singlePBO(osg::GLBufferObject::Extensions* ext)
{
    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        //OSG_NOTICE<<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    GLuint& pbo = _pboBuffer[0];

    osg::Image* image = _imageBuffer[_currentImageIndex].get();
    if (image->s() != _width ||
        image->t() != _height)
    {
        //OSG_NOTICE<<"ScreenCaptureHandler: Allocating image "<<std::endl;
        image->allocateImage(_width, _height, 1, _pixelFormat, _type);

        if (pbo!=0)
        {
            //OSG_NOTICE<<"ScreenCaptureHandler: deleting pbo "<<pbo<<std::endl;
            ext->glDeleteBuffers (1, &pbo);
            pbo = 0;
        }
    }


    if (pbo==0)
    {
        ext->glGenBuffers(1, &pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        //OSG_NOTICE<<"ScreenCaptureHandler: Generating pbo "<<pbo<<std::endl;
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

    if (_captureOperation.valid())
    {
        (*_captureOperation)(*image, _index);
    }

    osg::Timer_t tick_afterCaptureOperation = osg::Timer::instance()->tick();
    updateTimings(tick_start, tick_afterReadPixels, tick_afterMemCpy, tick_afterCaptureOperation, image->getTotalSizeInBytes());

    _currentImageIndex = nextImageIndex;
}

void WindowCaptureCallback::ContextData::multiPBO(osg::GLBufferObject::Extensions* ext)
{
    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();
    unsigned int nextPboIndex = (_currentPboIndex+1)%_pboBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        //OSG_NOTICE<<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    GLuint& copy_pbo = _pboBuffer[_currentPboIndex];
    GLuint& read_pbo = _pboBuffer[nextPboIndex];

    osg::Image* image = _imageBuffer[_currentImageIndex].get();
    if (image->s() != _width ||
        image->t() != _height)
    {
        //OSG_NOTICE<<"ScreenCaptureHandler: Allocating image "<<std::endl;
        image->allocateImage(_width, _height, 1, _pixelFormat, _type);

        if (read_pbo!=0)
        {
            //OSG_NOTICE<<"ScreenCaptureHandler: deleting pbo "<<read_pbo<<std::endl;
            ext->glDeleteBuffers (1, &read_pbo);
            read_pbo = 0;
        }

        if (copy_pbo!=0)
        {
            //OSG_NOTICE<<"ScreenCaptureHandler: deleting pbo "<<copy_pbo<<std::endl;
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

        //OSG_NOTICE<<"ScreenCaptureHandler: Generating pbo "<<read_pbo<<std::endl;
    }

    if (read_pbo==0)
    {
        ext->glGenBuffers(1, &read_pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, read_pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        //OSG_NOTICE<<"ScreenCaptureHandler: Generating pbo "<<read_pbo<<std::endl;
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

        if (_captureOperation.valid())
        {
            (*_captureOperation)(*image, _index);
        }
    }

    ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

    osg::Timer_t tick_afterMemCpy = osg::Timer::instance()->tick();

    updateTimings(tick_start, tick_afterReadPixels, tick_afterMemCpy, tick_afterMemCpy, image->getTotalSizeInBytes());

    _currentImageIndex = nextImageIndex;
    _currentPboIndex = nextPboIndex;
}

WindowCaptureCallback::WindowCaptureCallback(int numFrames, Mode mode, FramePosition position, GLenum readBuffer)
    : _mode(mode),
      _position(position),
      _readBuffer(readBuffer),
      _numFrames(numFrames)
{
}

WindowCaptureCallback::ContextData* WindowCaptureCallback::createContextData(osg::GraphicsContext* gc) const
{
    WindowCaptureCallback::ContextData* cd = new WindowCaptureCallback::ContextData(gc, _mode, _readBuffer);
    cd->_captureOperation = _defaultCaptureOperation;
    return cd;
}

WindowCaptureCallback::ContextData* WindowCaptureCallback::getContextData(osg::GraphicsContext* gc) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    osg::ref_ptr<ContextData>& data = _contextDataMap[gc];
    if (!data) data = createContextData(gc);

    return data.get();
}

void WindowCaptureCallback::setCaptureOperation(ScreenCaptureHandler::CaptureOperation* operation)
{
    _defaultCaptureOperation = operation;

    // Set the capture operation for each ContextData.
    for (ContextDataMap::iterator it = _contextDataMap.begin(); it != _contextDataMap.end(); ++it)
    {
        it->second->_captureOperation = operation;
    }
}


void WindowCaptureCallback::operator () (osg::RenderInfo& renderInfo) const
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    glReadBuffer(_readBuffer);
#endif

    osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
    osg::ref_ptr<ContextData> cd = getContextData(gc);
    cd->read();

    // If _numFrames is > 0 it means capture that number of frames.
    if (_numFrames > 0)
    {
        --_numFrames;
        if (_numFrames == 0)
        {
            // the callback must remove itself when it's done.
            if (_position == START_FRAME)
                renderInfo.getCurrentCamera()->setInitialDrawCallback(0);
            if (_position == END_FRAME)
                renderInfo.getCurrentCamera()->setFinalDrawCallback(0);
        }
    }

    int prec = osg::notify(osg::INFO).precision(5);
    OSG_INFO << "ScreenCaptureHandler: "
                           << "copy="      << (cd->_timeForFullCopy*1000.0f)             << "ms, "
                           << "operation=" << (cd->_timeForCaptureOperation*1000.0f)     << "ms, "
                           << "total="     << (cd->_timeForFullCopyAndOperation*1000.0f) << std::endl;
    osg::notify(osg::INFO).precision(prec);

    cd->_timeForFullCopy = 0;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ScreenCaptureHandler::WriteToFile
//
ScreenCaptureHandler::WriteToFile::WriteToFile(const std::string& filename,
                                               const std::string& extension,
                                               SavePolicy savePolicy)
    : _filename(filename), _extension(extension), _savePolicy(savePolicy)
{
}

void ScreenCaptureHandler::WriteToFile::operator () (const osg::Image& image, const unsigned int context_id)
{
    if (_savePolicy == SEQUENTIAL_NUMBER)
    {
        if (_contextSaveCounter.size() <= context_id)
        {
            unsigned int oldSize = _contextSaveCounter.size();
            _contextSaveCounter.resize(context_id + 1);
            // Initialize all new values to 0 since context ids may not be consecutive.
            for (unsigned int i = oldSize; i <= context_id; i++)
                _contextSaveCounter[i] = 0;
        }
    }

    std::stringstream filename;
    filename << _filename << "_" << context_id;

    if (_savePolicy == SEQUENTIAL_NUMBER)
        filename << "_" << _contextSaveCounter[context_id];

    filename << "." << _extension;

    osgDB::writeImageFile(image, filename.str());

    OSG_INFO<<"ScreenCaptureHandler: Taking a screenshot, saved as '"<<filename.str()<<"'"<<std::endl;

    if (_savePolicy == SEQUENTIAL_NUMBER)
    {
        _contextSaveCounter[context_id]++;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ScreenCaptureHandler
//
ScreenCaptureHandler::ScreenCaptureHandler(CaptureOperation* defaultOperation,
                                           int numFrames)
    : _startCapture(false),
      _stopCapture(false),
      _keyEventTakeScreenShot('c'),
      _keyEventToggleContinuousCapture('C'),
      _callback(new WindowCaptureCallback( numFrames,
                                           WindowCaptureCallback::READ_PIXELS,
//                                          WindowCaptureCallback::SINGLE_PBO,
//                                          WindowCaptureCallback::DOUBLE_PBO,
//                                          WindowCaptureCallback::TRIPLE_PBO,
                                           WindowCaptureCallback::END_FRAME, GL_BACK))
{
    if (defaultOperation)
        setCaptureOperation(defaultOperation);
    else
        setCaptureOperation(new WriteToFile("screen_shot", "jpg"));
}

void ScreenCaptureHandler::setCaptureOperation(CaptureOperation* operation)
{
    WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());
    callback->setCaptureOperation(operation);
}

ScreenCaptureHandler::CaptureOperation* ScreenCaptureHandler::getCaptureOperation() const
{
    WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());
    return callback->getCaptureOperation();
}

void ScreenCaptureHandler::addCallbackToViewer(osgViewer::ViewerBase& viewer)
{
    osg::Camera* camera = findAppropriateCameraForCallback(viewer);
    if (!camera) return;

    WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());
    if (callback && callback->getFramePosition() == WindowCaptureCallback::START_FRAME)
    {
        camera->setInitialDrawCallback(_callback.get());
    }
    else
    {
        camera->setFinalDrawCallback(_callback.get());
    }
}

void ScreenCaptureHandler::removeCallbackFromViewer(osgViewer::ViewerBase& viewer)
{
    osg::Camera* camera = findAppropriateCameraForCallback(viewer);
    if (!camera) return;

    WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());
    if (callback && callback->getFramePosition() == WindowCaptureCallback::START_FRAME)
    {
        camera->setInitialDrawCallback(0);
    }
    else
    {
        camera->setFinalDrawCallback(0);
    }
}

osg::Camera* ScreenCaptureHandler::findAppropriateCameraForCallback(osgViewer::ViewerBase& viewer)
{
    // Select either the first or the last active camera, depending on the
    // frame position set in the callback.
    // One case where testing the node mask is important is when the stats
    // handler has been initialized, but stats are not displayed. In that
    // case, there is a post render camera on the viewer, but its node mask
    // is zero, so the callback added to that camera would never be called.
    WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());

    if (callback->getFramePosition() == WindowCaptureCallback::START_FRAME)
    {
        osgViewer::ViewerBase::Contexts contexts;
        viewer.getContexts(contexts);
        for(osgViewer::ViewerBase::Contexts::iterator itr = contexts.begin();
            itr != contexts.end();
            ++itr)
        {
            osg::GraphicsContext* context = *itr;
            osg::GraphicsContext::Cameras& cameras = context->getCameras();
            osg::Camera* firstCamera = 0;
            for(osg::GraphicsContext::Cameras::iterator cam_itr = cameras.begin();
                cam_itr != cameras.end();
                ++cam_itr)
            {
                if (firstCamera)
                {
                    if ((*cam_itr)->getRenderOrder() < firstCamera->getRenderOrder())
                    {
                        if ((*cam_itr)->getNodeMask() != 0x0)
                            firstCamera = (*cam_itr);
                    }
                    if ((*cam_itr)->getRenderOrder() == firstCamera->getRenderOrder() &&
                        (*cam_itr)->getRenderOrderNum() < firstCamera->getRenderOrderNum())
                    {
                        if ((*cam_itr)->getNodeMask() != 0x0)
                            firstCamera = (*cam_itr);
                    }
                }
                else
                {
                    if ((*cam_itr)->getNodeMask() != 0x0)
                        firstCamera = *cam_itr;
                }
            }

            if (firstCamera)
            {
                //OSG_NOTICE<<"ScreenCaptureHandler: First camera "<<firstCamera<<std::endl;

                return firstCamera;
            }
            else
            {
                OSG_NOTICE<<"ScreenCaptureHandler: No camera found"<<std::endl;
            }
        }
    }
    else
    {
        osgViewer::ViewerBase::Contexts contexts;
        viewer.getContexts(contexts);
        for(osgViewer::ViewerBase::Contexts::iterator itr = contexts.begin();
            itr != contexts.end();
            ++itr)
        {
            osg::GraphicsContext* context = *itr;
            osg::GraphicsContext::Cameras& cameras = context->getCameras();
            osg::Camera* lastCamera = 0;
            for(osg::GraphicsContext::Cameras::iterator cam_itr = cameras.begin();
                cam_itr != cameras.end();
                ++cam_itr)
            {
                if (lastCamera)
                {
                    if ((*cam_itr)->getRenderOrder() > lastCamera->getRenderOrder())
                    {
                        if ((*cam_itr)->getNodeMask() != 0x0)
                            lastCamera = (*cam_itr);
                    }
                    if ((*cam_itr)->getRenderOrder() == lastCamera->getRenderOrder() &&
                        (*cam_itr)->getRenderOrderNum() >= lastCamera->getRenderOrderNum())
                    {
                        if ((*cam_itr)->getNodeMask() != 0x0)
                            lastCamera = (*cam_itr);
                    }
                }
                else
                {
                    if ((*cam_itr)->getNodeMask() != 0x0)
                        lastCamera = *cam_itr;
                }
            }

            if (lastCamera)
            {
                //OSG_NOTICE<<"ScreenCaptureHandler: Last camera "<<lastCamera<<std::endl;

                return lastCamera;
            }
            else
            {
                OSG_NOTICE<<"ScreenCaptureHandler: No camera found"<<std::endl;
            }
        }
    }

    return 0;
}

// aa will point to an osgViewer::View, so we will take a screenshot
// of that view's graphics contexts.
bool ScreenCaptureHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::ViewerBase* viewer = dynamic_cast<osgViewer::View*>(&aa)->getViewerBase();
    if (!viewer) return false;

    switch(ea.getEventType())
    {
        case (osgGA::GUIEventAdapter::FRAME):
        {
            // Booleans aren't the best way of doing this, but I want to do
            // the actual adding here because I don't want to require
            // startCapture() take a viewer as argument, which could not be
            // the right one.
            if (_startCapture)
            {
                // Start capturing with the currently set number of frames.
                // If set to -1 it will capture continuously, if set to >0
                // it will capture that number of frames.
                _startCapture = false;
                addCallbackToViewer(*viewer);
            }
            else if (_stopCapture)
            {
                _stopCapture = false;
                removeCallbackFromViewer(*viewer);
            }
            break;
        }

        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey() == _keyEventTakeScreenShot)
            {
                // Check that we will capture at least one frame.
                // Just check for ==0, because >0 is means we're already
                // capturing and <0 means it will capture all frames.
                WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());
                if (callback->getFramesToCapture() == 0)
                {
                    setFramesToCapture(1);
                }
                addCallbackToViewer(*viewer);
                return true;
            }

            if (ea.getKey() == _keyEventToggleContinuousCapture)
            {
                if (getFramesToCapture() < 0)
                {
                    setFramesToCapture(0);
                    removeCallbackFromViewer(*viewer);
                }
                else
                {
                    setFramesToCapture(-1);
                    addCallbackToViewer(*viewer);
                }
                return true;
            }
            break;
        }
    default:
        break;
    }

    return false;
}

/** Capture the given viewer's views on the next frame. */
void ScreenCaptureHandler::captureNextFrame(osgViewer::ViewerBase& viewer)
{
    addCallbackToViewer(viewer);
}

/** Set the number of frames to capture. */
void ScreenCaptureHandler::setFramesToCapture(int numFrames)
{
    WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());
    callback->setFramesToCapture(numFrames);
}

/** Get the number of frames to capture. */
int ScreenCaptureHandler::getFramesToCapture() const
{
    WindowCaptureCallback* callback = static_cast<WindowCaptureCallback*>(_callback.get());
    return callback->getFramesToCapture();
}

/** Start capturing at the end of the next frame. */
void ScreenCaptureHandler::startCapture()
{
    if (getFramesToCapture() != 0)
        _startCapture = true;
}

/** Stop capturing. */
void ScreenCaptureHandler::stopCapture()
{
    _stopCapture = true;
}

/** Get the keyboard and mouse usage of this manipulator.*/
void ScreenCaptureHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding(_keyEventTakeScreenShot,"Take screenshot.");
    usage.addKeyboardMouseBinding(_keyEventToggleContinuousCapture,"Toggle continuous screen capture.");
}

}
