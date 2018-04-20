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

#include <stdlib.h>
#include <float.h>
#include <limits.h>

#include <iomanip>
#include <sstream>

#include <osgDB/FileNameUtils>

#include <osg/Version>
#include <osg/Geometry>
#include <osg/TexMat>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/os_utils>
#include <osg/io_utils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

namespace osgViewer
{

/*
** WindowSizeHandler
*/

WindowSizeHandler::WindowSizeHandler() :
    _keyEventToggleFullscreen('f'),
    _toggleFullscreen(true),
    _keyEventWindowedResolutionUp('>'),
    _keyEventWindowedResolutionDown('<'),
    _changeWindowedResolution(true),
    _currentResolutionIndex(-1)
{
    _resolutionList.push_back(osg::Vec2(640, 480));
    _resolutionList.push_back(osg::Vec2(800, 600));
    _resolutionList.push_back(osg::Vec2(1024, 768));
    _resolutionList.push_back(osg::Vec2(1152, 864));
    _resolutionList.push_back(osg::Vec2(1280, 720));
    _resolutionList.push_back(osg::Vec2(1280, 768));
    _resolutionList.push_back(osg::Vec2(1280, 1024));
    _resolutionList.push_back(osg::Vec2(1440, 900));
    _resolutionList.push_back(osg::Vec2(1400, 1050));
    _resolutionList.push_back(osg::Vec2(1600, 900));
    _resolutionList.push_back(osg::Vec2(1600, 1024));
    _resolutionList.push_back(osg::Vec2(1600, 1200));
    _resolutionList.push_back(osg::Vec2(1680, 1050));
    _resolutionList.push_back(osg::Vec2(1920, 1080));
    _resolutionList.push_back(osg::Vec2(1920, 1200));
    _resolutionList.push_back(osg::Vec2(2048, 1536));
    _resolutionList.push_back(osg::Vec2(2560, 2048));
    _resolutionList.push_back(osg::Vec2(3200, 2400));
    _resolutionList.push_back(osg::Vec2(3840, 2400));
}

void WindowSizeHandler::getUsage(osg::ApplicationUsage &usage) const
{
    usage.addKeyboardMouseBinding(_keyEventToggleFullscreen, "Toggle full screen.");
    usage.addKeyboardMouseBinding(_keyEventWindowedResolutionUp, "Increase the screen resolution (in windowed mode).");
    usage.addKeyboardMouseBinding(_keyEventWindowedResolutionDown, "Decrease the screen resolution (in windowed mode).");
}

bool WindowSizeHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;

    osgViewer::ViewerBase* viewer = view->getViewerBase();

    if (viewer == NULL)
    {
        return false;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (_toggleFullscreen == true && ea.getKey() == _keyEventToggleFullscreen)
            {

                // sleep to allow any viewer rendering threads to complete before we
                // resize the window
                OpenThreads::Thread::microSleep(100000);

                osgViewer::Viewer::Windows windows;
                viewer->getWindows(windows);
                for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
                    itr != windows.end();
                    ++itr)
                {
                    toggleFullscreen(*itr);
                }

                aa.requestRedraw();
                return true;
            }
            else if (_changeWindowedResolution == true && ea.getKey() == _keyEventWindowedResolutionUp)
            {
                // sleep to allow any viewer rendering threads to complete before we
                // resize the window
                OpenThreads::Thread::microSleep(100000);

                // Increase resolution
                osgViewer::Viewer::Windows    windows;
                viewer->getWindows(windows);
                for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
                    itr != windows.end();
                    ++itr)
                {
                    changeWindowedResolution(*itr, true);
                }

                aa.requestRedraw();
                return true;
            }
            else if (_changeWindowedResolution == true && ea.getKey() == _keyEventWindowedResolutionDown)
            {
                // sleep to allow any viewer rendering threads to complete before we
                // resize the window
                OpenThreads::Thread::microSleep(100000);

                // Decrease resolution
                osgViewer::Viewer::Windows    windows;
                viewer->getWindows(windows);
                for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
                    itr != windows.end();
                    ++itr)
                {
                    changeWindowedResolution(*itr, false);
                }

                aa.requestRedraw();
                return true;
            }
            break;
        }
    default:
        break;
    }
    return false;
}

void WindowSizeHandler::toggleFullscreen(osgViewer::GraphicsWindow *window)
{
    osg::GraphicsContext::WindowingSystemInterface    *wsi = osg::GraphicsContext::getWindowingSystemInterface();

    if (wsi == NULL)
    {
        OSG_NOTICE << "Error, no WindowSystemInterface available, cannot toggle window fullscreen." << std::endl;
        return;
    }

    unsigned int    screenWidth;
    unsigned int    screenHeight;

    wsi->getScreenResolution(*(window->getTraits()), screenWidth, screenHeight);

    int x;
    int y;
    int width;
    int height;

    window->getWindowRectangle(x, y, width, height);

    bool    isFullScreen = x == 0 && y == 0 && width == (int)screenWidth && height == (int)screenHeight;

    if (isFullScreen)
    {
        osg::Vec2    resolution;

        if (_currentResolutionIndex == -1)
        {
            _currentResolutionIndex = getNearestResolution(screenWidth, screenHeight, screenWidth / 2, screenHeight / 2);
        }
        resolution = _resolutionList[_currentResolutionIndex];
        window->setWindowDecoration(true);
        window->setWindowRectangle((screenWidth - (int)resolution.x()) / 2, (screenHeight - (int)resolution.y()) / 2, (int)resolution.x(), (int)resolution.y());
        OSG_INFO << "Screen resolution = " << (int)resolution.x() << "x" << (int)resolution.y() << std::endl;
    }
    else
    {
        window->setWindowDecoration(false);
        window->setWindowRectangle(0, 0, screenWidth, screenHeight);
    }

    window->grabFocusIfPointerInWindow();
}

void WindowSizeHandler::changeWindowedResolution(osgViewer::GraphicsWindow *window, bool increase)
{
    osg::GraphicsContext::WindowingSystemInterface    *wsi = osg::GraphicsContext::getWindowingSystemInterface();

    if (wsi == NULL)
    {
        OSG_NOTICE << "Error, no WindowSystemInterface available, cannot toggle window fullscreen." << std::endl;
        return;
    }

    unsigned int    screenWidth;
    unsigned int    screenHeight;

    wsi->getScreenResolution(*(window->getTraits()), screenWidth, screenHeight);

    int x;
    int y;
    int width;
    int height;

    window->getWindowRectangle(x, y, width, height);

    bool    isFullScreen = x == 0 && y == 0 && width == (int)screenWidth && height == (int)screenHeight;

    if (window->getWindowDecoration() == true || isFullScreen == false)
    {
        osg::Vec2    resolution;

        if (_currentResolutionIndex == -1)
        {
            _currentResolutionIndex = getNearestResolution(screenWidth, screenHeight, width, height);
        }

        if (increase == true)
        {
            // Find the next resolution
            for (int i = _currentResolutionIndex + 1; i < (int)_resolutionList.size(); ++i)
            {
                if ((unsigned int)_resolutionList[i].x() <= screenWidth && (unsigned int)_resolutionList[i].y() <= screenHeight)
                {
                    _currentResolutionIndex = i;
                    break;
                }
            }
        }
        else
        {
            // Find the previous resolution
            for (int i = _currentResolutionIndex - 1; i >= 0; --i)
            {
                if ((unsigned int)_resolutionList[i].x() <= screenWidth && (unsigned int)_resolutionList[i].y() <= screenHeight)
                {
                    _currentResolutionIndex = i;
                    break;
                }
            }
        }

        resolution = _resolutionList[_currentResolutionIndex];
        window->setWindowDecoration(true);
        window->setWindowRectangle((screenWidth - (int)resolution.x()) / 2, (screenHeight - (int)resolution.y()) / 2, (int)resolution.x(), (int)resolution.y());
        OSG_INFO << "Screen resolution = " << (int)resolution.x() << "x" << (int)resolution.y() << std::endl;

        window->grabFocusIfPointerInWindow();
    }
}

unsigned int WindowSizeHandler::getNearestResolution(int screenWidth, int screenHeight, int width, int height) const
{
    unsigned int    position = 0;
    unsigned int    result = 0;
    int                delta = INT_MAX;

    for (std::vector<osg::Vec2>::const_iterator it = _resolutionList.begin();
        it != _resolutionList.end();
        ++it, ++position)
    {
        if ((int)it->x() <= screenWidth && (int)it->y() <= screenHeight)
        {
            int tmp = static_cast<int>(osg::absolute((width * height) - (it->x() * it->y())));

            if (tmp < delta)
            {
                delta = tmp;
                result = position;
            }
        }
    }
    return (result);
}

/*
** ThreadingHandler
*/

ThreadingHandler::ThreadingHandler() :
    _keyEventChangeThreadingModel('m'),
    _changeThreadingModel(true),
    _keyEventChangeEndBarrierPosition('e'),
    _changeEndBarrierPosition(true)
{
    _tickOrLastKeyPress = osg::Timer::instance()->tick();
}

void ThreadingHandler::getUsage(osg::ApplicationUsage &usage) const
{
    usage.addKeyboardMouseBinding(_keyEventChangeThreadingModel, "Toggle threading model.");
    usage.addKeyboardMouseBinding(_keyEventChangeEndBarrierPosition, "Toggle the placement of the end of frame barrier.");
}

bool ThreadingHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;

    osgViewer::ViewerBase* viewerBase = view->getViewerBase();
    osgViewer::Viewer* viewer = dynamic_cast<Viewer*>(viewerBase);

    if (viewerBase == NULL)
    {
        return false;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            double    delta = osg::Timer::instance()->delta_s(_tickOrLastKeyPress, osg::Timer::instance()->tick());

            if (_changeThreadingModel == true && ea.getKey() == _keyEventChangeThreadingModel && delta > 1.0)
            {

                _tickOrLastKeyPress = osg::Timer::instance()->tick();

                switch(viewerBase->getThreadingModel())
                {
                case(osgViewer::ViewerBase::SingleThreaded):
                    viewerBase->setThreadingModel(osgViewer::ViewerBase::CullDrawThreadPerContext);
                    OSG_NOTICE<<"Threading model 'CullDrawThreadPerContext' selected."<<std::endl;
                    break;
                case(osgViewer::ViewerBase::CullDrawThreadPerContext):
                    viewerBase->setThreadingModel(osgViewer::ViewerBase::DrawThreadPerContext);
                    OSG_NOTICE<<"Threading model 'DrawThreadPerContext' selected."<<std::endl;
                    break;
                case(osgViewer::ViewerBase::DrawThreadPerContext):
                    viewerBase->setThreadingModel(osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext);
                    OSG_NOTICE<<"Threading model 'CullThreadPerCameraDrawThreadPerContext' selected."<<std::endl;
                    break;
                case(osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext):
                    viewerBase->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
                    OSG_NOTICE<<"Threading model 'SingleThreaded' selected."<<std::endl;
                    break;
#if 1
                case(osgViewer::ViewerBase::AutomaticSelection):
                    viewerBase->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
                    OSG_NOTICE<<"Threading model 'SingleThreaded' selected."<<std::endl;
#else
                case(osgViewer::ViewerBase::AutomaticSelection):
                    viewerBase->setThreadingModel(viewer->suggestBestThreadingModel());
                    OSG_NOTICE<<"Threading model 'AutomaticSelection' selected."<<std::endl;
#endif
                    break;
                }

                aa.requestRedraw();
                return true;
            }
            if (viewer && _changeEndBarrierPosition == true && ea.getKey() == _keyEventChangeEndBarrierPosition)
            {
                switch(viewer->getEndBarrierPosition())
                {
                case(osgViewer::Viewer::BeforeSwapBuffers):
                    viewer->setEndBarrierPosition(osgViewer::Viewer::AfterSwapBuffers);
                    OSG_NOTICE<<"Threading end of frame barrier position 'AfterSwapBuffers' selected."<<std::endl;
                    break;
                case(osgViewer::Viewer::AfterSwapBuffers):
                    viewer->setEndBarrierPosition(osgViewer::Viewer::BeforeSwapBuffers);
                    OSG_NOTICE<<"Threading end of frame barrier position 'BeforeSwapBuffers' selected."<<std::endl;
                    break;
                }

                aa.requestRedraw();
                return true;
            }
            break;
        }
    default:
        break;
    }
    return false;
}

RecordCameraPathHandler::RecordCameraPathHandler(const std::string& filename, float fps):
    _filename(filename),
    _autoinc( -1 ),
    _keyEventToggleRecord('z'),
    _keyEventTogglePlayback('Z'),
    _currentlyRecording(false),
    _currentlyPlaying(false),
    _delta(0.0f),
    _animStartTime(0),
    _lastFrameTime(osg::Timer::instance()->tick())
{
    osg::getEnvVar("OSG_RECORD_CAMERA_PATH_FPS", fps);

    _interval = 1.0f / fps;
}

void RecordCameraPathHandler::getUsage(osg::ApplicationUsage &usage) const
{
    usage.addKeyboardMouseBinding(_keyEventToggleRecord, "Toggle camera path recording.");
    usage.addKeyboardMouseBinding(_keyEventTogglePlayback, "Toggle camera path playback.");
}

bool RecordCameraPathHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);

    if (view == NULL)
    {
        return false;
    }

    if(ea.getEventType()==osgGA::GUIEventAdapter::FRAME)
    {
        // Calculate our current delta (difference) in time between the last frame and
        // current frame, regardless of whether we actually store a ControlPoint...
        osg::Timer_t time = osg::Timer::instance()->tick();
        double delta = osg::Timer::instance()->delta_s(_lastFrameTime, time);
        _lastFrameTime = time;

        // If our internal _delta is finally large enough to warrant a ControlPoint
        // insertion, do so now. Be sure and reset the internal _delta, so we can start
        // calculating when the next insert should happen.
        if (_animPath.valid() && _currentlyRecording && _delta >= _interval)
        {
            const osg::Matrixd& m = view->getCamera()->getInverseViewMatrix();
            double animationPathTime = osg::Timer::instance()->delta_s(_animStartTime, time);
            _animPath->insert(animationPathTime, osg::AnimationPath::ControlPoint(m.getTrans(), m.getRotate()));
            _delta = 0.0f;

            if (_fout)
            {
                _animPath->write(_animPath->getTimeControlPointMap().find(animationPathTime), _fout);
                _fout.flush();
            }

        }
        else _delta += delta;

        return true;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            // The user has requested to toggle recording.
            if (ea.getKey() ==_keyEventToggleRecord)
            {
                // The user has requested to BEGIN recording.
                if (!_currentlyRecording)
                {
                    _currentlyRecording = true;
                    _animStartTime = osg::Timer::instance()->tick();
                    _animPath = new osg::AnimationPath();

                    if (!_filename.empty())
                    {
                        std::stringstream ss;
                        ss << osgDB::getNameLessExtension(_filename);
                        if ( _autoinc != -1 )
                        {
                            ss << "_"<<std::setfill( '0' ) << std::setw( 2 ) << _autoinc;
                            _autoinc++;
                        }
                        ss << "."<<osgDB::getFileExtension(_filename);

                        OSG_NOTICE << "Recording camera path to file " << ss.str() << std::endl;
                        _fout.open( ss.str().c_str() );

                        // make sure doubles are not truncated by default stream precision = 6
                        _fout.precision( 15 );
                    }
                    else
                    {
                        OSG_NOTICE<<"Recording camera path."<<std::endl;
                    }
                }

                // The user has requested to STOP recording, write the file!
                else
                {
                    _currentlyRecording = false;
                    _delta = 0.0f;

                    if (_fout) _fout.close();
                }

                return true;
            }

            // The user has requested to toggle playback. You'll notice in the code below that
            // we take over the current manipulator; it was originally recommended that we
            // check for a KeySwitchManipulator, create one if not present, and then add this
            // to either the newly created one or the existing one. However, the code do that was
            // EXTREMELY dirty, so I opted for a simpler solution. At a later date, someone may
            // want to implement the original recommendation (which is in a mailing list reply
            // from June 1st by Robert in a thread called "osgviewer Camera Animation (preliminary)".
            else if (ea.getKey() == _keyEventTogglePlayback)
            {
                if (_currentlyRecording)
                {
                    _currentlyRecording = false;
                    _delta = 0.0f;

                    if (_animPath.valid() && !_animPath->empty())
                    {
                        // In the future this will need to be written continuously, rather
                        // than all at once.
                        osgDB::ofstream out(_filename.c_str());
                        OSG_NOTICE<<"Writing camera file: "<<_filename<<std::endl;
                        _animPath->write(out);
                        out.close();
                    }
                    else
                    {
                        OSG_NOTICE<<"No animation path to write out."<<std::endl;
                    }
                }

                // The user has requested to BEGIN playback.
                if (!_currentlyPlaying)
                {
                     if (_animPath.valid() && !_animPath->empty())
                     {
                        _animPathManipulator = new osgGA::AnimationPathManipulator(_animPath.get());
                        _animPathManipulator->home(ea,aa);


                        // If we successfully found our _filename file, set it and keep a copy
                        // around of the original CameraManipulator to restore later.
                        if (_animPathManipulator.valid() && _animPathManipulator->valid())
                        {
                            _oldManipulator = view->getCameraManipulator();
                            view->setCameraManipulator(_animPathManipulator.get());
                            _currentlyPlaying = true;
                        }
                     }
                }

                // The user has requested to STOP playback.
                else
                {
                    // Restore the old manipulator if necessary and stop playback.
                    if(_oldManipulator.valid()) view->setCameraManipulator(_oldManipulator.get());
                    _currentlyPlaying = false;
                    _oldManipulator = 0;
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

LODScaleHandler::LODScaleHandler():
    _keyEventIncreaseLODScale('*'),
    _keyEventDecreaseLODScale('/')
{
}

bool LODScaleHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    osg::Camera* camera = view ? view->getCamera() : 0;
    if (!camera) return false;

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey() == _keyEventIncreaseLODScale)
            {
                camera->setLODScale(camera->getLODScale()*1.1);
                OSG_NOTICE<<"LODScale = "<<camera->getLODScale()<<std::endl;

                aa.requestRedraw();
                return true;
            }

            else if (ea.getKey() == _keyEventDecreaseLODScale)
            {
                camera->setLODScale(camera->getLODScale()/1.1);
                OSG_NOTICE<<"LODScale = "<<camera->getLODScale()<<std::endl;

                aa.requestRedraw();
                return true;
            }

            break;
        }
    default:
        break;
    }

    return false;
}

void LODScaleHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding(_keyEventIncreaseLODScale,"Increase LODScale.");
    usage.addKeyboardMouseBinding(_keyEventDecreaseLODScale,"Decrease LODScale.");
}

ToggleSyncToVBlankHandler::ToggleSyncToVBlankHandler():
    _keyEventToggleSyncToVBlank('v')
{
}

bool ToggleSyncToVBlankHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;

    osgViewer::ViewerBase* viewer = view->getViewerBase();

    if (viewer == NULL)
    {
        return false;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey() == _keyEventToggleSyncToVBlank)
            {
                // Increase resolution
                osgViewer::Viewer::Windows    windows;

                viewer->getWindows(windows);
                for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
                    itr != windows.end();
                    ++itr)
                {
                    (*itr)->setSyncToVBlank( !(*itr)->getSyncToVBlank() );
                }

                aa.requestRedraw();
                return true;
            }

            break;
        }
    default:
        break;
    }

    return false;
}


void ToggleSyncToVBlankHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding(_keyEventToggleSyncToVBlank,"Toggle SyncToVBlank.");
}


InteractiveImageHandler::InteractiveImageHandler(osg::Image* image) :
    _image(image),
    _texture(0),
    _fullscreen(false),
    _camera(0)
{
}

InteractiveImageHandler::InteractiveImageHandler(osg::Image* image, osg::Texture2D* texture, osg::Camera* camera) :
    _image(image),
    _texture(texture),
    _fullscreen(true),
    _camera(camera)
{
    if (_camera.valid() && _camera->getViewport())
    {
        // Send an initial resize event (with the same size) so the image can
        // resize itself initially.
        double width = _camera->getViewport()->width();
        double height = _camera->getViewport()->height();

        resize(static_cast<int>(width), static_cast<int>(height));
    }
}

bool InteractiveImageHandler::mousePosition(osgViewer::View* view, osg::NodeVisitor* nv, const osgGA::GUIEventAdapter& ea, int& x, int &y) const
{
    if (!view) return false;
    if (_fullscreen)
    {
        x = (int) ea.getX();
        y = (int) ea.getY();
        return true;
    }

    osgUtil::LineSegmentIntersector::Intersections intersections;
    bool foundIntersection = (nv==0) ? view->computeIntersections(ea, intersections) :
                                       view->computeIntersections(ea, nv->getNodePath(), intersections);

    if (foundIntersection)
    {
        osg::Vec2 tc(0.5f,0.5f);

        // use the nearest intersection
        const osgUtil::LineSegmentIntersector::Intersection& intersection = *(intersections.begin());
        osg::Drawable* drawable = intersection.drawable.get();
        osg::Geometry* geometry = drawable ? drawable->asGeometry() : 0;
        osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;
        if (vertices)
        {
            // get the vertex indices.
            const osgUtil::LineSegmentIntersector::Intersection::IndexList& indices = intersection.indexList;
            const osgUtil::LineSegmentIntersector::Intersection::RatioList& ratios = intersection.ratioList;

            if (indices.size()==3 && ratios.size()==3)
            {
                unsigned int i1 = indices[0];
                unsigned int i2 = indices[1];
                unsigned int i3 = indices[2];

                float r1 = ratios[0];
                float r2 = ratios[1];
                float r3 = ratios[2];

                osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
                osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
                if (texcoords_Vec2Array)
                {
                    // we have tex coord array so now we can compute the final tex coord at the point of intersection.
                    osg::Vec2 tc1 = (*texcoords_Vec2Array)[i1];
                    osg::Vec2 tc2 = (*texcoords_Vec2Array)[i2];
                    osg::Vec2 tc3 = (*texcoords_Vec2Array)[i3];
                    tc = tc1*r1 + tc2*r2 + tc3*r3;
                }
            }

            osg::TexMat* activeTexMat = 0;
            osg::Texture* activeTexture = 0;

            if (drawable->getStateSet())
            {
                osg::TexMat* texMat = dynamic_cast<osg::TexMat*>(drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXMAT));
                if (texMat) activeTexMat = texMat;

                osg::Texture* texture = dynamic_cast<osg::Texture*>(drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
                if (texture) activeTexture = texture;
            }

            if (activeTexMat)
            {
                osg::Vec4 tc_transformed = osg::Vec4(tc.x(), tc.y(), 0.0f,0.0f) * activeTexMat->getMatrix();
                tc.x() = tc_transformed.x();
                tc.y() = tc_transformed.y();
            }

            if (dynamic_cast<osg::TextureRectangle*>(activeTexture))
            {
                x = int( tc.x() );
                y = int( tc.y() );
            }
            else if (_image.valid())
            {
                x = int( float(_image->s()) * tc.x() );
                y = int( float(_image->t()) * tc.y() );
            }

            return true;
        }

    }

    return false;
}


bool InteractiveImageHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    if (ea.getHandled()) return false;

    if (!_image) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::DRAG):
        case(osgGA::GUIEventAdapter::PUSH):
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            int x,y;
            if (mousePosition(view, nv, ea, x, y))
            {
                return _image->sendPointerEvent(x, y, ea.getButtonMask());
            }
            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            int x,y;
            bool sendKeyEvent = mousePosition(view, nv, ea, x, y);

            if (sendKeyEvent)
            {
                return _image->sendKeyEvent(ea.getKey(), ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN);
            }
            break;
        }
        case (osgGA::GUIEventAdapter::RESIZE):
        {
            if (_fullscreen && _camera.valid())
            {
                _camera->setViewport(0, 0, ea.getWindowWidth(), ea.getWindowHeight());

                resize(ea.getWindowWidth(), ea.getWindowHeight());
                return true;
            }
            break;
        }

        default:
            return false;
    }
    return false;
}

bool InteractiveImageHandler::cull(osg::NodeVisitor* nv, osg::Drawable*, osg::RenderInfo*) const
{
    if (_image.valid())
    {
        _image->setFrameLastRendered(nv->getFrameStamp());
    }

    return false;
}

void InteractiveImageHandler::resize(int width, int height)
{
    if (_image.valid())
    {
        _image->scaleImage(width, height, 1);
    }

    // Make sure the texture does not rescale the image because
    // it thinks it should still be the previous size...
    if (_texture.valid())
        _texture->setTextureSize(width, height);
}

}
